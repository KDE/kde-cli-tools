/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __KPassDlg_h_included__
#define __KPassDlg_h_included__

#include <qstring.h>
#include <qlineedit.h>
#include <qevent.h>

#include <kdialogbase.h>

class QLabel;
class QWidget;

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
 * KPassDialog: A password dialog.
 */

class KPassDialog
    : public KDialogBase
{
    Q_OBJECT

public: 
    KPassDialog(QString command, int keep, int extrabtn=0);
    virtual ~KPassDialog();

    void setEchoMode(int mode) { m_pEdit->setEchoMode(mode); }
    const char *getPass() const { return m_pEdit->getPass(); }
    bool keepPass() const { return m_Keep; }

protected:
    virtual bool checkPass(const char *) { return true; }
    void setHelpText(QString text);

private slots:
    void slotOk();
    void slotCancel();
    void slotKeep(bool);

private:
    int m_Keep;
    QString m_Command;
    QLabel *m_pHelpLbl, *m_pCommandLbl;
    KPasswordEdit *m_pEdit;
};


#endif // __KPassDlg_h_included__
