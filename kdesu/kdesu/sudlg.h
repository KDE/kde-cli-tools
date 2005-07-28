/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __SuDlg_h_Included__
#define __SuDlg_h_Included__

#include <kpassdlg.h>
//Added by qt3to4:
#include <Q3CString>

class KDEsuDialog
    : public KPasswordDialog
{
    Q_OBJECT

public:
    KDEsuDialog(Q3CString user, Q3CString auth_user, bool enableKeep, const QString& icon );
    ~KDEsuDialog();

    enum ResultCodes { AsUser = 10 };
    
protected:
    bool checkPassword(const char *password);
    void slotUser1();
    
private:
    Q3CString m_User;
};
    

#endif // __SuDlg_h_Included__
