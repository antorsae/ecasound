// ------------------------------------------------------------------------
// audioio_sndfile.cpp: Interface to the sndfile library.
// Copyright (C) 2003 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

#include <kvu_message_item.h>
#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "audioio_sndfile.h"
#include "samplebuffer.h"
#include "eca-version.h"
#include "eca-error.h"
#include "eca-logger.h"

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
static const char* audio_io_keyword_const = "sndfile";
static const char* audio_io_keyword_regex_const = "(^sndfile$)|(w64$)|(vox$)|(paf$)|(iff$)|(nist$)|($mat[45])|(nist$)|(xi$)|(htk$)";

const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
int audio_io_interface_version(void) { return(ecasound_library_version_current); }
#endif

using namespace std;

SNDFILE_INTERFACE::SNDFILE_INTERFACE (const string& name)
{
  finished_rep = false;
  snd_repp = 0;
  set_label(name);
}

SNDFILE_INTERFACE::~SNDFILE_INTERFACE(void)
{
  if (is_open() == true) {
    close();
  }
}

SNDFILE_INTERFACE* SNDFILE_INTERFACE::clone(void) const
{
  SNDFILE_INTERFACE* target = new SNDFILE_INTERFACE();
  for(int n = 0; n < number_of_params(); n++) {
    target->set_parameter(n + 1, get_parameter(n + 1));
  }
  return(target);
}

void SNDFILE_INTERFACE::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  AUDIO_IO::open();

  SF_INFO sfinfo;

  string real_filename = label();
  if (real_filename == "sndfile") {
    real_filename = opt_filename_rep;
  }

  switch(io_mode()) {
  case io_read:
    {
      snd_repp = sf_open(real_filename.c_str(), SFM_READ, &sfinfo);
      if (snd_repp == NULL) {
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Can't open file \"" + real_filename
			  + "\" using libsndfile."));
      }
      else {
	ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-sndfile) format: " + kvu_numtostr(sfinfo.format)); 
	set_samples_per_second((long int)sfinfo.samplerate);
	set_channels(sfinfo.channels);
	string format;
	switch(sfinfo.format & SF_FORMAT_SUBMASK) 
	  {
	  case SF_FORMAT_PCM_S8: { format = "s8"; break; }
	  case SF_FORMAT_PCM_U8: { format = "u8"; break; }
	  case SF_FORMAT_PCM_16: { format = "s16"; break; }
	  case SF_FORMAT_PCM_24: { format = "s24"; break; }
	  case SF_FORMAT_PCM_32: { format = "s32"; break; }
	  case SF_FORMAT_FLOAT: { format = "f32"; break; }
	    // FIXME: add missing SF_FORMAT_* SUBTYPES fields
	  default: { throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Unknown sample format.")); }
	  }
	
	if (sfinfo.format & SF_ENDIAN_LITTLE) 
	  format += "_le";
	else if (sfinfo.format & SF_ENDIAN_BIG) 
	  format += "_be";
	
	set_sample_format_string(format);
	set_length_in_samples(sfinfo.frames);
      }
      break;
    }
  case io_write:
    {
      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-sndfile) Using libsndfile to open file \"" +
		    real_filename + "\" for writing.");
#if 0
      AFfilesetup fsetup;
      fsetup = afNewFileSetup();

      int file_format = -1;
      string teksti = real_filename;
      kvu_to_lowercase(teksti);

      if (strstr(teksti.c_str(),".aiffc") != 0) { file_format = AF_FILE_AIFFC; }
      else if (strstr(teksti.c_str(),".aifc") != 0) { file_format = AF_FILE_AIFFC; }
      else if (strstr(teksti.c_str(),".aiff") != 0) { file_format = AF_FILE_AIFF; }
      else if (strstr(teksti.c_str(),".aif") != 0) { file_format = AF_FILE_AIFF; }
      else if (strstr(teksti.c_str(),".au") != 0) { file_format = AF_FILE_NEXTSND; }
      else if (strstr(teksti.c_str(),".snd") != 0) { file_format = AF_FILE_NEXTSND; }
      else {
	ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-af) Warning! Unknown audio format, using raw format instead.");
	file_format = AF_FILE_RAWDATA;
      }
      ::afInitFileFormat(fsetup, file_format);
      ::afInitChannels(fsetup, AF_DEFAULT_TRACK, channels());

      if (format_string()[0] == 'u')
	::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_UNSIGNED, bits());
      else if (format_string()[0] == 's')
	::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, bits());
      else if (format_string()[0] == 'f') {
	if (bits() == 32) 
	  ::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_FLOAT, bits());
	else
	  ::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_DOUBLE, bits());
      }

      ::afInitRate(fsetup, AF_DEFAULT_TRACK, static_cast<double>(samples_per_second()));

      afhandle = ::afOpenFile(real_filename.c_str(), "w", fsetup);
      if (afhandle == AF_NULL_FILEHANDLE) 
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Can't open file \"" + real_filename
			  + "\" using libaudiofile."));
#endif
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Output-mode not yet supported."));
      break;

    }
  
  case io_readwrite:
    {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Simultaneous intput/output not yet supported."));
    }
  }
}

void SNDFILE_INTERFACE::close(void)
{
  AUDIO_IO::close();
  if (snd_repp != 0) {
    sf_close(snd_repp);
  }
}

bool SNDFILE_INTERFACE::finished(void) const
{
  if (finished_rep == true || 
      (io_mode() == io_read && out_position())) return(true);

  return false;
}

long int SNDFILE_INTERFACE::read_samples(void* target_buffer, 
					 long int samples)
{
  // FIXME: convert to using sf_read_float()
  samples_read = sf_read_raw(snd_repp, target_buffer, frame_size() * samples);
  samples_read /= frame_size();
  finished_rep = (samples_read < samples) ? true : false;
  return(samples_read);
}

void SNDFILE_INTERFACE::write_samples(void* target_buffer, 
					long int samples)
{
  // FIXME: convert to using sf_write_float()
  sf_write_raw(snd_repp, target_buffer, frame_size() * samples);
}

void SNDFILE_INTERFACE::seek_position(void)
{
  // FIXME: check if format supports seeking
  sf_seek(snd_repp, position_in_samples(), SEEK_SET);
  finished_rep = false;
}

void SNDFILE_INTERFACE::set_parameter(int param, 
				      string value)
{
  switch (param) {
  case 1: 
    set_label(value);
    break;

  case 2: 
    opt_filename_rep = value;
    break;
  }
}

string SNDFILE_INTERFACE::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(opt_filename_rep);
  }
  return("");
}