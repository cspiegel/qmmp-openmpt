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

#include <QFile>
#include <QHash>
#include <QLatin1Char>
#include <QObject>
#include <QString>
#include <QtGui/qtextdocument.h>

#include <qmmp/metadatamodel.h>

#include "metadatamodel.h"
#include "mptwrap.h"

MPTMetaDataModel::MPTMetaDataModel(const QString &path) :
    MetaDataModel(true)
{
    QFile file(path);

    if (file.open(QIODevice::ReadOnly)) {
        try {
            MPTWrap mpt(&file);
            fill_in_extra_properties(mpt);
            fill_in_descriptions(mpt);
        } catch (const MPTWrap::InvalidFile &) {
        }
    }
}

void MPTMetaDataModel::fill_in_extra_properties(MPTWrap &mpt)
{
    QString text;
    auto is_empty_string = [](const std::string &s) { return s == ""; };

    if (!std::all_of(mpt.instruments().begin(), mpt.instruments().end(), is_empty_string)) {
        for (const std::string &s : mpt.instruments()) {
            text += QString::fromStdString(s) + "\n";
        }
        m_desc << MetaDataItem(tr("Instruments"), text);
    }

    if (!std::all_of(mpt.samples().begin(), mpt.samples().end(), is_empty_string)) {
        text = "";
        for (const std::string &s : mpt.samples()) {
            text += QString::fromStdString(s) + "\n";
        }
        m_desc << MetaDataItem(tr("Samples"), text);
    }

    if (!mpt.comment().empty()) {
        m_desc << MetaDataItem(tr("Comment"), QString::fromStdString(mpt.comment()));
    }
}

void MPTMetaDataModel::fill_in_descriptions(MPTWrap &mpt)
{
    m_ap << MetaDataItem(tr("Patterns"), QString::number(mpt.pattern_count()));
    m_ap << MetaDataItem(tr("Channels"), QString::number(mpt.channel_count()));
    m_ap << MetaDataItem(tr("Instruments"), QString::number(mpt.instruments().size()));
    m_ap << MetaDataItem(tr("Samples"), QString::number(mpt.samples().size()));
}

MPTMetaDataModel::~MPTMetaDataModel()
{
}

QList<MetaDataItem> MPTMetaDataModel::extraProperties() const
{
    return m_ap;
}

QList<MetaDataItem> MPTMetaDataModel::descriptions() const
{
    return m_desc;
}
