#ifndef INCLUDED_AUDIOIO_ARTS_H
#define INCLUDED_AUDIOIO_ARTS_H

#include <string>
#include <iostream>

extern "C" {
#include <artsc.h>
}

#include "sample-specs.h"
#include "eca-version.h"

/**
 * Interface for communicating with aRts/MCOP.
 * @author Kai Vehmanen
 */
class ARTS_INTERFACE : public AUDIO_IO_DEVICE {

  ARTS_INTERFACE(const ARTS_INTERFACE& x) { }
  ARTS_INTERFACE& operator=(const ARTS_INTERFACE& x) { return *this; }

  arts_stream_t stream_rep;

  SAMPLE_SPECS::sample_pos_t samples_rep;
  static int ref_rep;
  
 public:

  string name(void) const { return("aRts client"); }
  string description(void) const { return("aRts client. Audio input and output using aRts server."); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual bool supports_nonblocking_mode(void) const { return(false); }
  virtual bool supports_seeking(void) const { return(false); }
  virtual bool locked_audio_format(void) const { return(false); }

  virtual void stop(void);
  virtual void start(void);
  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;

  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  ARTS_INTERFACE (const string& name = "arts");
  ~ARTS_INTERFACE(void);
    
  ARTS_INTERFACE* clone(void) const { std::cerr << "Not implemented!" << std::endl; return 0; }
  ARTS_INTERFACE* new_expr(void) const { return new ARTS_INTERFACE(); }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new ARTS_INTERFACE()); }
int audio_io_interface_version(void);
const char* audio_io_keyword(void);
const char* audio_io_keyword_regex(void);
};

#endif
