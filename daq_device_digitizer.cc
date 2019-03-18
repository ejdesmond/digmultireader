#include "daq_device_digitizer.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <ctype.h>

#define IDHBD_FPGASHORT       1222

pthread_mutex_t DataReadoutDone;
pthread_mutex_t DataReadyReset;

using namespace std;
//eventtype, subevent_id,dcm2datfileName,partjseb2name, dcm2jseb2name, l1_delay, nevents, num_xmitgroups, slot_nr_g1, nr_modules_g1,slot_nr_g2, nr_modules_g2,slot_nr_g3, nr_modules_g3

daq_device_digitizer::daq_device_digitizer(const int eventtype
					   , const int sub_id
					   , const string & dcm2datfileName
					   , const string & dcm2partreadoutJsebName
					   , const string & dcm2Jseb2Name
					   , const string & adcJseb2Name
					   , const string & adcconfigfilename 
					   , const int nevents )
{

  m_eventType  = eventtype;
  _dcm2configfilename = dcm2datfileName;
  _partreadoutJseb2Name = dcm2partreadoutJsebName;
  _dcm2jseb2name = dcm2Jseb2Name;
  _adcjseb2name =  adcJseb2Name;
  _outputDataFileName = "rcdaq_digitizerdata.prdf"; // default file for filebuffer
 
  if (parseAdcConfigFile( adcconfigfilename) != 0 )
    {
      std::cout  << __FILE__ << " " << __LINE__ << " Error: unable to open configuration file  "<< adcconfigfilename.c_str() << " using defaults 16,3,58,31 " << endl;

	xmit_groups[0].slot_nr = 16;
	xmit_groups[0].nr_modules = 3;
	xmit_groups[0].l1_delay = 58;
	xmit_groups[0].n_samples = 31;
	_num_xmitgroups = 1;
    }
  _nevents    = nevents;
  //_num_xmitgroups = n_xmitgroups;

  std::cout << __LINE__ << "  " << __FILE__ << " num xmit groups = " << _num_xmitgroups << endl;
  


  _th = 0;

  _broken = 0;

  pthread_mutex_init( &DataReadoutDone, 0);
  pthread_mutex_init( &DataReadyReset, 0);
  pthread_mutex_lock(  &DataReadoutDone);



  cout << __LINE__ << "  " << __FILE__ << " create new triggerhandler " << endl;
  //  _th  = new digitizerTriggerHandler ( m_eventType,_dcm2configfilename,_outputDataFileName,_partreadoutJseb2Name,_dcm2jseb2name ,_adcjseb2name,_slot_nr,_nr_modules,_l1_delay,_n_samples,_nevents);

  _th  = new digitizerTriggerHandler ( m_eventType,xmit_groups,_dcm2configfilename,_outputDataFileName,_partreadoutJseb2Name,_dcm2jseb2name ,_adcjseb2name,_num_xmitgroups,_nevents);

  _broken = _th->get_status();
  registerTriggerHandler ( _th);

}


/*
 * init
 * returns 0 on success ; non-zero on failure
 */
int  daq_device_digitizer::init()
{
  int status = 0;
  if ( _th) 
    status = _th->init();

  return status;
}

bool daq_device_digitizer::adc_getline(ifstream *fin, string *line) 
{
  // needed to parse config files with and without
  // end of line escape characters
  string tempstring;
  char * newline = NULL;
  (*line) = "";
  
  // read in all lines until the last char is not a '\'
  while (getline(*fin, tempstring)) { 
    // if no escape character, end of line
    int length = tempstring.size();
    if (tempstring[length-1] != '\\') {
      (*line) += tempstring;
      
      return true;
    } else { // continue to next line
      tempstring.resize(length-2);
      (*line) += tempstring;
    }
  }

  // end of file
  return false;

}

/*
 *
 * file format 
 *  each line contains a single line for each xmit group. Each line contains:
 *  nevents, n_xmitgroups, n_slot, n_modules, l1_delay, n_samples
 *  0,1,16,3,48,31
 */
int daq_device_digitizer::parseAdcConfigFile( string adcconfigfilename)
{
  string cfg_line;
 // open configuration file
  ifstream fin(adcconfigfilename.c_str());
  if (!fin) {
    
    return 1;
  }
  size_t pos = 0;
  
  
  int tokencount = 0;
  int nxmitgroups = 0;
  int tmpvalue = 0;
  char * ptmps;
  // parse each line
  while (adc_getline(&fin, &cfg_line)) 
    {
      tokencount = 0;
      cout << __FILE__ << " " << __LINE__ << " parse search line " << cfg_line.c_str() << endl;
      
      // if first character is a / or a ' ' then skip the line
      char *tmpstr = new char [cfg_line.length() + 1];
      strcpy(tmpstr,cfg_line.c_str());
      if (isdigit(tmpstr[0]) != 0)
	{
	  cout << __FILE__ << " " << __LINE__ << " found parameter line " << tmpstr<< endl;
	  // parse the config file string to int
	  ptmps = strtok(tmpstr,",");
	  // parse string for adc configuration values
	  cout << " GET THE VALUES " << endl;
	  while (ptmps != NULL)
	    {
	      
	      switch (tokencount)
		{
		case 0:
		  sscanf(ptmps,"%d",&tmpvalue);
		  _nevents = tmpvalue;
		  cout << "nevents = " << _nevents << endl;
		case 1:
		  
		  sscanf(ptmps,"%d",&tmpvalue);
		  
		  xmit_groups[nxmitgroups].nr_modules = tmpvalue;
		  cout << " got nmodules " << tmpvalue << endl;
		  break;
		case 2:
		  sscanf(ptmps,"%d",&tmpvalue);
		  xmit_groups[nxmitgroups].slot_nr = tmpvalue;
		  cout << " got slot_nr " << tmpvalue << endl;
		  break;
		case 3:
		  sscanf(ptmps,"%d",&tmpvalue);
		  xmit_groups[nxmitgroups].l1_delay = tmpvalue;
		  cout << " got l1_delay " << tmpvalue << endl;
		  break;
		case 4:
		  sscanf(ptmps,"%d",&tmpvalue);
		  xmit_groups[nxmitgroups].n_samples = tmpvalue;
		  cout << " got n_samples " << tmpvalue << endl;
		  break;
		default :
		  break;
		}
	      
	      ptmps = strtok(NULL,",");
	      
	      tokencount++;
	      
	    } // found a token	
	  cout << __LINE__ << " next line " << endl;
	  nxmitgroups++;
	} // line starts with digit so is a parameter line
      
      
      delete tmpstr;
    } // search string
  // set total number of xmit groups found
  _num_xmitgroups = nxmitgroups;
    return 0;

} // parseAdcConfigFile



daq_device_digitizer::~daq_device_digitizer()
{
  /*
  if ( _th) 
    {
      clearTriggerHandler();
      delete _th;
      _th = 0;
    }
  */
}



// the put_data function

/**
 * put_data
 * When this function is called data is determined to be ready. So read the data
 */
int daq_device_digitizer::put_data(const int etype, int * adr, const int length )
{
  //cout << __FILE__ << "  " << __LINE__ << " put_data etype= " << etype  << " length= " << length << endl;

  if ( _broken ) 
    {
      cout << __LINE__ << "  " << __FILE__ << " broken ";
      identify();
      return 0; //  we had a catastrophic failure
    }

  if ( ! _th) return 0;  // if we don't have a trigger object, we are not reading anything right now

  if (etype != m_eventType )  // not our id
    {
      return 0;
    }

  // PRDFEVENT == 1
  if (!  daq_getEventFormat() ) 
    {
      return 0;  // we only do prdf and don't do anything for now if not
    }
  
  /*
  sevt =  (subevtdata_ptr) adr;
  // set the initial subevent length
  sevt->sub_length =  SEVTHEADERLENGTH;

  // update id's etc
  sevt->sub_id =  m_subeventid;
  sevt->sub_type=4;
  sevt->sub_decoding = ID4EVT;
  sevt->reserved[0] = 0;
  sevt->reserved[1] = 0;
      
  
  int  *d = (int *) &sevt->data;
  
  int len = length - SEVTHEADERLENGTH -1;
  */
  int len;
  len = _th->put_data(adr, length ); // if not using header
  /*
  len = _th->put_data(d, len );
  sevt->sub_padding = len%2;
  len = len + (len%2);
  sevt->sub_length += len;
  
  return  sevt->sub_length;
  */
  //cout << __FILE__ << "  " << __LINE__ << " put_data: returning data read = " << len << endl;
  return len;
}


void daq_device_digitizer::identify(std::ostream& os) const
{
  
  os  << "JSEB2READER Device  Event Type: " << m_eventType << " partition Jseb2 " << _partreadoutJseb2Name.c_str();

  if ( _th) os << " **trigger ";
  if ( _broken) os << " ** not functional ** " << _broken;
  os << endl;

}

int daq_device_digitizer::max_length(const int etype) const
{
  if (etype != m_eventType) return 0;
  //return  (_nr_modules * 40 * 12 + 10 + 8);
  return (5989); // (5997); // (5998); // max size of event from jseb2reader
}

int  daq_device_digitizer::endrun()
{
  if ( _th) _th->endrun();
  return 0;
}

// the rearm() function
int  daq_device_digitizer::rearm(const int etype)
{
  if (etype != m_eventType) return 0;
  return 0;
}


