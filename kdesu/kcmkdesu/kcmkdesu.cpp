/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
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
#include <kglobal.h>
#include <klocale.h>
#include <kcmodule.h>
#include <kpassdlg.h>

#include "defaults.h"
#include "kcmkdesu.h"
#include "client.h"


/**
 * DLL interface.
 */
extern "C" {
    KCModule *create_kdesu(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmkdesu");
	return new KDEsuConfig(parent, name);
    }
}


/**** KDEsuConfig ****/

KDEsuConfig::KDEsuConfig(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    QVBoxLayout *top = new QVBoxLayout(this, 10, 10);

    // Echo mode
    m_EMGroup = new QButtonGroup(i18n("Echo characters as"), this);
    top->addWidget(m_EMGroup);
    QVBoxLayout *vbox = new QVBoxLayout(m_EMGroup, 10, 10);
    vbox->addSpacing(10);
    QRadioButton *rb = new QRadioButton(i18n("1 star"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    rb = new QRadioButton(i18n("3 stars"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    rb = new QRadioButton(i18n("no echo"), m_EMGroup);
    vbox->addWidget(rb, 0, AlignLeft);
    connect(m_EMGroup, SIGNAL(clicked(int)), SLOT(slotEchoMode(int)));

    // Keep password

    m_KeepBut = new QCheckBox(i18n("&Remember passwords"), this);
    connect(m_KeepBut, SIGNAL(toggled(bool)), SLOT(slotKeep(bool)));
    top->addWidget(m_KeepBut);
    QHBoxLayout *hbox = new QHBoxLayout();
    top->addLayout(hbox);
    QLabel *lbl = new QLabel(i18n("&Timeout"), this);
    lbl->setFixedSize(lbl->sizeHint());
    hbox->addSpacing(20);
    hbox->addWidget(lbl);
    m_TimeoutEdit = new QSpinBox(this);
    lbl->setBuddy(m_TimeoutEdit);
    m_TimeoutEdit->setRange(5, 1200);
    m_TimeoutEdit->setSteps(5, 10);
    m_TimeoutEdit->setSuffix(i18n(" minutes"));
    m_TimeoutEdit->setFixedSize(m_TimeoutEdit->sizeHint());
    hbox->addWidget(m_TimeoutEdit);
    hbox->addStretch();

    top->addStretch();

    setButtons(buttons());
    m_pConfig = KGlobal::config();
    load();
}


KDEsuConfig::~KDEsuConfig()
{
}


void KDEsuConfig::load()
{
    KConfigGroupSaver saver(m_pConfig, "Passwords");

    QString val = m_pConfig->readEntry("EchoMode", "x");
    if (val == "OneStar")
	m_Echo = KPasswordEdit::OneStar;
    else if (val == "ThreeStars")
	m_Echo = KPasswordEdit::ThreeStars;
    else if (val == "NoEcho")
	m_Echo = KPasswordEdit::NoEcho;
    else
	m_Echo = defEchoMode;

    m_bKeep = m_pConfig->readBoolEntry("Keep", defKeep);
    m_Timeout = m_pConfig->readNumEntry("Timeout", defTimeout);

    apply();
    emit changed(false);
}


void KDEsuConfig::save()
{
    KConfigGroupSaver saver(m_pConfig, "Passwords");

    QString val;
    if (m_Echo == KPasswordEdit::OneStar)
	val = "OneStar";
    else if (m_Echo == KPasswordEdit::ThreeStars)
	val = "ThreeStars";
    else 
	val = "NoEcho";
    m_pConfig->writeEntry("EchoMode", val, true, true);

    m_pConfig->writeEntry("Keep", m_bKeep, true, true);
    m_Timeout = m_TimeoutEdit->value()*60;
    m_pConfig->writeEntry("Timeout", m_Timeout, true, true);

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
    m_Echo = defEchoMode;
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

