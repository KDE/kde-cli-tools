/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#include <qstring.h>
#include <klocale.h>

#include "su.h"
#include "sudlg.h"


KDEsuDialog::KDEsuDialog(QCString user, QCString command, int check)
    : KPassDialog(command, check, User1)
{
    m_User = user;
    setCaption(i18n("Run as %1").arg(m_User));

    QString help;
    if (m_User == "root")
	help = i18n("The action you requested needs root priviliges. "
		"Please enter root's password below or click "
		"Ignore to continue with your current priviliges.");
    else
	help = i18n("The action you requested needs additional priviliges. "
		"Please enter the password for \"%1\" below or click "
		"Ignore to continue with your current privileges.").arg(m_User);
    setHelpText(help);

    setButtonText(User1, i18n("&Ignore"));
}


KDEsuDialog::~KDEsuDialog()
{
}


bool KDEsuDialog::checkPass(const char *password)
{
    SuProcess proc;
    proc.setUser(m_User);
    int ret = proc.checkInstall(password);
    return (ret == 0);
}

void KDEsuDialog::slotUser1()
{
    done(AsUser);
} 
