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

#include <QDialog>
#include <QPair>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    m_ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    for (const auto &interpolator : m_settings.get_interpolators()) {
        m_ui.interpolate_combo->addItem(interpolator.first, interpolator.second);
    }

    set_interpolator(m_settings.get_interpolator());

    m_ui.stereo_separation->setSliderPosition(m_settings.get_stereo_separation());

    m_ui.use_filename->setChecked(m_settings.get_use_filename());
}

void SettingsDialog::accept()
{
    m_settings.set_interpolator(m_ui.interpolate_combo->itemData(m_ui.interpolate_combo->currentIndex()).toInt());
    m_settings.set_stereo_separation(m_ui.stereo_separation->value());
    m_settings.set_use_filename(m_ui.use_filename->isChecked());

    QDialog::accept();
}

void SettingsDialog::restore_defaults()
{
    set_interpolator(m_settings.default_interpolator());
    m_ui.stereo_separation->setSliderPosition(m_settings.default_stereo_separation());
    m_ui.use_filename->setChecked(m_settings.default_use_filename());
}

void SettingsDialog::set_interpolator(int interpolator)
{
    int i = m_ui.interpolate_combo->findData(interpolator);
    if (i != -1) {
        m_ui.interpolate_combo->setCurrentIndex(i);
    }
}
