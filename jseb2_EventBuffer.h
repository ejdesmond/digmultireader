#ifndef __JSEB2_EVENTBUFFER_H__
#define __JSEB2_EVENTBUFFER_H__

//================================================
/// \file jseb2_EventBuffer.h
/// \brief stores events; based on M. McCumber Dcm2_FileBuffer.h
/// \brief handles PRDF file output
/// \author Michael P. McCumber
//================================================

// Dcm2 includes
#include <Dcm2_BaseObject.h>

// WinDriver includes
#include <windrvr.h>
#include <wdc_defs.h>

// standard includes
#include <string>

// forward declarations
class Dcm2_Board;
class Dcm2_Partitioner;
class Dcm2_Partition;

/// \class Jseb2_EventBuffer
///
/// \brief This class handles PRDF output
///
class Jseb2_EventBuffer : public Dcm2_BaseObject {

 public:

  Jseb2_EventBuffer();
  virtual ~Jseb2_EventBuffer();

  /// closes the object
  bool Close();  

  /// opens a new output file
  bool OpenFile(std::string outfilename);
  /// closes current output file
  bool CloseFile();

  /// starts a new event in the output stream
  bool StartEvent();
  /// copy a frame of words into the stream
  bool CopyFrameIntoBuffer(unsigned int nwords, UINT32 *frameWords);
  /// ends the current event in the output stream
  bool EndEvent();
  /// resets the buffer to the beginning of the current event
  bool ResetEvent();

  /// set the current write pointer in the buffer
  void SetBufferPointer(UINT32 *bufferpointer);
  /// get the current write pointer in the buffer
  UINT32* GetBufferPointer() {return _pobuffer;}

  /// set the current PHENIX event number
  void SetEventNumber(UINT32 event_number) {_event_number = event_number;}
  /// get the current PHENIX event number
  UINT32 GetEventNumber() {return _event_number;}

  /// set the current PHENIX run number
  void SetRunNumber(UINT32 run_number) {_run_number = run_number;}
  /// get the current PHENIX run number
  UINT32 GetRunNumber() {return _run_number;}

  /// verify buffer has space remaining
  bool CheckBufferSpace();

  /// writes current memory buffer to open file
  bool WriteBufferToFile();

  /// resets memory buffer
  bool ResetBuffer();

 private:

  /// full path to output file
  std::string _outfilename;

  /// pointer to the output file
  FILE *_outfile;

  // here i am picking a reasonable buffer size...
  // 10000 words per packet * 8 packets per board 
  // * 4 boards per partition * 100 events
  /// buffer size in words (~100 events) 
  static const unsigned int MAX_WORDS_PER_FILEBUFFER = 
    MAX_WORDS_PER_PACKET*8*4*100;

  /// buffer size in bytes (~100 events)
  UINT32 _nbufferbytes;
  /// storage for the output buffer
  UINT32 _obuffer[MAX_WORDS_PER_FILEBUFFER]; 

  /// current writing position
  UINT32 *_pobuffer;
  /// beginning of last event header
  UINT32 *_pobuffer_eventhead;
  /// beginning of last frame header
  UINT32 *_pobuffer_framehead;
  /// beginning of last buffer header
  UINT32 *_pobuffer_bufferhead;

  /// prdf buffer header storage
  UINT32 _buffer_header[4];
  /// event buffer header storage 
  UINT32 _event_header[8]; 

  /// prdf buffer sequence number
  UINT32 _bufferseq;
  /// PHENIX event number
  UINT32 _event_number;
  /// prdf event type
  UINT32 _eventType;
  /// PHENIX run number
  UINT32 _run_number;

  /// running tally of words in current event
  UINT32 _nWordsInEvent;
  /// to preserve buffered data when closing file
  bool _bufferHasData;
  /// appears to be unused
  bool _writeToFile;
};

#endif // __DCM2_FILEBUFFER_H__
