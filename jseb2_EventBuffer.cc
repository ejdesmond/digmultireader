
#include <jseb2_EventBuffer.h>

// Dcm2 includes
#include <Dcm2_Logger.h>
#include <Dcm2_Board.h>
#include <Dcm2_Partitioner.h>
#include <Dcm2_Partition.h>

// standard includes
#include <stdio.h>
 
using namespace std;

Jseb2_EventBuffer::Jseb2_EventBuffer() {

  _nbufferbytes = MAX_WORDS_PER_FILEBUFFER*sizeof(UINT32); 

  // sensible defaults
  _bufferseq    = 1;
  _event_number = 1;
  _eventType    = 1;
  _run_number   = 1;

  _bufferHasData = false;
  
  return;
}

Jseb2_EventBuffer::~Jseb2_EventBuffer() {
  Close();
  return;
}

bool Jseb2_EventBuffer::Close() {
  return true;
}

/// This method opens a 64bit binary output file and resets the buffer 
/// sequence counter.
///
/// \param[in] outfilename The path to the outputfile
///
/// \return true if successful
///
bool Jseb2_EventBuffer::OpenFile(std::string outfilename) {

  _outfilename =  outfilename;

  // open output file
  //_outfile = fopen64(outfilename.c_str(),"wb"); // trying 64bit file output

 

  // reset file-wise counters
  _bufferseq = 1;

  ResetBuffer();

  return true;
}

/// This method opens a resets the buffer write pointer and
/// header storage. It then starts a new PRDF buffer in
/// the output stream.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::ResetBuffer() {

  // reset buffer pointer to the beginning
  _pobuffer_bufferhead = (UINT32*)&_obuffer;
  _pobuffer = (UINT32*)&_obuffer;

  // reset headers
  _buffer_header[0] = 0x00000000;
  _buffer_header[1] = 0xffffffc0;  // buffer mark
  _buffer_header[2] = _bufferseq;
  _buffer_header[3] = 0x00000001;

  // write out PRDF File Header
  for (unsigned int iword = 0; iword < 4; ++iword) {
    *(_pobuffer++) = _buffer_header[iword];
  }

  _bufferHasData = false;
  
  return true;
}

/// This method writes out any remaining memory data to
/// the file and then closes the file handle.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::CloseFile() {

  // Write out any remaining data
  bool status = true;
  if (_bufferHasData) {
    status = WriteBufferToFile();
  }

  // Close file
  //if (fclose(_outfile) != 0) {
  //  status = false;
  //}

  if (_verbosity >= INFO) {
    Dcm2_Logger::Cout(INFO,"Jseb2_EventBuffer::WriteBufferToFile()") 
      << "Wrote output file: " << _outfilename 
      << Dcm2_Logger::Endl;
  }

  return status;
}

/// This method checks that there is available space
/// in the buffer for the next event. If not, it will
/// flush the buffer to file and start a new one.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::CheckBufferSpace() {

  // 10000 dwords per packet * 4 bytes/dword * 8 fibers
  // * 4 boards per partition = 1,280,000 bytes per event
  // save room for next event only
  //UINT32 nbytes = 131072; // 128 KB
  UINT32 nbytes = MAX_WORDS_PER_PACKET*128; // 1.28 MB
  UINT32 remainingbytes = _nbufferbytes 
    - sizeof(UINT32)*(_pobuffer - _pobuffer_bufferhead);

  // if nbytes is too many
  if (nbytes > remainingbytes) {
    if (_verbosity >= INFO) {
      Dcm2_Logger::Cout(INFO,"Jseb2_EventBuffer::CheckBufferSpace()") 
	<< "Writing buffer to disk and resetting" 
	<< Dcm2_Logger::Endl;
    }

    if (!WriteBufferToFile()) {
      return false;
    }
  }

  // otherwise proceed
  return true;
}

/// This method writes the memory buffer to file. First,
/// it adds the two PRDF buffer trailer words. Then,
/// it updates the current PRDF buffer header words. After
/// pushing the data to file, it checks that all the bytes
/// were written and restarts the memory buffer.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::WriteBufferToFile() {

  // PRDF buffer final two footer words
  *(_pobuffer++) = 0x00000002;
  *(_pobuffer++) = 0x00000000;

  // PRDF file header
  // go back and update buffer header
  _buffer_header[0] = (_pobuffer-_pobuffer_bufferhead)*4; // length in bytes
  _buffer_header[1] = 0xffffffc0;                         // buffer mark
  _buffer_header[2] = _bufferseq;                         // buffer sequence
  _buffer_header[3] = _event_number;                      // event number

  for (unsigned int iword = 0; iword < 4; ++iword) {
    *(_pobuffer_bufferhead++) = _buffer_header[iword];
  }
  // return pointer to beginning of buffer
  _pobuffer_bufferhead = (UINT32*)&_obuffer;
  
  // pad the remaining space until aligned to 8KB
  while ((_pobuffer - _pobuffer_bufferhead)%2048 != 0) {
    *(_pobuffer++) = 0;
  }

  // Write out buffer to file-(must be in 8KB chunks)-----------------
  UINT32 requestedBytes = _pobuffer - _pobuffer_bufferhead;
  //UINT32 writtenBytes = fwrite(_pobuffer_bufferhead, sizeof(UINT32), 
  //			       requestedBytes, _outfile);

  // check that the bytes went to the file
  //if (writtenBytes != requestedBytes) {
  //  return false;
  //}

  // increment buffer sequence counter
  ++_bufferseq;

  // reset buffer
  ResetBuffer();

  return true;
}

/// This method sets the current buffer writing location, but
/// also trips the indicators that data words are in the buffer.
///
/// \param[in] bufferpointer The current write address.
///
void Jseb2_EventBuffer::SetBufferPointer(UINT32 *bufferpointer) {

  UINT32 nWordsAdded = (UINT32)(bufferpointer - _pobuffer);

  _pobuffer = bufferpointer;
  _nWordsInEvent += nWordsAdded;
  _bufferHasData = true;

  return;
}

/// This method starts a new event buffer within the output stream
/// by writing out a new event buffer header.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::StartEvent() {

  // reset event header
  _event_header[0] = 0x00000008;  // event length (words)
  _event_header[1] = _eventType;  // event type (1 = dataevent)
  _event_header[2] = _event_number; // event number of the contained frame(s)
  _event_header[3] = _run_number; // run_number
  _event_header[4] = time(NULL);  // number of seconds since 1-1-1970
  _event_header[5] = 0xffffffff;  // time format
  _event_header[6] = 0x00000000;  // reserved word 1
  _event_header[7] = 0x00000000;  // reserved word 2

  // write out PRDF event (run control "event") header
  _nWordsInEvent = 0;
  _pobuffer_eventhead = _pobuffer;
  for (unsigned int iword = 0; iword < 8; ++iword) {
    *(_pobuffer++) = _event_header[iword];
  }
  _nWordsInEvent += 8;

  return true;
}

/// Frames can be inserted directly (and more quickly) as done in 
/// Dcm2_Board::DMAFrameIntoBuffer() using the pointer gets/sets. 
/// But that way is specific to that kind of object. This function
/// provides a generic way to insert frames in by copying them out 
/// of array. However, use of this function also requires the user 
/// has to handle starting and ending events. This is used in the 
/// receiver functions in jseb2.cc to produce PRDFs on readback 
/// through the jseb2.
///
/// \param[in] nwords The length of the array in 32bit words.
/// \param[in] frameWords A pointer to the beginning of the array.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::CopyFrameIntoBuffer(unsigned int nwords, 
					  UINT32 *frameWords) {

  // get event number from the frame
  SetEventNumber(_event_number);
  
  // get pointer to current writing location
  UINT32 *pobuffer = GetBufferPointer();

  // write out frame header
  for (unsigned int iword = 0; iword < nwords; ++iword) {
    *(pobuffer++) = frameWords[iword];
  }

  // tell the buffer where we've incremented the pointer to
  SetBufferPointer(pobuffer);
  
  return true;
}

/// This method ends the current event buffer in the PRDF
/// output stream. This involves updating the current
/// event buffer header with updated values.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::EndEvent() {

  // PRDF event header
  // go back and update event header
  _event_header[0] = _nWordsInEvent;  // event length (words)
  _event_header[1] = _eventType;      // event type (1 = dataevent)
  _event_header[2] = _event_number;   // event number of the contained frame(s)
  _event_header[3] = _run_number;     // run_number
  _event_header[4] = time(NULL);      // number of seconds since 1-1-1970
  _event_header[5] = 0xffffffff;      // time format
  _event_header[6] = 0x00000000;      // reserved word 1
  _event_header[7] = 0x00000000;      // reserved word 2

  for (unsigned int iword = 0; iword < 8; ++iword) {
    *(_pobuffer_eventhead++) = _event_header[iword];
  }

  return true;
}

/// This method ends the current event creation
/// and backtracks on buffer to erase any data
/// already inserted.
///
/// \return true if successful
///
bool Jseb2_EventBuffer::ResetEvent() {

  // write pointer back to beginning of event
  _pobuffer_framehead = _pobuffer_eventhead;
  _pobuffer = _pobuffer_eventhead;
  
  return true;
}
