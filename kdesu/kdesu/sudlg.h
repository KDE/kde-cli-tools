/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __SuDlg_h_Included__
#define __SuDlg_h_Included__

#include <kpassdlg.h>

class KDEsuDialog
    : public KPasswordDialog
{
    Q_OBJECT

public:
    KDEsuDialog(QCString user, QCString auth_user, bool enableKeep);
    ~KDEsuDialog();

    enum ResultCodes { AsUser = 10 };
    
protected:
    bool checkPassword(const char *password);
    void slotUser1();
    
private:
    QCString m_User;
};
    

#endif // __SuDlg_h_Included__
