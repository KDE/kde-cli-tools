/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <qstring.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "ssh.h"
#include "sshdlg.h"


KDEsshDialog::KDEsshDialog(QCString host, QCString user, QCString command, 
	QString prompt, bool enableKeep)
    : KPasswordDialog(Password, "", command, enableKeep)
{
    m_Host = host;
    m_User = user;

    setCaption(i18n("%1@%2").arg(m_User).arg(m_Host));

    // Make the prompt a little more polite :-)
    if (!strnicmp(prompt, "Enter ", 6))
	prompt.remove(0, 6);
    int pos = prompt.find(':');
    if (pos != -1)
	prompt.remove(pos, 10);
    prompt += '.';
    prompt.prepend(i18n("The action you requested needs authentication. "
	    "Please enter "));
    setPrompt(prompt);
}


KDEsshDialog::~KDEsshDialog()
{
}


bool KDEsshDialog::checkPassword(const char *password)
{
    SshProcess proc(m_Host, m_User);
    int ret = proc.checkInstall(password);
    if (ret != 0) {
        KMessageBox::sorry(this, i18n("Incorrect password! Please try again."));
	return false;
    }
    return true;
}


// static
int KDEsshDialog::getPassword(QCString &password, QCString host, 
	QCString user, QCString command, QString prompt, int *keep)
{
    bool enableKeep = keep && *keep;
    KDEsshDialog *dlg = new KDEsshDialog(host, user, command, prompt,
	    enableKeep);
    int res = dlg->exec();
    if (res == Accepted) {
	password = dlg->password();
	if (enableKeep)
	    *keep = dlg->keep();
    }
    delete dlg;
    return res;
}


#include "sshdlg.moc"
