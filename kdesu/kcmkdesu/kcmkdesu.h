/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __Kcmkdesu_h_included__
#define __Kcmkdesu_h_included__

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qsize.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qslider.h>

#include <kcontrol.h>
#include <kapp.h>
#include <kconfig.h>

class KDEsuConfig : public KConfigWidget
{
    Q_OBJECT

public:
    KDEsuConfig(QWidget *parent=0, const char *name=0);
    ~KDEsuConfig();

public slots:
    void loadSettings();
    void applySettings();
    void defaultSettings();
    void updateSettings();

private slots:

    void slotEchoMode(int);
    void slotKeep(bool);

private:
    QButtonGroup *em_group;
    QCheckBox *keep_but;
    QLineEdit *timeout_edit;
    KConfig *config;

    int echo_mode;
    int pw_timeout;
    bool keep_pw;
};

class KDEsuApplication : public KControlApplication
{
public:
    KDEsuApplication(int &argc, char *argv[], const char *name);

    void init();
    void apply();
    void defaultValues();
       
private:
    KDEsuConfig *kdesu;
};

#endif
