#ifndef _NEWTYPEDLG_H
#define _NEWTYPEDLG_H

#include <qstring.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <kdialog.h>
#include <klineedit.h>

class NewTypeDialog : public KDialog
{
public:
  NewTypeDialog(QStringList groups, QWidget *parent = 0, 
		const char *name = 0);
  QString group() const { return groupCombo->currentText(); }
  QString text() const { return typeEd->text(); }
private:
  KLineEdit *typeEd;
  QComboBox *groupCombo;
};

#endif
