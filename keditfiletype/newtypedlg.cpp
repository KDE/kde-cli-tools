
#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcombobox.h>

#include <klocale.h>
#include <klineedit.h>

#include "newtypedlg.h"

NewTypeDialog::NewTypeDialog(QStringList groups,
			     QWidget *parent, const char *name)
  : KDialogBase(parent, name, true, i18n( "Create New File Type" ), 
    Ok|Cancel, Ok, true)
{
  QFrame *main = makeMainWidget();
  QVBoxLayout *topl = new QVBoxLayout(main, 0, spacingHint());

  QGridLayout *grid = new QGridLayout(2, 2);
  grid->setColStretch(1, 1);
  topl->addLayout(grid);

  QLabel *l = new QLabel(i18n("Group:"), main);
  grid->addWidget(l, 0, 0);

  groupCombo = new QComboBox(main);
  //groupCombo->setEditable( true ); M.O.: Currently, the code in filetypesview isn't capable of handling
  //new top level types; so better not let them be added than crash.
  groupCombo->insertStringList(groups);
  grid->addWidget(groupCombo, 0, 1);

  QWhatsThis::add( groupCombo, i18n("Select the category under which"
    " the new file type should be added.") );

  l = new QLabel(i18n("Type name:"), main);
  grid->addWidget(l, 1, 0);

  typeEd = new KLineEdit(main);
  grid->addWidget(typeEd, 1, 1);

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
