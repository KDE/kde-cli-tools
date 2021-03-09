/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef __kserviceselectdlg_h
#define __kserviceselectdlg_h

#include <QDialog>
#include <QListWidget>

#include <kservice.h>

class QDialogButtonBox;

class KServiceSelectDlg : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create a dialog to select a service (not application) for a given service type.
     *
     * @param serviceType the service type we want to choose a service for.
     * @param value is the initial service to select (not implemented currently)
     * @param parent parent widget
     */
    explicit KServiceSelectDlg(const QString &serviceType, const QString &value = QString(), QWidget *parent = nullptr);

    ~KServiceSelectDlg() override;

    /**
     * @return the chosen service
     */
    KService::Ptr service();

private:
    QListWidget *m_listbox;
    QDialogButtonBox *m_buttonBox;
};

#endif
