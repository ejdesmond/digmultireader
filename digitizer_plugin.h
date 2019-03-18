#ifndef __DIGITIZER_PLUGIN_H__
#define __DIGITIZER_PLUGIN_H__

#include <rcdaq_plugin.h>

#include <daq_device_digitizer.h>
#include <iostream>


class digitizer_plugin : public RCDAQPlugin {

 public:
  int  create_device(deviceblock *db);

  void identify(std::ostream& os = std::cout, const int flag=0) const;

};


#endif

