/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __KPasswordDlg_h_included__
#define __KPasswordDlg_h_included__

#include <qstring.h>
#include <qlineedit.h>
#include <qevent.h>

#include <kaboutdialog.h>

/**
 * KPasswordEdit: A safe password input widget.
 */

class KPasswordEdit
    : public QLineEdit
{
    Q_OBJECT

public:
    KPasswordEdit(QWidget *parent=0, const char *name=0);
    ~KPasswordEdit();

    const char *getPass() { return m_Password; }
    void erase();
    void setEchoMode(int echomode) { m_EchoMode = echomode; }

    static const int PassLen = 100;

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    void showPass();

    char *m_Password;
    int m_EchoMode, m_length;
};


/**
 * KDEsuDialog: The main dialog for kdesu.
 */

class KDEsuDialog
    : public KDialogBase
{
    Q_OBJECT

public: 
    KDEsuDialog(QString command, QString user, int keep);
    ~KDEsuDialog();

    void setEchoMode(int mode) { m_pEdit->setEchoMode(mode); }

    const char *getPass() const { return m_pEdit->getPass(); }
    bool keepPass() const { return m_Keep; }

    enum ResultCodes { Rejected, Accepted, AsUser };

protected slots:
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotKeep(bool);

private:
    int m_Keep;
    QString m_Command;
    QString m_User;
    KPasswordEdit *m_pEdit;
};


#endif
