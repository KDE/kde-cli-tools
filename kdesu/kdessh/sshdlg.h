/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __SshDlg_h_Included__
#define __SshDlg_h_Included__

#include <kpassdlg.h>

class KDEsshDialog
    : public KPasswordDialog
{
    Q_OBJECT

public:
    KDEsshDialog(QCString host, QCString user, QCString command, 
	    QString prompt, bool enableKeep);
    ~KDEsshDialog();

    static int getPassword(QCString &password, QCString host, QCString user,
	    QCString command, QString prompt, int *keep);

protected:
    bool checkPassword(const char *password);
    
private:
    QCString m_User, m_Host;
};
    

#endif // __SshDlg_h_Included__
