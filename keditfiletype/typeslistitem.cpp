/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003, 2007 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

// Own
#include "typeslistitem.h"

// Qt
#include <QDebug>

TypesListItem::TypesListItem(QTreeWidget *parent, const QString &major)
    : QTreeWidgetItem(parent)
    , m_mimetypeData(major)
{
    setText(0, major);
}

TypesListItem::TypesListItem(TypesListItem *parent, QMimeType mimetype)
    : QTreeWidgetItem(parent)
    , m_mimetypeData(mimetype)
{
    setText(0, m_mimetypeData.minorType());
}

TypesListItem::TypesListItem(TypesListItem *parent, const QString &newMimetype)
    : QTreeWidgetItem(parent)
    , m_mimetypeData(newMimetype, true)
{
    setText(0, m_mimetypeData.minorType());
}

TypesListItem::~TypesListItem()
{
}

void TypesListItem::setIcon(const QString &icon)
{
    m_mimetypeData.setUserSpecifiedIcon(icon);
    loadIcon(true);
}

void TypesListItem::loadIcon(bool forceReload)
{
    if ((!m_mimetypeData.icon().isEmpty() && icon(0).isNull()) || forceReload) {
        QTreeWidgetItem::setIcon(0, QIcon::fromTheme(m_mimetypeData.icon()));
    }
}
