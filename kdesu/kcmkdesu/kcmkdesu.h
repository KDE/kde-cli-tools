/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __Kcmkdesu_h_included__
#define __Kcmkdesu_h_included__

class QButtonGroup;
class QCheckBox;
class QSpinBox;
class KConfig;

#include <kcmodule.h>

class KDEsuConfig : public KCModule
{
    Q_OBJECT

public:
    KDEsuConfig(QWidget *parent=0, const char *name=0);
    ~KDEsuConfig();

    virtual void load();
    virtual void save();
    virtual void defaults();

    void apply();
    int buttons();

signals:
    void changed(bool);

private slots:

    void slotEchoMode(int);
    void slotKeep(bool);

private:
    QButtonGroup *m_EMGroup;
    QCheckBox *m_KeepBut;
    QSpinBox *m_TimeoutEdit;
    KConfig *m_pConfig;

    int m_Echo, m_Timeout;
    bool m_bKeep;
};

#endif
