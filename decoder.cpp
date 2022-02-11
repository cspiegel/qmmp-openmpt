// Copyright (c) 2015 Chris Spiegel
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#include <cstring>
#include <memory>

#include <QString>
#include <QtGlobal>

#include <qmmp/decoder.h>

#include "decoder.h"
#include "mptwrap.h"

MPTDecoder::MPTDecoder(QIODevice *device) : Decoder(device)
{
}

bool MPTDecoder::initialize()
{
    if (!input()) {
        return false;
    }

    if (!input()->isOpen() && !input()->open(QIODevice::ReadOnly)) {
        return false;
    }

    try {
        m_mpt = std::make_unique<MPTWrap>(input());
    } catch (const MPTWrap::InvalidFile &) {
        return false;
    }

    configure(m_mpt->rate(), m_mpt->channels(), Qmmp::PCM_FLOAT);

    return true;
}

qint64 MPTDecoder::totalTime() const
{
    return m_mpt->duration();
}

int MPTDecoder::bitrate() const
{
    return m_mpt->channel_count();
}

qint64 MPTDecoder::read(unsigned char *audio, qint64 max_size)
{
    m_mpt->set_interpolator(m_settings.get_interpolator());
    m_mpt->set_stereo_separation(m_settings.get_stereo_separation());

    return m_mpt->read(audio, max_size);
}

void MPTDecoder::seek(qint64 pos)
{
    m_mpt->seek(pos);
}
