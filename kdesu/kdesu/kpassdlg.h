/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __KPasswordDlg_h_included__
#define __KPasswordDlg_h_included__

#include <qstring.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qsize.h>
#include <qevent.h>

#include <kiconloader.h>

/**
 * KPasswordEdit: A safe password input widget.
 */

class KPasswordEdit: public QLineEdit
{
    Q_OBJECT;

public:
    KPasswordEdit(QWidget *parent=0, const char *name=0);
    ~KPasswordEdit();

    char *getPass() { return mPassword; }
    void erase();
    void setEchoMode(int echo) { mEcho = echo; }
    static const int PassLen = 100;

protected:
    virtual void keyPressEvent(QKeyEvent *);

private:
    void showit();

    char *mPassword;
    int mEcho;
};


/**
 * KPasswordDlg: A dialog for the input of a password.
 *
 * This dialog uses it's own result codes, KPasswordDlg::ResultCode because
 * it can have more results than QDialog. The result are: Accepted, Rejected
 * (same as QDialog) and AsUser. The last result means the user pressed the 
 * "As User" button.
 */

class KPasswordDlg: public QDialog
{
    Q_OBJECT;

public: 
    /**
     * This contructs a password dialog with helptext. If no text is given,
     * the helptext is set to: "Pleas enter root password".
     *
     * @parm help A descriptive text which is show to the user.
     * @parm command The command you are going to execute. This is for
     * feedback to the user only.
     * @parm keep Pass true if password keeping is enabled. This enables a
     * control in the widget to turn it off.
     * @parm helpFile The html help file you want to make available to the
     * user.
     * @parm helpTopic The topic in the help file.
     */
    KPasswordDlg(const QString& help, const QString& command=0, 
	    const QString& user="root", int keep=0, const QString& helpfile=0,
	    const QString &helptopic=0, QWidget *parent=0, 
	    const char *name=0);

    /** This is the destructor.  */
    ~KPasswordDlg();

    /**
     * Sets the echo mode. This can be one of: One_star, Three_star and
     * No_echo.
     */
    void setEchoMode(int mode) { edit->setEchoMode(mode); }

    /** Return the password */
    char *getPass() const { return edit->getPass(); }

    bool keepPass() const { return mKeep; }

    /** This dailog's result codes. */
    enum ResultCodes { Rejected, Accepted, AsUser };

protected slots:
    void slotHelp();
    void slotCheckPass();
    void slotCancel();
    void slotAsUser();
    void slotKeep(bool);

private:
    QString mHelp;
    QString mCommand;
    QString mUser;
    int mKeep;
    QString mHelpfile, mHelptopic;


    KPasswordEdit *edit;
};


#endif
