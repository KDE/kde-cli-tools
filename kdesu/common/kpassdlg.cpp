/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * kpassdlg.cpp: Password input dialog.
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
#include "su.h"


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
	if ((m_length < PassLen) && e->ascii()) {
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


KPassDialog::KPassDialog(QString command, int keep, int extrabtn)
    : KDialogBase(0L, "Password Dialog", true, "", Ok|Cancel|extrabtn,
	    Ok, true, "&Ignore")
{
    m_Command = command;
    m_Keep = keep;

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

    // Text will be set by setHelpText()
    m_pHelpLbl = new QLabel(main);
    m_pHelpLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
    grid->addWidget(m_pHelpLbl, 0, 2, AlignLeft);

    // Command label
    lbl = new QLabel(main);
    lbl->setText(i18n("Command:"));
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, AlignLeft);

    m_pCommandLbl = new QLabel(main);
    m_pCommandLbl->setAlignment(AlignLeft|AlignVCenter|WordBreak);
    m_pCommandLbl->setText(m_Command);
    m_pCommandLbl->setFixedSize(275, m_pCommandLbl->heightForWidth(275));
    grid->addWidget(m_pCommandLbl, 1, 2, AlignLeft);
	    
    // Password label + line editor
    lbl = new QLabel(main);
    lbl->setAlignment(AlignLeft|AlignVCenter);
    lbl->setText(i18n("&Password:"));
    lbl->setFixedSize(lbl->sizeHint());
    lbl->setBuddy(main);
    grid->addWidget(lbl, 2, 0, AlignLeft);
    
    QHBoxLayout *h_lay = new QHBoxLayout();
    grid->addLayout(h_lay, 2, 2);
    m_pEdit = new KPasswordEdit(main);
    QSize size = m_pEdit->sizeHint();
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


KPassDialog::~KPassDialog()
{
}


void KPassDialog::setHelpText(QString text)
{
    m_pHelpLbl->setText(text);
    m_pHelpLbl->setFixedSize(275, m_pHelpLbl->heightForWidth(275));
}


void KPassDialog::slotOk()
{
    if (!checkPass(m_pEdit->getPass())) {
        KMessageBox::sorry(this, i18n("Incorrect password! Please try again."));
        m_pEdit->erase();
        m_pEdit->setFocus();
    } else
        accept();
}


void KPassDialog::slotCancel()
{
    reject();
}


void KPassDialog::slotKeep(bool keep)
{
    m_Keep = keep;
}
