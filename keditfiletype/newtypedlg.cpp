
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcombobox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kbuttonbox.h>
#include <klineedit.h>


#include "newtypedlg.h"

NewTypeDialog::NewTypeDialog(QStringList groups,
			     QWidget *parent, const char *name)
  : KDialog(parent, name, true)
{
  setCaption(i18n("Create new file type"));
  QVBoxLayout *topl = new QVBoxLayout(this, marginHint(), spacingHint());

  QGridLayout *grid = new QGridLayout(2, 2);
  grid->setColStretch(1, 1);
  topl->addLayout(grid);

  QLabel *l = new QLabel(i18n("Group:"), this);
  grid->addWidget(l, 0, 0);

  groupCombo = new QComboBox(this);
  groupCombo->insertStringList(groups);
  grid->addWidget(groupCombo, 0, 1);

  QWhatsThis::add( groupCombo, i18n("Select the category under which"
    " the new file type should be added.") );

  l = new QLabel(i18n("Type name:"), this);
  grid->addWidget(l, 1, 0);

  typeEd = new KLineEdit(this);
  grid->addWidget(typeEd, 1, 1);

  KButtonBox *bbox = new KButtonBox(this);
  topl->addWidget(bbox);

  bbox->addStretch(1);
  QPushButton *okButton = bbox->addButton(i18n("OK"));
  okButton->setDefault(true);
  connect(okButton, SIGNAL(clicked()),
	  this, SLOT(accept()));

  QPushButton *cancelButton = bbox->addButton(i18n("Cancel"));
  connect(cancelButton, SIGNAL(clicked()),
	  this, SLOT(reject()));

  typeEd->setFocus();

  // Set a minimum size so that caption is not half-hidden
  setMinimumSize( 300, 50 );
}

QString NewTypeDialog::group() const 
{ 
  return groupCombo->currentText(); 
}


QString NewTypeDialog::text() const 
{ 
  return typeEd->text(); 
}
