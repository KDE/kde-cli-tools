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


KDEsuDialog::KDEsuDialog(QCString user, QCString auth_user, bool enableKeep)
    : KPasswordDialog(Password, "", enableKeep, User1)
{
    m_User = auth_user;
    setCaption(i18n("Run as %1").arg(user));

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

#include "sudlg.moc"
