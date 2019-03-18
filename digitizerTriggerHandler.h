#ifndef __DIGITIZERTRIGGERHANDLER_H__
#define __DIGITIZERTRIGGERHANDLER_H__

#include <string>

#include "TriggerHandler.h"



#include "wdc_defs.h"
#include "wdc_lib.h"
#include "utils.h"
#include "status_strings.h"
#include "samples/shared/diag_lib.h"
#include "samples/shared/wdc_diag_lib.h"
#include "pci_regs.h"
#include "wizard/my_projects/jseb2_lib.h"
#include "adcj.h"
//#include "rcdaqController.h"
#include "jseb2Controller.h"
#include "dcm2Impl.h"

#include <Dcm2_FileBuffer.h>

struct xmit_group_modules {
    int slot_nr;
    int nr_modules;
    int l1_delay;
    int n_samples;
  };

#define VENDORID 0x1172
#define DEVICEID 0x4


class Eventiterator;

class digitizerTriggerHandler : public TriggerHandler {
  
public:

  digitizerTriggerHandler(  const int eventtype
			    , xmit_group_modules  xmit_groups[] 
			    , const string & dcm2datfileName = "dcm2FvtxPar.dat"
			    , const string & outputDataFileName = "rcdaq_fvtxdata.prdf"
			    ,const string & dcm2partreadoutJsebName = "JSEB2.8.0"
			    , const string & dcm2Jseb2Name = "JSEB2.5.0"
			    , const string & adcJseb2Name = "JSEB2.5.0"    
			    , const int num_xmitgroups = 1
			    , const int nevents = 0
			    
);

  ~digitizerTriggerHandler(); 


  //  virtual void identify(std::ostream& os = std::cout) const = 0;

  // this is the virtual worker routine
  int wait_for_trigger( const int moreinfo=0);
  int initialize();
  int init();
  int endrun();
  int parseDcm2Configfile(string dcm2datfilename);
  int put_data( int *d, const int max_words);
  int get_status() const {return _broken;};
  bool dcm2_getline(ifstream *fin, string *line);
 int rearm();

 protected:



  int _broken;

  int get_buffer();  // read the next buffer and parse it
  ADC * padc;
  ADC * pADC[3]; // array of pointers to adc objects
  jseb2Controller * pjseb2c;
  unsigned long _nevents;
  string dcm2jseb2name;
  string _adcJseb2Name;
  dcm2Impl *pdcm2Object;
  string partreadoutJseb2Name;
  string dcm2configfilename; // dcm2 dat file
  string dcm2name;
  string _outputDataFileName;
  string datafiledirectory;
  pthread_t dcm2_tid;
  bool enable_rearm;
  int _num_xmitgroups;
  int _slot_nr;
  int _nr_modules;
  int _l1_delay;
  int _n_samples;
  int jseb2adcportnumber;

  WD_PCI_SLOT thePCISlot;
  WDC_DEVICE_HANDLE _theHandle;

  unsigned int _evt_count;
  unsigned int _current_event;
  bool debugReadout;
 
  
  struct xmit_group_modules _xmit_groups[3];


  //  static WDC_DEVICE_HANDLE hPlx;
  //static PLX_DMA_HANDLE hDMA;

  //static UINT32 *buffer;

};

#endif
