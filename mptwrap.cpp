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

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>

#include <QIODevice>
#include <QObject>

#include <libopenmpt/libopenmpt.h>

#include "mptwrap.h"

MPTWrap::MPTWrap(QIODevice *device) :
    m_mod(decltype(m_mod)(openmpt_module_create2(callbacks, device, openmpt_log_func_silent, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr), openmpt_module_destroy))
{
    if (m_mod == nullptr) {
        throw InvalidFile();
    }

    openmpt_module_select_subsong(m_mod.get(), -1);

    auto copystr = [](const char *src) -> std::string {
        if(src != nullptr)
        {
            std::string dst = src;
            openmpt_free_string(src);
            return dst;
        }
        else
        {
            return "";
        }
    };

    m_duration = openmpt_module_get_duration_seconds(m_mod.get()) * 1000;
    m_title = copystr(openmpt_module_get_metadata(m_mod.get(), "title"));
    m_format = copystr(openmpt_module_get_metadata(m_mod.get(), "type_long"));
    m_pattern_count = openmpt_module_get_num_patterns(m_mod.get());
    m_channel_count = openmpt_module_get_num_channels(m_mod.get());

    for (int i = 0; i < openmpt_module_get_num_instruments(m_mod.get()); i++) {
        m_instruments.push_back(copystr(openmpt_module_get_instrument_name(m_mod.get(), i)));
    }

    for (int i = 0; i < openmpt_module_get_num_samples(m_mod.get()); i++) {
        m_samples.push_back(copystr(openmpt_module_get_sample_name(m_mod.get(), i)));
    }

    m_comment = copystr(openmpt_module_get_metadata(m_mod.get(), "message_raw"));
}

size_t MPTWrap::stream_read(void *instance, void *buf, std::size_t n)
{
    return VFS(instance)->read(reinterpret_cast<char *>(buf), n);
}

int MPTWrap::stream_seek(void *instance, std::int64_t offset, int whence)
{
    qint64 pos;

    if (VFS(instance)->isSequential()) {
        return -1;
    }

    switch (whence) {
    case OPENMPT_STREAM_SEEK_SET:
        pos = offset;
        break;
    case OPENMPT_STREAM_SEEK_CUR:
        pos = VFS(instance)->pos() + offset;
        break;
    case OPENMPT_STREAM_SEEK_END:
        pos = VFS(instance)->size() + offset;
        break;
    default:
        return -1;
    }

    return VFS(instance)->seek(pos) ? 0 : -1;
}

std::int64_t MPTWrap::stream_tell(void *instance)
{
    return VFS(instance)->isSequential() ? -1 : VFS(instance)->pos();
}

std::vector<MPTWrap::Interpolator> MPTWrap::get_interpolators()
{
    return {
        Interpolator(QObject::tr("None").toUtf8().constData(), interp_none),
        Interpolator(QObject::tr("Linear").toUtf8().constData(), interp_linear),
        Interpolator(QObject::tr("Cubic").toUtf8().constData(), interp_cubic),
        Interpolator(QObject::tr("Windowed sinc").toUtf8().constData(), interp_windowed),
    };
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
    if (is_valid_interpolator(interpolator_value)) {
        openmpt_module_set_render_param(m_mod.get(), OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH, interpolator_value);
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
    if (is_valid_stereo_separation(separation)) {
        openmpt_module_set_render_param(m_mod.get(), OPENMPT_MODULE_RENDER_STEREOSEPARATION_PERCENT, separation);
    }
}

std::int64_t MPTWrap::read(void *buf, std::int64_t bufsiz)
{
    bufsiz /= sizeof(float) * channels();
    std::size_t n;

    n = openmpt_module_read_interleaved_float_stereo(m_mod.get(), rate(), bufsiz, reinterpret_cast<float *>(buf));

    return n * channels() * sizeof(float);
}

void MPTWrap::seek(int pos)
{
    openmpt_module_set_position_seconds(m_mod.get(), pos / 1000.0);
}
