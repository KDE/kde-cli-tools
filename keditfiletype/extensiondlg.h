#ifndef _EXTENSIONDLG_H
#define _EXTENSIONDLG_H

#include <qstring.h>

#include <kdialog.h>

class KLineEdit;

class ExtensionDialog : public KDialog
{
public:
  ExtensionDialog(QWidget *parent = 0, const char *name = 0);
  QString text() const { return extEd->text(); }

private:
  KLineEdit *extEd;
};

#endif
