/*
   SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

   SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef MAIN_H
#define MAIN_H

#include <KCMultiDialog>
#include <KPageDialog>
#include <QApplication>

/**
 * The application instance for kcmshell.
 */
class KCMShell : public QApplication
{
    Q_OBJECT

public:
    KCMShell(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    /**
     * Sets m_serviceName basically to @p serviceName,
     * and then registers with D-BUS.
     *
     * @param serviceName name to set the D-BUS name to
     */
    void setServiceName(const QString &serviceName);

    /**
     * Waits until the last instance of kcmshell with the same
     * module as this one exits, and then exits.
     */
    void waitForExit();

    /**
     * @return true if the shell is running
     */
    bool isRunning();

private Q_SLOTS:

    /**
     */
    void appExit(const QString &appId, const QString &, const QString &);

private:
    /**
     * The D-Bus name which actually is registered.
     * For example "kcmshell_mouse".
     */
    QString m_serviceName;
};

/**
 * Essentially a plain KCMultiDialog, but has the additional functionality
 * of allowing it to be told to request windows focus.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KCMShellMultiDialog : public KCMultiDialog
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KCMShellMultiDialog")

public:
    /**
     * Constructor. Parameter @p dialogFace is passed to KCMultiDialog
     * unchanged.
     */
    explicit KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent = nullptr);

public Q_SLOTS:

    /**
     * Activate a module with id @p asn_id . This is used when
     * black helicopters are spotted overhead.
     */
    virtual Q_SCRIPTABLE void activate(const QByteArray &asn_id);
};

// vim: sw=4 et sts=4
#endif // MAIN_H
