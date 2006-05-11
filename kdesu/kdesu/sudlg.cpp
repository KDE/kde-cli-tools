/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <QString>
#include <klocale.h>
#include <kmessagebox.h>
#include <QByteArray>

#include <kdesu/su.h>
#include "sudlg.h"


KDEsuDialog::KDEsuDialog(QByteArray user, QByteArray auth_user, bool enableKeep, const QString& icon, bool withIgnoreButton)
    : KPasswordDialog(Password, enableKeep, withIgnoreButton?User1:NoDefault, icon)
{
    m_User = auth_user;
    setCaption(i18n("Run as %1", QString::fromLatin1(user)));

    QString prompt;
    if (m_User == "root")
    {
        if ( withIgnoreButton )
            prompt = i18n("The action you requested needs root privileges. "
                          "Please enter root's password below or click "
                          "Ignore to continue with your current privileges.");
        else
            prompt = i18n("The action you requested needs root privileges. "
                          "Please enter root's password below ");
    }
    else
	prompt = i18n("The action you requested needs additional privileges. "
		"Please enter the password for \"%1\" below or click "
		"Ignore to continue with your current privileges.", QString::fromLatin1(m_User));
    setPrompt(prompt);

    if( withIgnoreButton )
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
		i18n("The program 'su' is not found;\n"
		     "make sure your PATH is set correctly."));
	done(Rejected);
	return false;

    case SuProcess::SuNotAllowed:
        KMessageBox::sorry(this,
		i18n("You are not allowed to use 'su';\n"
		     "on some systems, you need to be in a special "
		     "group (often: wheel) to use this program."));
	done(Rejected);
	return false;

    case SuProcess::SuIncorrectPassword:
        KMessageBox::sorry(this, i18n("Incorrect password; please try again."));
	return false;

    default:
        KMessageBox::error(this, i18n("Internal error: illegal return from "
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
