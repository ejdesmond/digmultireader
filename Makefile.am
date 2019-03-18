AUTOMAKE_OPTIONS = foreign


allsources = \
	adcj.cc \
	jseb2_EventBuffer.cc \
	jseb2Controller.cc \
	dcm2Impl.cc \
	digitizerTriggerHandler.cc \
	daq_device_digitizer.cc \
	digitizer_plugin.cc 


CFLAGS += -DLINUX  -Wall

CXXFLAGS += -DLINUX  -Wall

AM_LDFLAGS = -L$(libdir) -L$(ONLINE_MAIN)/lib 
SPHENIX_MAIN_DIR=/home/phnxrc/softwarerepo/online_distribution
RCDAQ_MAIN=/home/phnxrc/softwarerepo/rcdaq
OS_VER=3.10.0-693.2.2.el7.x86_64
#OS_VER=3.10.0-514.6.1.el7.x86_64
#DCM2_LIB_DIR=/export/software/Linux.${OS_VER}/lib/
DCM2_LIB_DIR=/home/phnxrc/desmond/daq/install/lib
BOOST_ROOT=/export/software/boost/boost_1_61_0

INCLUDES = -I$(includedir) -I$(DCM2_INC)  -I$(RCDAQ_INC) -I$(ONLINE_MAIN)/include  -I${JUNGO_MAIN} \
 -I${BOOST_ROOT}/ -I${BOOST_ROOT}/boost -I${BOOST_ROOT}/boost/thread/ \
 -I${SPHENIX_MAIN_DIR}/newbasic \
 -I${RCDAQ_MAIN} \
 -I${JUNGO_MAIN}/include \
 -I${JUNGO_MAIN}/samples/shared \
 -I${JUNGO_MAIN}/plx/lib \
 -I${JUNGO_MAIN}/plx/diag_lib \
 -I${JUNGO_MAIN}/wizard/my_projects


lib_LTLIBRARIES = librcdaqplugin_digitizer_multireader.la 

librcdaqplugin_digitizer_multireader_la_SOURCES = \
	adcj.cc \
	jseb2_EventBuffer.cc \
	jseb2Controller.cc \
	dcm2Impl.cc \
	daq_device_digitizer.cc \
	digitizerTriggerHandler.cc \
	digitizer_plugin.cc \
	${JUNGO_MAIN}/wizard/my_projects/jseb2_lib.c \
	${JUNGO_MAIN}/samples/shared/diag_lib.c \
	${JUNGO_MAIN}/samples/shared/wdc_diag_lib.c



librcdaqplugin_digitizer_multireader_la_LIBADD = -L${JUNGO_MAIN}/wizard/my_projects/jseb2_installation/lib/ -L${JUNGO_MAIN}/lib -L/export/software/boost/lib  -lboost_thread -lboost_system -L${RCDAQ_LIB} -L${DCM2_LIB_DIR} -lDcm2  -lrcdaqutils -lwdapi1260 -lpthread 

clean-local:
	rm -f $(BUILT_SOURCES)
