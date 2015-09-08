/*-
 * Copyright (c) 2015 Chris Spiegel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>

#include <QObject>

#include <libopenmpt/libopenmpt.h>
#include <libopenmpt/libopenmpt_stream_callbacks_file.h>

#include "mptwrap.h"

MPTWrap::MPTWrap(std::string filename)
{
  std::FILE *fp;

  fp = std::fopen(filename.c_str(), "r");
  if(fp == NULL) throw InvalidFile();

  mod = openmpt_module_create(openmpt_stream_get_file_callbacks(), fp, openmpt_log_func_silent, NULL, NULL);
  if(mod == NULL)
  {
    std::fclose(fp);
    throw InvalidFile();
  }

  std::fclose(fp);

  openmpt_module_select_subsong(mod, -1);

  duration_ = openmpt_module_get_duration_seconds(mod) * 1000;
  title_ = copystr(openmpt_module_get_metadata(mod, "title"));
  format_ = copystr(openmpt_module_get_metadata(mod, "type_long"));
  pattern_count_ = openmpt_module_get_num_patterns(mod);
  instrument_count_ = openmpt_module_get_num_instruments(mod);
  sample_count_ = openmpt_module_get_num_samples(mod);
  channel_count_ = openmpt_module_get_num_channels(mod);

  for(int i = 0; i < openmpt_module_get_num_instruments(mod); i++)
  {
    instruments_.push_back(copystr(openmpt_module_get_instrument_name(mod, i)));
  }

  for(int i = 0; i < openmpt_module_get_num_samples(mod); i++)
  {
    samples_.push_back(copystr(openmpt_module_get_sample_name(mod, i)));
  }

  comment_ = copystr(openmpt_module_get_metadata(mod, "message_raw"));
}

MPTWrap::~MPTWrap()
{
  openmpt_module_destroy(mod);
}

std::string MPTWrap::copystr(const char *src)
{
  std::string dst = src;
  openmpt_free_string(src);
  return dst;
}

std::vector<MPTWrap::Interpolator> MPTWrap::get_interpolators()
{
  std::vector<Interpolator> interpolators =
  {
    Interpolator(QObject::tr("None").toUtf8().constData(), interp_none),
    Interpolator(QObject::tr("Linear").toUtf8().constData(), interp_linear),
    Interpolator(QObject::tr("Cubic").toUtf8().constData(), interp_cubic),
    Interpolator(QObject::tr("Windowed sinc").toUtf8().constData(), interp_windowed),
  };

  return interpolators;
}

bool MPTWrap::is_valid_interpolator(int interpolator_value)
{
  std::vector<Interpolator> interpolators = get_interpolators();

  return std::any_of(interpolators.begin(), interpolators.end(),
                     [&interpolator_value]
                     (const Interpolator &interpolator){ return interpolator.value == interpolator_value; });
}

int MPTWrap::default_interpolator()
{
  return interp_windowed;
}

void MPTWrap::set_interpolator(int interpolator_value)
{
  if(is_valid_interpolator(interpolator_value))
  {
    openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH, interpolator_value);
  }
}

bool MPTWrap::is_valid_stereo_separation(int separation)
{
  return separation >= 0 && separation <= 100;
}

int MPTWrap::default_stereo_separation()
{
  return 70;
}

void MPTWrap::set_stereo_separation(int separation)
{
  if(is_valid_stereo_separation(separation))
  {
    openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_STEREOSEPARATION_PERCENT, separation);
  }
}

bool MPTWrap::can_play(std::string filename)
{
  try
  {
    MPTWrap mpt(filename.c_str());
    return true;
  }
  catch(MPTWrap::InvalidFile)
  {
  }

  return false;
}

std::int64_t MPTWrap::read(void *buf, std::int64_t bufsiz)
{
  bufsiz /= sizeof(std::int16_t);
  size_t n;

  n = openmpt_module_read_interleaved_stereo(mod, rate(), bufsiz / channels(), reinterpret_cast<std::int16_t *>(buf));

  return n * channels() * sizeof(std::int16_t);
}

void MPTWrap::seek(int pos)
{
  openmpt_module_set_position_seconds(mod, pos / 1000.0);
}
