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

#include <kdesu/su.h>
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
    int status = proc.checkInstall(password);
    switch (status)
    {
    case -1:
	KMessageBox::sorry(this, i18n("Conversation with su failed."));
	done(Rejected);
	return false;

    case 0:
	return true;

    case SuProcess::SuNotFound:
        KMessageBox::sorry(this, 
		i18n("The program `su' is not found!\n\n"
		     "Make sure your PATH is set correctly."));
	done(Rejected);
	return false;

    case SuProcess::SuNotAllowed:
        KMessageBox::sorry(this, 
		i18n("You are not allowed to use `su'!\n\n"
		     "On some systems, you need to be in a special\n"
		     "group (often: wheel) to use this program."));
	done(Rejected);
	return false;

    case SuProcess::SuIncorrectPassword:
        KMessageBox::sorry(this, i18n("Incorrect password! Please try again."));
	return false;

    default:
        KMessageBox::error(this, i18n("Internal error: Illegal return from "
		"SuProcess::checkInstall()"));
	done(Rejected);
	return false;
    }
}

void KDEsuDialog::slotUser1()
{
    done(AsUser);
} 

#include "sudlg.moc"
