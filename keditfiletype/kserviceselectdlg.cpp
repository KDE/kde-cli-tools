/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kserviceselectdlg.h"
#include "kservicelistwidget.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

#include <KLocalizedString>

KServiceSelectDlg::KServiceSelectDlg(const QString & /*serviceType*/, const QString & /*value*/, QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QLatin1String("serviceSelectDlg"));
    setModal(true);
    setWindowTitle(i18n("Add Service"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(i18n("Select service:")));
    m_listbox = new QListWidget();
    m_buttonBox = new QDialogButtonBox;
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Can't make a KTrader query since we don't have a servicetype to give,
    // we want all services that are not applications.......
    // So we have to do it the slow way
    // ### Why can't we query for KParts/ReadOnlyPart as the servicetype? Should work fine!
    const KService::List allServices = KService::allServices();
    for (const auto &service : allServices) {
        if (service->hasServiceType(QStringLiteral("KParts/ReadOnlyPart"))) {
            m_listbox->addItem(new KServiceListItem(service, KServiceListWidget::SERVICELIST_SERVICES));
        }
    }

    m_listbox->model()->sort(0);
    m_listbox->setMinimumHeight(350);
    m_listbox->setMinimumWidth(400);
    layout->addWidget(m_listbox);
    layout->addWidget(m_buttonBox);
    connect(m_listbox, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

KServiceSelectDlg::~KServiceSelectDlg()
{
}

KService::Ptr KServiceSelectDlg::service()
{
    int selIndex = m_listbox->currentRow();
    KServiceListItem *selItem = static_cast<KServiceListItem *>(m_listbox->item(selIndex));
    return KService::serviceByDesktopPath(selItem->desktopPath);
}
