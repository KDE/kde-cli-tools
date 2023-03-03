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

#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 102)
KService::List allParts()
{
    KService::List result;
    const KService::List allServices = KService::allServices();
    for (const auto &service : allServices) {
        if (service->hasServiceType(QStringLiteral("KParts/ReadOnlyPart"))) {
            result << service;
        }
    }
    return result;
}
#else
QVector<KPluginMetaData> allParts()
{
    return KPluginMetaData::findPlugins(QStringLiteral("kf" QT_STRINGIFY(QT_VERSION_MAJOR) "/parts"));
}
#endif

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

    const auto parts = allParts();

    for (const auto &part : parts) {
#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 102)
        m_listbox->addItem(new KServiceListItem(part, KServiceListWidget::SERVICELIST_SERVICES));
#else
        m_listbox->addItem(new PluginListItem(part));
#endif
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
