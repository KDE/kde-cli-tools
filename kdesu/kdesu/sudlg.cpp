/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include "sudlg.h"

#include <QByteArray>

#include <KDebug>
#include <KLocale>

KDEsuDialog::KDEsuDialog(QByteArray user, QByteArray authUser, bool enableKeep, const QString& icon, bool withIgnoreButton)
    : KPasswordDialog(0, enableKeep ? ShowKeepPassword : NoFlags,
                      withIgnoreButton ? User1 : NoDefault)
{
    if ( !icon.isEmpty() ) {
        setPixmap(DesktopIcon(icon));
    }

    if ( withIgnoreButton ) {
        setButtonText( User1, i18n("Ignore") );
    }

    QString superUserCommand = proc.superUserCommand();

    proc.setUser(authUser);

    setCaption(i18n("Run as %1", QString::fromLatin1(user)));

    QString prompt;
    if (proc.useUsersOwnPassword()) {
        prompt = i18n("Please enter your password below." );
    } else {
        if (authUser == "root") {
            if(withIgnoreButton)
	            prompt = "<qt>" + i18n("The action you requested needs <b>root privileges</b>. "
        	    "Please enter <b>root's</b> password below or click "
	            "Ignore to continue with your current privileges.") + "</qt>";
	    else
	            prompt = "<qt>" + i18n("The action you requested needs <b>root privileges</b>. "
        	    "Please enter <b>root's</b> password below.") + "</qt>";
        } else {
            if(withIgnoreButton)
	            prompt = "<qt>" + i18n("The action you requested needs additional privileges. "
	                "Please enter the password for <b>%1</b> below or click "
        	        "Ignore to continue with your current privileges.", QString::fromLatin1(authUser)) +
	                "</qt>";
	    else
	            prompt = "<qt>" + i18n("The action you requested needs additional privileges. "
	                "Please enter the password for <b>%1</b> below.", QString::fromLatin1(authUser)) +
	                "</qt>";

        }
    }
    setPrompt(prompt);

    if( withIgnoreButton )
        setButtonText(User1, i18n("&Ignore"));
    connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));

    setMinimumSize(minimumSizeHint());
}


KDEsuDialog::~KDEsuDialog()
{
}

bool KDEsuDialog::checkPassword()
{
    int status = proc.checkInstall(password().toLocal8Bit());
    switch (status)
    {
    case -1:
        showErrorMessage(i18n("Conversation with su failed."), UsernameError);
        return false;

    case 0:
        return true;

    case SuProcess::SuNotFound:
        showErrorMessage(i18n("The program 'su' could not be found.<br />"
                             "Ensure your PATH is set correctly."), FatalError);
        return false;

    case SuProcess::SuNotAllowed:
        // This is actually never returned, as kdesu cannot tell the difference.
        showErrorMessage(QLatin1String("The impossible happened."), FatalError);
        return false;

    case SuProcess::SuIncorrectPassword:
        showErrorMessage(i18n("Permission denied.<br />"
                              "Possibly incorrect password, please try again.<br />"
                              "On some systems, you need to be in a special "
                              "group (often: wheel) to use this program."), PasswordError);
        return false;

    default:
        showErrorMessage(i18n("Internal error: illegal return from "
                             "SuProcess::checkInstall()"), FatalError);
        done(Rejected);
        return false;
    }
}

void KDEsuDialog::slotUser1()
{
    done(AsUser);
}

#include "sudlg.moc"
