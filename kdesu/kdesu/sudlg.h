/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __SuDlg_h_Included__
#define __SuDlg_h_Included__

#include <k3passworddialog.h>
#include <QByteArray>

class KDEsuDialog
    : public K3PasswordDialog
{
    Q_OBJECT

public:
    KDEsuDialog(QByteArray user, QByteArray auth_user, bool enableKeep, const QString& icon , bool withIgnoreButton);
    ~KDEsuDialog();

    enum ResultCodes { AsUser = 10 };

private slots:
    void slotUser1();
protected:
    bool checkPassword(const char *password);

private:
    QByteArray m_User;
};


#endif // __SuDlg_h_Included__
