/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * kpassdlg.cpp: A password input dialog for kdesu.
 */

#include <config.h>

#include <string.h>

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsize.h>
#include <qevent.h>
#include <qkeycode.h>
#include <qmsgbox.h>
#include <qcheckbox.h>
#include <qdir.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kdesu.h"
#include "kpassdlg.h"
#include "process.h"


KPasswordEdit::KPasswordEdit(QWidget *parent, const char *name)
    : QLineEdit(parent, name), mEcho(OneStar)
{
    // I Keep the password in a char * instead of a QString.
    // I don't know what's happening inside a QString and I
    // cannot let the password lie around. I want to be sure
    // that when I delete it, it is deleted wholly.
    mPassword = new char[PassLen];
    strcpy(mPassword, "");
}


KPasswordEdit::~KPasswordEdit()
{
    for (unsigned i=0; i<strlen(mPassword); i++)
	mPassword[i] = 'x';
    delete[] mPassword;
}


void KPasswordEdit::erase()
{
    for (unsigned i=0; i<strlen(mPassword); i++)
	mPassword[i] = 'x';
    strcpy(mPassword, "");
    setText("");
}


void KPasswordEdit::keyPressEvent(QKeyEvent *e)
{    
    int len = strlen(mPassword);

    switch (e->key()) {
    case Key_Backspace:
	if (len) {
	    mPassword[len-1] = '\000';
	    showit();
	}
	break;
    case Key_Return:
	e->ignore();
	break;
    case Key_Escape:
	e->ignore();
	break;
    default:
	if (len < PassLen) {
	    mPassword[len] = (char) e->ascii();
	    mPassword[len+1] = '\000';
	    showit();
	}
	break;
    }
}

void KPasswordEdit::showit()
{    
    int len = strlen(mPassword);
    QString tmp;

    switch (mEcho) {
    case OneStar:
	tmp.fill('*', len);
	setText(tmp);
	break;
    case ThreeStars:
	tmp.fill('*', len*3);
	setText(tmp);
	break;
    case NoStars: default:
	break;
    }
}


KPasswordDlg::KPasswordDlg(const QString& help, const QString& command, 
	const QString& user, int keep, const QString& file, 
	const QString& topic, QWidget *parent, const char *name) :

	QDialog(parent, name, true), mHelp(help), mCommand(command), 
	mUser(user), mKeep(keep), mHelpfile(file), mHelptopic(topic)
{
    QString caption = "KDE su: ";
    caption += mUser;
    setCaption(caption);

    if (mHelp.isNull())
	mHelp = i18n("Please enter root password");
		 
    QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
    QGridLayout *grid = new QGridLayout(5, 3);
    topLayout->addLayout(grid, 6);

    grid->setColStretch(0, 0);
    grid->setColStretch(1, 0);
    grid->setColStretch(2, 12);

    grid->setRowStretch(0, 12);
    grid->setRowStretch(1, 0);
    grid->addRowSpacing(1, 10);
    grid->setRowStretch(2, 12);
    grid->setRowStretch(3, 0);
    grid->setRowStretch(4, 12);


    // Pixmap + Informational text

    QLabel *lbl; QPixmap pix;
    KIconLoader *loader = KGlobal::iconLoader();
    pix = loader->loadIcon("kdesu-keys.xpm");
    if (!pix.isNull()) {
	lbl = new QLabel(this);
	lbl->setPixmap(pix);
	lbl->setFixedSize(lbl->sizeHint());
	grid->addWidget(lbl, 0, 0, AlignCenter);
    }

    lbl = new QLabel(this);
    lbl->setAlignment(AlignLeft);
    lbl->setText(help);
    QSize size = lbl->sizeHint();
    size.setWidth(size.width()+20);
    lbl->setFixedSize(size);
    grid->addWidget(lbl, 0, 2, AlignLeft|AlignVCenter);

    // Command label

    if (!command.isNull()) {
	lbl = new QLabel(this);
	lbl->setText(i18n("Command:"));
	lbl->setFixedSize(lbl->sizeHint());
	grid->addWidget(lbl, 2, 0, AlignLeft);

	lbl = new QLabel(this);
	lbl->setText(command);
	lbl->setFixedWidth(size.width());
	lbl->setFixedHeight(lbl->sizeHint().height());
	grid->addWidget(lbl, 2, 2, AlignLeft);
    }
	    
    // Password label + line editor

    lbl = new QLabel(this);
    lbl->setAlignment(AlignLeft|AlignVCenter);
    lbl->setText(i18n("&Password:"));
    lbl->setFixedSize(lbl->sizeHint());
    lbl->setBuddy(this);
    grid->addWidget(lbl, 4, 0, AlignLeft);
    
    QHBoxLayout *h_lay = new QHBoxLayout();
    grid->addLayout(h_lay, 4, 2);
    edit = new KPasswordEdit(this);
    size = edit->sizeHint();
    edit->setFixedHeight(size.height());
    edit->setMinimumWidth(size.width());

    h_lay->addWidget(edit, 12);
    h_lay->addStretch(6);

    // Keep password checkbox

    if (mKeep) {
	QCheckBox *cb = new QCheckBox(i18n("&Keep Password"), this);
	size = cb->sizeHint();
	cb->setFixedSize(size);
	if (mKeep > 1) {
	    cb->setChecked(true);
	    mKeep = 1;
	} else
	    mKeep = 0;
	connect(cb, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));

	topLayout->addWidget(cb, 0, AlignLeft|AlignVCenter);
    } 

    // Buttons

    h_lay = new QHBoxLayout(10);
    topLayout->addSpacing(10);
    topLayout->addStretch(12);
    topLayout->addLayout(h_lay, 0);

    // This string controls how wide the buttons are gonna be. 
    // Translators can change this
    QPushButton *but = new QPushButton(i18n("ABCDEF"), this);
    size = but->sizeHint();
    delete but;

    if (!mHelpfile.isNull()) {
	but = new QPushButton(i18n("&Help"), this);
	but->setFixedSize(size);
	connect(but, SIGNAL(clicked()), SLOT(slotHelp()));
	h_lay->addWidget(but);
    } 

    but = new QPushButton(i18n("&Cancel"), this);
    but->setFixedSize(size);
    connect(but, SIGNAL(clicked()), SLOT(slotCancel()));
    h_lay->addWidget(but);
    h_lay->addStretch(12);
    
    but = new QPushButton(i18n("&OK"), this);
    but->setFixedSize(size);
    but->setDefault(true);
    connect(but, SIGNAL(clicked()), SLOT(slotCheckPass()));
    h_lay->addWidget(but);
    
    but = new QPushButton(i18n("&Ignore"), this);
    but->setFixedSize(size);
    connect(but, SIGNAL(clicked()), SLOT(slotAsUser()));
    h_lay->addWidget(but);
    
    topLayout->activate();
    resize(minimumSize());

    edit->setFocus();
}


KPasswordDlg::~KPasswordDlg()
{
}


// checkPass() checks if the password is valid. This is done by checking
// the return value of "su -c /bin/true".

void KPasswordDlg::slotCheckPass()
{
    int ret;
    QString s;

    SuProcess proc;
    proc.setTerminal(true);
    proc.setUser(mUser.latin1());
    ret = proc.checkPass(edit->getPass());
    if (ret < 0) {
        QMessageBox::warning(this, kapp->caption(), 
                             i18n("Incorrect password!"), i18n("OK"));
        edit->erase();
        edit->setFocus();
    } else
        accept();
}

void KPasswordDlg::slotHelp()
{
    if (!kapp) 
	return;
    kapp->invokeHTMLHelp(mHelpfile, mHelptopic);
}

void KPasswordDlg::slotCancel()
{
    reject();
}

void KPasswordDlg::slotAsUser()
{
    done(AsUser);
}

void KPasswordDlg::slotKeep(bool keep)
{
    mKeep = keep;
}
