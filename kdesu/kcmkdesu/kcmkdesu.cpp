/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <qwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <qspinbox.h>
#include <qlabel.h>

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcmodule.h>

#include "kcmkdesu.h"
#include "kdesu.h"
#include "client.h"


/**
 * DLL interface.
 */
extern "C" {
    KCModule *create_kdesu(QWidget *parent, const char *name) {
	return new KDEsuConfig(parent, name);
    }
}


/**** KDEsuConfig ****/

KDEsuConfig::KDEsuConfig(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    QVBoxLayout *top = new QVBoxLayout(this, 10, 10);

    // Echo mode
    m_EMGroup = new QButtonGroup(i18n("Echo mode"), this);
    top->addWidget(m_EMGroup);
    QVBoxLayout *vbox = new QVBoxLayout(m_EMGroup, 10, 10);
    vbox->addSpacing(10);
    QRadioButton *rb = new QRadioButton(i18n("One star"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    rb = new QRadioButton(i18n("Three stars"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    rb = new QRadioButton(i18n("No echo"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    connect(m_EMGroup, SIGNAL(clicked(int)), SLOT(slotEchoMode(int)));

    // Keep password
    QHBoxLayout *hbox = new QHBoxLayout();
    top->addLayout(hbox);

    m_KeepBut = new QCheckBox(i18n("&Remember password for"), this);
    connect(m_KeepBut, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
    hbox->addWidget(m_KeepBut);
    m_TimeoutEdit = new QSpinBox(this);
    m_TimeoutEdit->setRange(5, 1200);
    m_TimeoutEdit->setSteps(5, 10);
    m_TimeoutEdit->setSuffix(i18n(" minutes"));
    m_TimeoutEdit->setFixedSize(m_TimeoutEdit->sizeHint());
    hbox->addWidget(m_TimeoutEdit);
    hbox->addStretch();

    top->addStretch();

    setButtons(buttons());
    m_pConfig = new KConfig("kdesurc");
    load();
}


KDEsuConfig::~KDEsuConfig()
{
    delete m_pConfig;
}


void KDEsuConfig::load()
{
    m_pConfig->setGroup("Common");

    QString val = m_pConfig->readEntry("EchoMode", "x");
    if (val == "OneStar")
	m_Echo = OneStar;
    else if (val == "ThreeStars")
	m_Echo = ThreeStars;
    else if (val == "NoStars")
	m_Echo = NoStars;
    else
	m_Echo = defEchomode;

    m_bKeep = m_pConfig->readBoolEntry("KeepPassword", defKeep);
    m_Timeout = m_pConfig->readNumEntry("KeepPasswordTimeout", defTimeout);

    apply();
    emit changed(false);
}


void KDEsuConfig::save()
{
    m_pConfig->setGroup("Common");

    QString val;
    if (m_Echo == OneStar)
	val = "OneStar";
    else if (m_Echo == ThreeStars)
	val = "ThreeStars";
    else 
	val = "NoStars";
    m_pConfig->writeEntry("EchoMode", val);

    m_pConfig->writeEntry("KeepPassword", m_bKeep);
    m_Timeout = m_TimeoutEdit->value()*60;
    m_pConfig->writeEntry("KeepPasswordTimeout", m_Timeout);

    m_pConfig->sync();

    if (!m_bKeep) {
	// Try to stop daemon
	KDEsuClient client;
	if (client.ping() != -1)
	    client.stopServer();
    }
    emit changed(false);
}


void KDEsuConfig::defaults()
{
    m_Echo = defEchomode;
    m_bKeep = defKeep;
    m_Timeout = defTimeout;

    apply();
    emit changed(true);
}


void KDEsuConfig::apply()
{
    m_EMGroup->setButton(m_Echo);
    m_KeepBut->setChecked(m_bKeep);

    m_TimeoutEdit->setValue(m_Timeout/60);
    m_TimeoutEdit->setEnabled(m_bKeep);
}
    

void KDEsuConfig::slotEchoMode(int i)
{
    m_Echo = i;
    emit changed(true);
}


void KDEsuConfig::slotKeep(bool keep)
{
    m_bKeep = keep;
    m_TimeoutEdit->setEnabled(m_bKeep);
    emit changed(true);
}


int KDEsuConfig::buttons()
{
    return KCModule::Help | KCModule::Default | KCModule::Reset |
	   KCModule::Cancel | KCModule::Ok;
}

