/* This file is part of the KDE project
   Copyright 2000 Kurt Granroth <granroth@kde.org>
             2008 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 or at your option version 3 as published by
   the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Own
#include "newtypedlg.h"

// Qt
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QFormLayout>
#include <QLabel>
#include <QLayout>

// KDE
#include <klineedit.h>
#include <klocalizedstring.h>

NewTypeDialog::NewTypeDialog(const QStringList &groups, QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18n("Create New File Type"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout;

    QLabel *l = new QLabel(i18n("Group:"));

    m_groupCombo = new QComboBox;
    m_groupCombo->setEditable(true);
    m_groupCombo->addItems(groups);
    m_groupCombo->setCurrentIndex(m_groupCombo->findText(QStringLiteral("application"))); // certainly a better default than "all"
    formLayout->addRow(l, m_groupCombo);

    m_groupCombo->setWhatsThis(i18n("Select the category under which"
                                    " the new file type should be added."));

    // Line 1: mimetype name

    l = new QLabel(i18n("Type name:"));

    m_typeEd = new KLineEdit;
    formLayout->addRow(l, m_typeEd);

    m_typeEd->setWhatsThis(i18n(
                               "Type the name of the file type. For instance, if you selected 'image' as category and you type 'custom' here, the file type 'image/custom' will be created."));

    m_typeEd->setFocus();

    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Set a minimum width so that caption is not half-hidden
    setMinimumWidth(300);
}

QString NewTypeDialog::group() const
{
    return m_groupCombo->currentText();
}

QString NewTypeDialog::text() const
{
    return m_typeEd->text();
}
