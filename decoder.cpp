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

#include <cstring>
#include <memory>

#include <QString>
#include <QtGlobal>

#include <qmmp/decoder.h>

#include "decoder.h"
#include "mptwrap.h"

MPTDecoder::MPTDecoder(const QString &path)
        : Decoder(),
          path(path)
{
}

bool MPTDecoder::initialize()
{
  try
  {
    mpt = std::unique_ptr<MPTWrap>(new MPTWrap(path.toUtf8().constData()));
  }
  catch(MPTWrap::InvalidFile)
  {
    return false;
  }

  configure(mpt->rate(), mpt->channels(), Qmmp::PCM_S16LE);

  return true;
}

qint64 MPTDecoder::totalTime()
{
  return mpt->duration();
}

int MPTDecoder::bitrate()
{
  return mpt->channel_count();
}

qint64 MPTDecoder::read(char *audio, qint64 max_size)
{
  mpt->set_interpolator(settings.get_interpolator());
  mpt->set_stereo_separation(settings.get_stereo_separation());

  return mpt->read(reinterpret_cast<unsigned char *>(audio), max_size);
}

void MPTDecoder::seek(qint64 pos)
{
  mpt->seek(pos);
}
