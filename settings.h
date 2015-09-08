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

#ifndef QMMP_MPT_SETTINGS_H
#define QMMP_MPT_SETTINGS_H

#include <QList>
#include <QPair>
#include <QSettings>
#include <QString>

#include <qmmp/qmmp.h>

#include "mptwrap.h"

class MPTSettings
{
  public:
    MPTSettings() : settings(new QSettings(Qmmp::configFile(), QSettings::IniFormat))
    {
      for(const MPTWrap::Interpolator &interpolator : MPTWrap::get_interpolators())
      {
        interpolators.append(QPair<QString, int>(QString::fromStdString(interpolator.name), interpolator.value));
      }

      settings->beginGroup("cas-openmpt-plugin");
    }

    ~MPTSettings()
    {
      settings->endGroup();
      delete settings;
    }

    const QList<QPair<QString, int>> get_interpolators()
    {
      return interpolators;
    }

    int get_interpolator()
    {
      int interpolator = settings->value("interpolator", MPTWrap::default_interpolator()).toInt();

      return MPTWrap::is_valid_interpolator(interpolator) ? interpolator : default_interpolator();
    }

    void set_interpolator(int value)
    {
      if(MPTWrap::is_valid_interpolator(value))
      {
        settings->setValue("interpolator", value);
      }
    }

    int default_interpolator()
    {
      return MPTWrap::default_interpolator();
    }

    int get_stereo_separation()
    {
      int separation = settings->value("stereo_separation", default_stereo_separation()).toInt();

      if(!MPTWrap::is_valid_stereo_separation(separation)) separation = default_stereo_separation();

      return separation;
    }

    void set_stereo_separation(int value)
    {
      if(MPTWrap::is_valid_stereo_separation(value))
      {
        settings->setValue("stereo_separation", value);
      }
    }

    int default_stereo_separation()
    {
      return MPTWrap::default_stereo_separation();
    }

    bool get_use_filename()
    {
      return settings->value("use_filename", default_use_filename()).toBool();
    }

    void set_use_filename(bool use)
    {
      settings->setValue("use_filename", use);
    }

    bool default_use_filename()
    {
      return false;
    }

  private:
    MPTSettings(const MPTSettings &);
    MPTSettings &operator=(const MPTSettings &);

    QSettings *settings;
    QList<QPair<QString, int>> interpolators;
};

#endif
