/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <assert.h>
#include <stdlib.h>
#include <iostream>

#include <qwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qslider.h>

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>

#include "kcmkdesu.h"
#include "kdesu.h"
#include "client.h"


KDEsuConfig::KDEsuConfig(QWidget *parent, const char *name)
    : KConfigWidget(parent, name)
{
    QVBoxLayout *top_lay = new QVBoxLayout(this, 10);

    // Echo mode
    em_group = new QButtonGroup(i18n("Echo mode"), this);
    QVBoxLayout *v_lay = new QVBoxLayout(em_group, 10);
    v_lay->addSpacing(10);

    QRadioButton *rb = new QRadioButton(i18n("One star"), em_group);
    v_lay->addStretch(4);
    rb->setFixedSize(rb->sizeHint());
    v_lay->addWidget(rb, 0, AlignLeft);
    v_lay->addStretch(12);
    rb = new QRadioButton(i18n("Three stars"), em_group);
    rb->setFixedSize(rb->sizeHint());
    v_lay->addWidget(rb, 0, AlignLeft);
    v_lay->addStretch(12);
    rb = new QRadioButton(i18n("No echo"), em_group);
    rb->setFixedSize(rb->sizeHint());
    v_lay->addWidget(rb, 0, AlignLeft);
    v_lay->addStretch(4);
    v_lay->activate();

    em_group->setMinimumSize(em_group->childrenRect().size());
    connect(em_group, SIGNAL(clicked(int)), SLOT(slotEchoMode(int)));

    top_lay->addWidget(em_group, 3);
    top_lay->addStretch(3);

    // Keep password
    QHBoxLayout *h_lay = new QHBoxLayout();
    top_lay->addLayout(h_lay);

    keep_but = new QCheckBox(i18n("&Remember password for"), this);
    keep_but->setFixedSize(keep_but->sizeHint());
    connect(keep_but, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
    h_lay->addWidget(keep_but);

    timeout_edit = new QLineEdit(this);
    timeout_edit->setText("999999");
    timeout_edit->setFixedSize(timeout_edit->sizeHint());
    timeout_edit->setText("");
    h_lay->addWidget(timeout_edit);

    QLabel *lbl = new QLabel(i18n("minutes."), this);
    lbl->setFixedSize(lbl->sizeHint());
    h_lay->addWidget(lbl);
    h_lay->addStretch(12);

    top_lay->addStretch(12);

    config = new KConfig("kdesurc", "kdesurc");

    loadSettings();
}


KDEsuConfig::~KDEsuConfig()
{
}

void KDEsuConfig::loadSettings()
{
    config->setGroup("Common");

    QString val = config->readEntry("EchoMode", "x");
    if (val == "OneStar")
	echo_mode = OneStar;
    else if (val == "ThreeStars")
	echo_mode = ThreeStars;
    else if (val == "NoStars")
	echo_mode = NoStars;
    else
	echo_mode = defEchomode;

    keep_pw = config->readBoolEntry("KeepPassword", defKeep);
    pw_timeout = config->readNumEntry("KeepPasswordTimeout", defTimeout);

    updateSettings();
}


void KDEsuConfig::applySettings()
{
    config->setGroup("Common");

    QString val;
    if (echo_mode == OneStar)
	val = "OneStar";
    else if (echo_mode == ThreeStars)
	val = "ThreeStars";
    else if (echo_mode == NoStars)
	val = "NoStars";
    assert(!val.isNull());
    config->writeEntry("EchoMode", val);

    config->writeEntry("KeepPassword", keep_pw);
    pw_timeout = atoi(timeout_edit->text())*60;
    config->writeEntry("KeepPasswordTimeout", pw_timeout);

    config->sync();

    if (!keep_pw) {
	// Try to stop daemon
	KDEsuClient client;
	if (client.ping() != -1)
	    client.stopServer();
    }
}

void KDEsuConfig::defaultSettings()
{
    echo_mode = defEchomode;
    keep_pw = defKeep;
    pw_timeout = defTimeout;

    updateSettings();
}

void KDEsuConfig::updateSettings()
{
    em_group->setButton(echo_mode);
    keep_but->setChecked(keep_pw);

    QString tmp;
    tmp.setNum(pw_timeout/60);
    timeout_edit->setText(tmp);
    timeout_edit->setEnabled(keep_pw);
}
    
void KDEsuConfig::slotEchoMode(int i)
{
    echo_mode = i;
}

void KDEsuConfig::slotKeep(bool _keep)
{
    keep_pw = _keep;
    updateSettings();
}

//
// KDEsuApplication
//

KDEsuApplication::KDEsuApplication(int &argc, char *argv[], const char *name) : 
    KControlApplication(argc, argv, name)
{
    kdesu = new KDEsuConfig(dialog, "kdesu");

    addPage(kdesu, "KDE su", "kcmkdesu-1.html");
    dialog->show();
}

void KDEsuApplication::init()
{
    kdesu->loadSettings();
}


void KDEsuApplication::apply()
{
    kdesu->applySettings();
}

void KDEsuApplication::defaultValues()
{
    kdesu->defaultSettings();
}

//
// Main application
//

int main(int argc, char **argv)
{
    KDEsuApplication app(argc, argv, "kcmkdesu");

    if (app.runGUI())
	return app.exec();
    else
	return 0;
}
