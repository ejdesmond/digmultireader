
#include "digitizerTriggerHandler.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

#include <signal.h>

#include "common_defs.h"

extern pthread_mutex_t M_cout;
extern pthread_mutex_t DataReadoutDone;
extern pthread_mutex_t DataReadyReset;

#define BUFFER_LENGTH 260096


using namespace std;


/**
 * digitizerTriggerHandle ctor
 * arguements
 * string & dcm2partreadoutJsebName ; jseb2 connected to dcm2 partitioner
 * string & dcm2deviceName ; dcm2 object name
 * string & dcm2datfileName ; dcm2 dat config file name
 * string & dcm2Jseb2Name ; jseb2 connected to dcm2 controller
 */
digitizerTriggerHandler::digitizerTriggerHandler( const int eventtype
						  , xmit_group_modules xmit_groups[]
						  , const string & dcm2datfileName
						  , const string & outputDataFileName
						  , const string & dcm2partreadoutJsebName
						  , const string & dcm2Jseb2Name
						  , const string & adcJseb2Name  
						  , const int num_xmitgroups	  
						  , const int nevents
						  
)
{

  _broken = 0;
  partreadoutJseb2Name = dcm2partreadoutJsebName;
  dcm2jseb2name = dcm2Jseb2Name;
  _adcJseb2Name = adcJseb2Name; 
  dcm2configfilename = "/home/phnxrc/desmond/daq/build/" + dcm2datfileName;
  datafiledirectory = "/data0/phnxrc/buffer_t1044/t1044/";
  _outputDataFileName =  outputDataFileName;

  _num_xmitgroups = num_xmitgroups;


  for (int ig = 0; ig < num_xmitgroups; ig++)
    {
      
	  _xmit_groups[ig].slot_nr = xmit_groups[ig].slot_nr;
	  _xmit_groups[ig].nr_modules = xmit_groups[ig].nr_modules;
	  _xmit_groups[ig].l1_delay = xmit_groups[ig].l1_delay;
	  _xmit_groups[ig].n_samples = xmit_groups[ig].n_samples;
	  
    }
 
  _nevents = nevents;
   //partreadoutJseb2Name = "JSEB2.8.0";
  //dcm2jseb2name = "JSEB2.5.0"; // dcm2 controller jseb2 on va095 port 1
  
  //dcm2name = "DCMGROUP.TEST.1";
  //dcm2configfilename = "/home/phnxrc/desmond/daq/build/dcm2ADCPar.dat";
  jseb2adcportnumber = 2; // default jseb2 port number for digitizer control
  _nevents   = (unsigned long)nevents;

  debugReadout = false;
  enable_rearm = false;
  initialize();

}


int digitizerTriggerHandler::rearm()
{
  
  if ( enable_rearm == false)
    {
      enable_rearm = true;
    } else
    {
      pthread_mutex_unlock(&DataReadoutDone ); // continue dma readout
      pthread_mutex_lock(&DataReadyReset); // wait for trigger ready is reset
      //pthread_mutex_lock(&M_cout);
      //cout << __FILE__ << " " << __LINE__ << " TH->rearm LOCKED DataReadyReset ** " << endl;
      //pthread_mutex_unlock(&M_cout);
    }
  
  return 0;
} // rearm

bool digitizerTriggerHandler::dcm2_getline(ifstream *fin, string *line) {

  // needed to parse dat files with and without
  // end of line escape characters

  string tempstring;
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



/**
 * parseDcm2Configfile
 * find the name of the dcm in the dat file
 * label:ONCS_PARTITIONER, name:DCMGROUP.TEST.1
 */
int digitizerTriggerHandler::parseDcm2Configfile(string dcm2configfilename)
{
  int status = 0;
  string cfg_line;
 // open configuration file
  ifstream fin(dcm2configfilename.c_str());
  if (!fin) {
    
    return 1;
  }
  size_t pos = 0;
  string tokenholder;
  int linecount = 0;
  // in dat file name is on line 2 after name:
  while (dcm2_getline(&fin, &cfg_line)) 
    {
      cout << "parse search line " << cfg_line.c_str() << endl;
      //while(( cfg_line.substr(cfg_line.find("name:"))) != string::npos) 
      // search for label:ONCS_PARTITIONER
      if((pos = cfg_line.find("label:")) != string::npos)
	{
	  tokenholder = cfg_line.substr(pos+6,16);
	  cout << __LINE__ <<  " found label " << tokenholder.c_str() << endl;
	  
	}
      if (strncmp(tokenholder.c_str(),"ONCS_PARTITION",14) == 0 )
	{
	  cout << __LINE__ << " found  ONCS_PARTITONER label " << endl;
      if((pos = cfg_line.find("name:")) != string::npos)
	{
	  // now should be at the position of the name: start after : character
	  dcm2name = cfg_line.substr(pos+5,15);
	  cout << __LINE__ << " digitizertriggerHandler found dcm2name " << dcm2name.c_str() << endl;
	  break;
	} // search for name word
	} else {
	cout << __LINE__ << " NOT on ONCS_PARTITONER line - get next line" << endl;
      }
      linecount++;
      if (linecount > 4)
	break;

    } // search string

    return status;

} // parseDcm2Configfile(dcm2configfilename)


int digitizerTriggerHandler::initialize()  // called at object creation
{
  int partitionNb = 0;
  dcm2_tid = 0;
  enable_rearm = false;

  string _outputDataFileName = "rcdaqadctest.prdf";
 

  parseDcm2Configfile(dcm2configfilename);

  pjseb2c = new jseb2Controller();
  jseb2Controller::forcexit = false; // signal jseb run end
  pjseb2c->Init(partreadoutJseb2Name);
  pjseb2c->setNumberofEvents(_nevents);
  pjseb2c->setOutputFilename(_outputDataFileName);
  
} // initialize

int digitizerTriggerHandler::init()  // called at begin-run
{

  
  int partitionNb = 0;
  dcm2_tid = 0;
  enable_rearm = false;

  string _outputDataFileName = "rcdaqfvtxtest.prdf";
  
 
  for ( int ig = 0; ig < _num_xmitgroups ; ig++ )
    {
      
	  
	  pADC[ig] = new ADC();

	  pADC[ig]->setNumAdcBoards(_xmit_groups[ig].nr_modules);
	  pADC[ig]->setFirstAdcSlot(_xmit_groups[ig].slot_nr);
	  pADC[ig]->setJseb2Name(_adcJseb2Name);
	  pADC[ig]->setL1Delay(_xmit_groups[ig].l1_delay);
	  pADC[ig]->setSampleSize(_xmit_groups[ig].n_samples);
	  
	
	
  

    }
  // 11/14/2018 adc is hardcoded to use JSEB2 port 2. This option implements the change
  //padc->setJseb2Port(jseb2adcportnumber); // TODO add option to set port number

  // create dcm2 control object
  pdcm2Object = new dcm2Impl(dcm2name.c_str(),partitionNb );
 
  jseb2Controller::forcexit = false; // signal jseb run end
  pjseb2c->setExitRequest(false);

 // set up process memory map for exit notificatin
  pdcm2Object->setExitMap();

   // configure (download) the dcm2
  pdcm2Object->download(dcm2name,dcm2configfilename,partitionNb);
  
       
  pjseb2c->start_read2file(); // start read2file  thread
// test if DMA buffer allocation succeeded
  if (jseb2Controller::dmaallocationsuccess == false)
	 {
	   cerr << "Failed to allocate DMA buffer space - aborting start run " << endl;
	   
	     delete pdcm2Object;
	     //pjseb2c->Close();
	     //delete pjseb2c;
	   return 1;
	 }

  // initialize adc modules _xmit_groups[ig].nr_modules // num_modules for each xmit group
  for (int ixmit = 0 ; ixmit < _num_xmitgroups ; ixmit++)
    {
      
	  if ( pADC[ixmit]->initialize(_nevents) == false ) {
	    cout << "Failed to initialize xmit - try restart windriver " << endl;
	    return 1;
	  }
	 
	  if ( pADC[ixmit]->initialize(_nevents) == false ) {
	    cout << "Failed to initialize xmit - try restart windriver " << endl;
	    return 1;
	  }
	 
	  if ( pADC[ixmit]->initialize(_nevents) == false ) {
	    cout << "Failed to initialize xmit - try restart windriver " << endl;
	    return 1;
	  }
	  break;
	
    } // init all xmit modules

dcm2_tid = pdcm2Object->startRunThread(dcm2name,_nevents,_outputDataFileName.c_str());


 enable_rearm = false;
 

  return 0;
} // init


digitizerTriggerHandler::~digitizerTriggerHandler()
{
  pjseb2c->Close();
  cout << __FILE__ << " " << __LINE__ << " delete jsebController object " << endl;
  delete pjseb2c;
  delete pdcm2Object;
	
  //  if (hPlx) DIGITIZER_Close( hPlx );
  //if (buffer) free(buffer);
}



int digitizerTriggerHandler::wait_for_trigger( const int moreinfo)
{
  //  const int delayvalue = 10000;
 while(jseb2Controller::rcdaqdataready != true)
   {
     if (debugReadout)
       cout << __FILE__ << " " << " wait_for_trigger NO data detected  " << endl; 
     
    return 0;
   }

 if (debugReadout) {
   cout << __FILE__ << " " << __LINE__ << " wait_for_trigger data detected dataready = "<< jseb2Controller::rcdaqdataready  << endl;
 }
 
  return 1;
}

int digitizerTriggerHandler::endrun()    // called at begin-run
{
  
  cout << __FILE__ << " " << __LINE__ << " endrun - CLOSE jseb " << endl;
  
  // A test for the run completed should be executed. This test is set
  // if the specified number of events are collected. 
  // This test is if (jseb2Controller::runcompleted == false) endrun

  delete padc;
 
  pdcm2Object->stopRun(dcm2name);
  // release lock on ReadoutDone so jseb can exit.
  pthread_mutex_unlock(&DataReadoutDone );
  pthread_mutex_unlock(&DataReadyReset );
  //cout << __FILE__ << " " << __LINE__ << " set forcexit to TRUE " << endl;
  jseb2Controller::forcexit = true; // signal jseb run end
  pjseb2c->setExitRequest(true);
  //cout << __FILE__ << " " << __LINE__ << " setExitRequest to TRUE sleep " << endl;
  sleep(4); // wait for data to be written
  //pjseb2c->Close();
  //cout << __FILE__ << " " << __LINE__ << " delete dcm2 object " << endl;
  delete pdcm2Object;
  
  return 0;
}

int digitizerTriggerHandler::put_data( int *d, const int max_words)
{
  
  if ( _broken ) return 0;


  static long imod,ichip;
  static UINT32 i,k,nread,iprint;

  iprint = 0;
                                               

  int *dest = d;
  // 8bits for the samples, 8 bits for the delay,  8 bits for slot, 8 bits for _nr_modules
  
  int wordcount = 0; // we have one word so far
  
  nread = pjseb2c->getEventSize();
  
  // get pointer to data buffer
  const UINT32 *peventdataarray = pjseb2c->getEventData();
  // sanity check on event size
  if (nread < 6000 )
    {
      for (int idata = 0; idata < nread ; idata++ )
	{
	  *dest = *(peventdataarray + idata);
	  dest++;
	}
    } // nread sanity check
  
  if (debugReadout) {
    //cout << __FILE__ << " " << __LINE__ << " put_data: read back  " << nread << " event words - sending back words read = " << nread - 8 << endl;
  }
      wordcount = nread;

     
      // remove discarded dcm2 header in returned word count
  return nread - 8;

}

