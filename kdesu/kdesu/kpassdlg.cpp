/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * kpassdlg.cpp: Password input dialog for kdesu.
 */

#include <config.h>

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsize.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qcheckbox.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kaboutdialog.h>

#include "kdesu.h"
#include "kpassdlg.h"
#include "process.h"


KPasswordEdit::KPasswordEdit(QWidget *parent, const char *name)
    : QLineEdit(parent, name), m_EchoMode(OneStar)
{
    // I Keep the password in a char * instead of a QString.
    // I don't know what's happening inside a QString and I
    // cannot let the password lie around. I want to be sure
    // that when I delete it, it is deleted wholly.
    m_Password = new char[PassLen];
    m_Password[0] = '\000';
    m_length = 0;
}


KPasswordEdit::~KPasswordEdit()
{
    for (int i=0; i<PassLen; i++)
	m_Password[i] = '\000';
    delete[] m_Password;
}


void KPasswordEdit::erase()
{
    m_length = 0;
    for (int i=0; i<PassLen; i++)
	m_Password[i] = '\000';
    setText("");
}


void KPasswordEdit::keyPressEvent(QKeyEvent *e)
{    
    switch (e->key()) {
    case Key_Return:
    case Key_Escape:
	e->ignore();
	break;
    case Key_Backspace:
	if (m_length) {
	    m_Password[--m_length] = '\000';
	    showPass();
	}
	break;
    default:
	if (m_length < PassLen) {
	    m_Password[m_length] = (char) e->ascii();
	    m_Password[++m_length] = '\000';
	    showPass();
	}
	break;
    }
}

void KPasswordEdit::showPass()
{    
    QString tmp;

    switch (m_EchoMode) {
    case OneStar:
	tmp.fill('*', m_length);
	setText(tmp);
	break;
    case ThreeStars:
	tmp.fill('*', m_length*3);
	setText(tmp);
	break;
    case NoStars: 
    default:
	break;
    }
}


KDEsuDialog::KDEsuDialog(QString command, QString user, int keep)
    : KDialogBase(0L, "KDE su Dialog", true, user, Ok|Cancel|User1,
	    Ok, true, "&Ignore")
{
    m_User = user;
    m_Command = command;
    m_Keep = keep;

    QString help;
    if (m_User == "root")
	help = i18n("The action you requested needs root priviliges.\n"
		"Please enter root's password below or click\n"
		"Ignore to continue with your current priviliges.");
    else
	help = i18n("The action you requested needs additional priviliges.\n"
		"Please enter the password for \"%1\" below or click\n"
		"Ignore to continue with your current privileges.").arg(m_User);

    QWidget *main = new QWidget(this);
    setMainWidget(main);
    QGridLayout *grid = new QGridLayout(main, 4, 3, 10, 10);

    // Pixmap + Informational text
    QLabel *lbl;
    QPixmap pix(BarIcon("kdesu-keys"));
    if (!pix.isNull()) {
	lbl = new QLabel(main);
	lbl->setPixmap(pix);
	lbl->setFixedSize(lbl->sizeHint());
	grid->addWidget(lbl, 0, 0, AlignCenter);
    }

    lbl = new QLabel(main);
    lbl->setAlignment(AlignLeft);
    lbl->setText(help);
    QSize size = lbl->sizeHint();
    size.setWidth(size.width()+20);
    lbl->setFixedSize(size);
    grid->addWidget(lbl, 0, 2, AlignLeft|AlignVCenter);

    // Command label
    lbl = new QLabel(main);
    lbl->setText(i18n("Command:"));
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, AlignLeft);

    lbl = new QLabel(main);
    // By making it richtext, the lines are wrapped if they are too long
    lbl->setText(QString("<qt>") + m_Command + "</qt>");
    lbl->setFixedWidth(size.width());
    grid->addWidget(lbl, 1, 2, AlignLeft);
	    
    // Password label + line editor
    lbl = new QLabel(main);
    lbl->setAlignment(AlignLeft|AlignVCenter);
    lbl->setText(i18n("%1's\n&Password:").arg(m_User));
    lbl->setFixedSize(lbl->sizeHint());
    lbl->setBuddy(main);
    grid->addWidget(lbl, 2, 0, AlignLeft);
    
    QHBoxLayout *h_lay = new QHBoxLayout();
    grid->addLayout(h_lay, 2, 2);
    m_pEdit = new KPasswordEdit(main);
    size = m_pEdit->sizeHint();
    m_pEdit->setFixedHeight(size.height());
    m_pEdit->setMinimumWidth(size.width());
    h_lay->addWidget(m_pEdit, 12);
    h_lay->addStretch(6);

    // Keep password checkbox
    if (m_Keep) {
	QCheckBox *cb = new QCheckBox(i18n("&Keep Password"), main);
	size = cb->sizeHint();
	cb->setFixedSize(size);
	if (m_Keep > 1) {
	    cb->setChecked(true);
	    m_Keep = 1;
	} else
	    m_Keep = 0;
	connect(cb, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
	grid->addMultiCellWidget(cb, 3, 3, 0, 2, AlignLeft|AlignVCenter);
    } 

    m_pEdit->setFocus();
}


KDEsuDialog::~KDEsuDialog()
{
}


void KDEsuDialog::slotOk()
{
    SuProcess proc;
    proc.setTerminal(true);
    proc.setUser(m_User.latin1());
    int ret = proc.checkPass(m_pEdit->getPass());
    if (ret < 0) {
        KMessageBox::sorry(this, i18n("Incorrect password! Please try again."));
        m_pEdit->erase();
        m_pEdit->setFocus();
    } else
        accept();
}


void KDEsuDialog::slotCancel()
{
    reject();
}


void KDEsuDialog::slotUser1()
{
    done(AsUser);
}


void KDEsuDialog::slotKeep(bool keep)
{
    m_Keep = keep;
}
