#include <qlayout.h>
#include <qlabel.h>
#include <kglobal.h>
#include <klocale.h>
#include <klined.h>
#include <kbuttonbox.h>

#include "extensiondlg.h"

ExtensionDialog::ExtensionDialog(QWidget *parent, const char *name)
  : KDialog(parent, name, true)
{
  setCaption(i18n("Add New Extension"));

  QVBoxLayout *topLayout = new QVBoxLayout(this, marginHint(), spacingHint());
  QHBoxLayout *hBox = new QHBoxLayout;
  topLayout->addLayout(hBox);

  QLabel *l = new QLabel(i18n("Extension:"), this);
  hBox->addWidget(l);
  extEd = new KLineEdit(this);
  hBox->addWidget(extEd, 1);

  KButtonBox *bbox = new KButtonBox(this);
  topLayout->addWidget(bbox);

  QPushButton *okButton = bbox->addButton(i18n("OK"));
  okButton->setDefault(true);
  connect(okButton, SIGNAL(clicked()),
	  this, SLOT(accept()));

  QPushButton *cancelButton = bbox->addButton(i18n("Cancel"));
  connect(cancelButton, SIGNAL(clicked()),
	  this, SLOT(reject()));
  extEd->setFocus();
}
