#ifndef __DAQ_DEVICE_DIGITIZER_H__
#define __DAQ_DEVICE_DIGITIZER_H__




#include <stdio.h>


#include <daq_device.h>
#include <digitizerTriggerHandler.h>
#include <iostream>
#include <fstream>
#include <string>

//nclude "jseb2reader_lib.h"


class daq_device_digitizer : public  daq_device {

public:

  daq_device_digitizer (const int eventtype = 0
			, const int subeventid = 0
			, const string & dcm2datfileName = "dcm2ADCPar.dat"
			, const string & dcm2partreadoutJsebName = "JSEB2.8.0"
			, const string & dcm2Jseb2Name = "JSEB2.5.0"
			, const string & adcJseb2Name = "JSEB2.5.0"
			, const string & adcconfigfilename = "adcconfigfile.txt"
			, const int nevents = 0
);
  
  ~daq_device_digitizer();


  void identify(std::ostream& os = std::cout) const;

  int max_length(const int etype) const;

  // functions to do the work

  int put_data(const int etype, int * adr, const int length);

  int init();
  int endrun();

  int rearm( const int etype);
  bool adc_getline(ifstream *fin, string *line); 
  int parseAdcConfigFile( string adcconfigfilename);

protected:
  subevtdata_ptr sevt;
  unsigned int number_of_words;
  string _dcm2jseb2name;
  string _adcjseb2name;
  string _partreadoutJseb2Name;
  string _dcm2configfilename; // dcm2 dat file
  string _dcm2name;
  string _outputDataFileName;
  int _slot_nr;
  int _nr_modules;
  int _l1_delay;
  int _nevents;
  int _n_samples;
  int _num_xmitgroups;
  /*
  struct xmit_group_modules {
    int slot_nr;
    int nr_modules;
    int l1_delay;
    int n_samples;
  };
  */
  struct xmit_group_modules xmit_groups[3];

  //  std::string _dcmhost;
  // std::string _configfile;

  int _broken;

  digitizerTriggerHandler *_th;


};

#endif

