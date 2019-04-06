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

#include <QFile>
#include <QIODevice>
#include <QList>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QtPlugin>

#include <qmmp/qmmp.h>

#include "decoderfactory.h"
#include "decoder.h"
#include "metadatamodel.h"
#include "settingsdialog.h"
#include "mptwrap.h"

bool MPTDecoderFactory::canDecode(QIODevice *device) const
{
  try
  {
    MPTWrap mpt(device);
    return true;
  }
  catch(const MPTWrap::InvalidFile &)
  {
    return false;
  }
}

DecoderProperties MPTDecoderFactory::properties() const
{
  DecoderProperties properties;

  properties.name = tr("OpenMPT Plugin");
  properties.description = tr("OpenMPT Module Files");
  properties.shortName = "cas-openmpt";
  properties.hasAbout = true;
  properties.hasSettings = true;
  properties.noInput = true;
  properties.protocols << "file";

  for(const auto &ext : QString(openmpt_get_supported_extensions()).split(";"))
  {
    properties.filters << QString("*.%1").arg(ext);
  }

  return properties;
}

Decoder *MPTDecoderFactory::create(const QString &, QIODevice *device)
{
  return new MPTDecoder(device);
}

QList<TrackInfo *> MPTDecoderFactory::createPlayList(const QString &filename, TrackInfo::Parts parts, QStringList *)
{
  QList<TrackInfo *> list;
  QFile file(filename);

  if(parts & (TrackInfo::MetaData | TrackInfo::Properties))
  {
    if(file.open(QIODevice::ReadOnly))
    {
      try
      {
        MPTWrap mpt(&file);
        TrackInfo *file_info = new TrackInfo(filename);

        if(parts & TrackInfo::Properties)
        {
          file_info->setValue(Qmmp::FORMAT_NAME, QString::fromStdString(mpt.format()));
          file_info->setDuration(mpt.duration());
        }

        if(parts & TrackInfo::MetaData)
        {
          if(settings.get_use_filename())
          {
            file_info->setValue(Qmmp::TITLE, filename.section('/', -1));
          }
          else if(!mpt.title().empty())
          {
            file_info->setValue(Qmmp::TITLE, QString::fromStdString(mpt.title()));
          }
        }

        list << file_info;
      }
      catch(const MPTWrap::InvalidFile &)
      {
      }
    }
  }

  return list;
}

MetaDataModel *MPTDecoderFactory::createMetaDataModel(const QString &path, bool)
{
  return new MPTMetaDataModel(path);
}

void MPTDecoderFactory::showSettings(QWidget *parent)
{
  SettingsDialog *d = new SettingsDialog(parent);
  d->show();
}

void MPTDecoderFactory::showAbout(QWidget *parent)
{
  QString title = tr("About OpenMPT Audio Plugin");
  QString text = tr("OpenMPT Audio Plugin");
  text += "\n";
  text += tr("Written by: Chris Spiegel <cspiegel@gmail.com>");

  QMessageBox::about(parent, title, text);
}

QString MPTDecoderFactory::translation() const
{
  return QString(":/cas-openmpt_plugin_");
}
