/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <qstring.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "su.h"
#include "sudlg.h"


KDEsuDialog::KDEsuDialog(QCString user, QCString command, bool enableKeep)
    : KPasswordDialog(Password, "", command, enableKeep, User1)
{
    m_User = user;
    setCaption(i18n("Run as %1").arg(m_User));

    QString prompt;
    if (m_User == "root")
	prompt = i18n("The action you requested needs root priviliges. "
		"Please enter root's password below or click "
		"Ignore to continue with your current priviliges.");
    else
	prompt = i18n("The action you requested needs additional priviliges. "
		"Please enter the password for \"%1\" below or click "
		"Ignore to continue with your current privileges.").arg(m_User);
    setPrompt(prompt);

    setButtonText(User1, i18n("&Ignore"));
}


KDEsuDialog::~KDEsuDialog()
{
}

bool KDEsuDialog::checkPassword(const char *password)
{
    SuProcess proc;
    proc.setUser(m_User);
    if (proc.checkInstall(password) < 0) {
        KMessageBox::sorry(this, i18n("Incorrect password! Please try again."));
	return false;
    }
    return true;
}

void KDEsuDialog::slotUser1()
{
    done(AsUser);
} 


//static
int KDEsuDialog::getPassword(QCString &password, QCString user, 
	QCString command, int *keep)
{
    bool enableKeep = keep && *keep;
    KDEsuDialog *dlg = new KDEsuDialog(user, command, enableKeep);
    int ret = dlg->exec();
    if (ret == Accepted) {
	password = dlg->password();
	if (enableKeep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return ret;
}


#include "sudlg.moc"
