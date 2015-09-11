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
#include <QHash>
#include <QLatin1Char>
#include <QObject>
#include <QString>
#include <QtGui/qtextdocument.h>

#include <qmmp/metadatamodel.h>

#include "metadatamodel.h"
#include "mptwrap.h"

MPTMetaDataModel::MPTMetaDataModel(const QString &path, QObject *parent) :
  MetaDataModel(parent)
{
  QFile file(path);

  if(file.open(QIODevice::ReadOnly))
  {
    try
    {
      MPTWrap mpt(&file);
      fill_in_audio_properties(mpt);
      fill_in_descriptions(mpt);
    }
    catch(MPTWrap::InvalidFile)
    {
    }
  }
}

void MPTMetaDataModel::fill_in_audio_properties(MPTWrap &mpt)
{
  QString text;

  for(const std::string &s : mpt.instruments())
  {
    text += QString::fromStdString(s) + "\n";
  }
  desc.insert(tr("Instruments"), text);

  text = "";
  for(const std::string &s : mpt.samples())
  {
    text += QString::fromStdString(s) + "\n";
  }
  desc.insert(tr("Samples"), text);

  if(!mpt.comment().empty())
  {
    desc.insert(tr("Comment"), QString::fromStdString(mpt.comment()));
  }
}

void MPTMetaDataModel::fill_in_descriptions(MPTWrap &mpt)
{
  ap.insert(tr("Title"), Qt::escape(QString::fromStdString(mpt.title())));
  ap.insert(tr("Format"), Qt::escape(QString::fromStdString(mpt.format())));
  ap.insert(tr("Patterns"), QString::number(mpt.pattern_count()));
  ap.insert(tr("Channels"), QString::number(mpt.channel_count()));
  ap.insert(tr("Instruments"), QString::number(mpt.instrument_count()));
  ap.insert(tr("Samples"), QString::number(mpt.sample_count()));

  int duration = mpt.duration() / 1000;
  if(duration < 60 * 60)
  {
    ap.insert(tr("Duration"), QString("%1:%2").
                                  arg(duration / 60).
                                  arg(duration % 60, 2, 10, QLatin1Char('0')));
  }
  else
  {
    ap.insert(tr("Duration"), QString("%1:%2:%3").
                                  arg(duration / 3600).
                                  arg((duration / 60) % 60, 2, 10, QLatin1Char('0')).
                                  arg(duration % 60, 2, 10, QLatin1Char('0')));
  }
}

MPTMetaDataModel::~MPTMetaDataModel()
{
}

QHash<QString, QString> MPTMetaDataModel::audioProperties()
{
  return ap;
}

QHash<QString, QString> MPTMetaDataModel::descriptions()
{
  return desc;
}
