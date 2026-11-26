/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2002 Daniel Molkentin <molkentin@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef _KSERVICELISTWIDGET_H
#define _KSERVICELISTWIDGET_H

#include <QGroupBox>
#include <QListWidget>

#include <kservice.h>

class QPushButton;

class MimeTypeData;
class KService;

class KServiceListItem : public QListWidgetItem
{
public:
    explicit KServiceListItem(const KService::Ptr &pService);
    QString storageId;
    QString desktopPath;
    QString localPath;
};

/**
 * This widget holds the list of applications associated with a mimetype,
 * with buttons to manage it.
 */
class KServiceListWidget : public QGroupBox
{
    Q_OBJECT
public:
    explicit KServiceListWidget(QWidget *parent = nullptr);

    void setMimeTypeData(MimeTypeData *item);

Q_SIGNALS:
    void changed(bool);

protected Q_SLOTS:
    void promoteService();
    void demoteService();
    void addService();
    void editService();
    void removeService();
    void enableMoveButtons();

protected:
    void updatePreferredServices();

private:
    QListWidget *servicesLB;
    QPushButton *servUpButton, *servDownButton;
    QPushButton *servNewButton, *servEditButton, *servRemoveButton;
    MimeTypeData *m_mimeTypeData;
};

#endif
