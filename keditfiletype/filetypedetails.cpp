#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qlabel.h>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kopenwith.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>

#include "filetypedetails.h"
#include "typeslistitem.h"

class ServiceListItem : public QListBoxText
{
public:
	ServiceListItem( QString &desktopPath );

	QString desktopPath;
};

ServiceListItem::ServiceListItem( QString &_desktopPath )
	: QListBoxText(), desktopPath(_desktopPath)
{
    KService::Ptr pService = KService::serviceByDesktopPath( _desktopPath );

	ASSERT(pService);

	setText( pService->name() );
}

FileTypeDetails::FileTypeDetails( QWidget * parent, const char * name )
  : QWidget( parent, name ), m_item( 0L )
{
  QString wtstr;
  QVBoxLayout *rightLayout = new QVBoxLayout(this);

  QHBoxLayout *hBox = new QHBoxLayout();
  rightLayout->addLayout(hBox, 2);

  iconButton = new KIconButton(this);
  iconButton->setIconType(KIcon::Desktop, KIcon::MimeType);
  connect(iconButton, SIGNAL(iconChanged(QString)), SLOT(updateIcon(QString)));

  iconButton->setFixedSize(50, 50);
  hBox->addWidget(iconButton);

  QWhatsThis::add( iconButton, i18n("This button displays the icon associated"
    " with the selected file type. Click on it to choose a different icon.") );

  QGroupBox *gb = new QGroupBox(i18n("Filename Patterns"), this);
  hBox->addWidget(gb);

  QGridLayout *grid = new QGridLayout(gb, 3, 2, KDialog::marginHint(),
                                      KDialog::spacingHint());
  grid->addRowSpacing(0, fontMetrics().lineSpacing());

  extensionLB = new QListBox(gb);
  connect(extensionLB, SIGNAL(highlighted(int)), SLOT(enableExtButtons(int)));
  grid->addMultiCellWidget(extensionLB, 1, 2, 0, 0);
  grid->setRowStretch(1, 1);

  QWhatsThis::add( extensionLB, i18n("This box contains a list of patterns that can be"
    " used to identify files of the selected type. For example, the pattern *.txt is"
    " associated with the file type 'text/plain'; all files ending in '.txt' are recognized"
    " as plain text files.") );

  addExtButton = new QPushButton(i18n("Add..."), gb);
  addExtButton->setEnabled(false);
  connect(addExtButton, SIGNAL(clicked()),
          this, SLOT(addExtension()));
  grid->addWidget(addExtButton, 1, 1);

  QWhatsThis::add( addExtButton, i18n("Add a new pattern for the selected file type.") );

  removeExtButton = new QPushButton(i18n("Remove"), gb);
  removeExtButton->setEnabled(false);
  connect(removeExtButton, SIGNAL(clicked()),
          this, SLOT(removeExtension()));
  grid->addWidget(removeExtButton, 2, 1);

  QWhatsThis::add( removeExtButton, i18n("Remove the selected filename pattern.") );

  gb = new QGroupBox(i18n("Description"), this);
  rightLayout->addWidget(gb);

  gb->setColumnLayout(1, Qt::Horizontal);
  description = new KLineEdit(gb);
  connect(description, SIGNAL(textChanged(const QString &)),
          SLOT(updateDescription(const QString &)));

  wtstr = i18n("You can enter a short description for files of the selected"
    " file type (e.g. 'HTML Page'). This description will be used by applications"
    " like Konqueror to display directory content.");
  QWhatsThis::add( gb, wtstr );
  QWhatsThis::add( description, wtstr );

  gb = new QGroupBox(i18n("Application Preference Order"), this);
  rightLayout->addWidget(gb, 1);

  grid = new QGridLayout(gb, 5, 2, KDialog::marginHint(),
                         KDialog::spacingHint());
  grid->addRowSpacing(0, fontMetrics().lineSpacing());
//  grid->setRowStretch(3, 1);

  servicesLB = new QListBox(gb);
  connect(servicesLB, SIGNAL(highlighted(int)), SLOT(enableMoveButtons(int)));
  grid->addMultiCellWidget(servicesLB, 1, 4, 0, 0);

  wtstr = i18n("This is a list of applications associated with files of the selected"
    " file type. This list is shown in Konqueror's context menus when you select"
    " \"Open with...\". If more than one application is associated with this file type,"
    " then the list is ordered by priority with the uppermost item taking precedence"
    " over the others.");
  QWhatsThis::add( gb, wtstr );
  QWhatsThis::add( servicesLB, wtstr );

  servUpButton = new QPushButton(i18n("Move &Up"), gb);
  servUpButton->setEnabled(false);
  connect(servUpButton, SIGNAL(clicked()), SLOT(promoteService()));
  grid->addWidget(servUpButton, 1, 1);

  QWhatsThis::add( servUpButton, i18n("Assigns a higher priority to the selected\n"
                              "application, moving it up in the list. Note:  This\n"
                              "only affects the selected application if the file type is\n"
			      "associated with more than one application."));

  servDownButton = new QPushButton(i18n("Move &Down"), gb);
  servDownButton->setEnabled(false);
  connect(servDownButton, SIGNAL(clicked()), SLOT(demoteService()));
  grid->addWidget(servDownButton, 2, 1);

  QWhatsThis::add( servDownButton, i18n("Assigns a lower priority to the selected\n"
			  "application, moving it down on the list.  Note:  This \n"
			  "only affects the selected application if the file type is\n"
			  "associated with more than one application."));

  servNewButton = new QPushButton(i18n("Add..."), gb);
  servNewButton->setEnabled(false);
  connect(servNewButton, SIGNAL(clicked()), SLOT(addService()));
  grid->addWidget(servNewButton, 3, 1);

  QWhatsThis::add( servNewButton, i18n( "Add a new application for this file type." ) );

  servRemoveButton = new QPushButton(i18n("Remove"), gb);
  servRemoveButton->setEnabled(false);
  connect(servRemoveButton, SIGNAL(clicked()), SLOT(removeService()));
  grid->addWidget(servRemoveButton, 4, 1);

  QWhatsThis::add( servRemoveButton, i18n( "Remove the selected application from the list." ) );
}

void FileTypeDetails::updateIcon(QString icon)
{
  if (!m_item)
    return;

  m_item->setIcon(icon);

  emit dirty();
}

void FileTypeDetails::updateDescription(const QString &desc)
{
  if (!m_item)
    return;

  m_item->setComment(desc);

  emit dirty();
}

void FileTypeDetails::addExtension()
{
  if ( !m_item )
    return;
  KLineEditDlg m(i18n("Extension:"), QString::null, this);
  m.setCaption( i18n("Add New Extension") );
  if (m.exec()) {
    if (!m.text().isEmpty()) {
      extensionLB->insertItem(m.text());
      QStringList patt = m_item->patterns();
      patt += m.text();
      m_item->setPatterns(patt);

      emit dirty();
    }
  }
}

void FileTypeDetails::removeExtension()
{
  if (extensionLB->currentItem() == -1)
    return;
  if ( !m_item )
    return;
  QStringList patt = m_item->patterns();
  patt.remove(extensionLB->text(extensionLB->currentItem()));
  m_item->setPatterns(patt);
  extensionLB->removeItem(extensionLB->currentItem());

  emit dirty();
}

void FileTypeDetails::setTypeItem( TypesListItem * tlitem )
{
  m_item = tlitem;
  iconButton->setIcon(tlitem->icon());
  description->setText(tlitem->comment());
  extensionLB->clear();
  addExtButton->setEnabled(true);
  removeExtButton->setEnabled(false);
  servNewButton->setEnabled(true);
  // will need a selection
  servUpButton->setEnabled(false);
  servDownButton->setEnabled(false);
  servRemoveButton->setEnabled(false);
  extensionLB->insertStringList(tlitem->patterns());

  servicesLB->clear();

  QStringList services = tlitem->defaultServices();

  if (services.count() == 0) {
    servicesLB->insertItem("None");
    servicesLB->setEnabled(false);
  } else {
    for ( QStringList::Iterator it = services.begin();
          it != services.end(); it++ )
    {
      servicesLB->insertItem( new ServiceListItem(*it) );
    }
    servicesLB->setEnabled(true);
  }
}

void FileTypeDetails::promoteService()
{
  if (!servicesLB->isEnabled()) {
    kapp->beep();
    return;
  }

  unsigned int selIndex = servicesLB->currentItem();
  if (selIndex == 0) {
    kapp->beep();
    return;
  }

  QListBoxItem *selItem = servicesLB->item(selIndex);
  servicesLB->takeItem(selItem);
  servicesLB->insertItem(selItem, selIndex-1);
  servicesLB->setCurrentItem(selIndex - 1);

  updatePreferredServices();

  emit dirty();
}

void FileTypeDetails::demoteService()
{
  if (!servicesLB->isEnabled()) {
    kapp->beep();
    return;
  }

  unsigned int selIndex = servicesLB->currentItem();
  if (selIndex == servicesLB->count() - 1) {
    kapp->beep();
    return;
  }

  QListBoxItem *selItem = servicesLB->item(selIndex);
  servicesLB->takeItem(selItem);
  servicesLB->insertItem(selItem, selIndex+1);
  servicesLB->setCurrentItem(selIndex + 1);

  updatePreferredServices();

  emit dirty();
}

void FileTypeDetails::addService()
{
  if (!m_item)
      return;

  KOpenWithDlg dlg(m_item->name(), QString::null, 0L);
  if (dlg.exec() == false)
    return;

  KService::Ptr service = dlg.service();

  ASSERT(service);

  // check if it is a duplicate entry
  for (unsigned int index = 0; index < servicesLB->count(); index++)
    if (servicesLB->text(index) == service->name())
      return;

  // if None is the only item, then there currently is no default
  if (servicesLB->text(0) == "None") {
      servicesLB->removeItem(0);
      servicesLB->setEnabled(true);
  }
  QString desktopPath = service->desktopEntryPath();

  servicesLB->insertItem( new ServiceListItem(desktopPath) );

  updatePreferredServices();

  emit dirty();
}

void FileTypeDetails::removeService()
{
  int selected = servicesLB->currentItem();

  if ( selected >= 0 ) {
    servicesLB->removeItem( selected );
    updatePreferredServices();

    emit dirty();
  }

  if ( servicesLB->currentItem() == -1 )
    servRemoveButton->setEnabled(false);

}

void FileTypeDetails::updatePreferredServices()
{
  if (!m_item)
    return;
  QStringList sl;
  unsigned int count = servicesLB->count();

  for (unsigned int i = 0; i < count; i++) {
    ServiceListItem *sli = (ServiceListItem *) servicesLB->item(i);
    sl.append( sli->desktopPath );
  }
  m_item->setDefaultServices(sl);
}

void FileTypeDetails::enableMoveButtons(int index)
{
  if (servicesLB->count() <= 1)
  {
    servUpButton->setEnabled(false);
    servDownButton->setEnabled(false);
  }
  else if ((uint) index == (servicesLB->count() - 1))
  {
    servUpButton->setEnabled(true);
    servDownButton->setEnabled(false);
  }
  else if (index == 0)
  {
    servUpButton->setEnabled(false);
    servDownButton->setEnabled(true);
  }
  else
  {
    servUpButton->setEnabled(true);
    servDownButton->setEnabled(true);
  }

  servRemoveButton->setEnabled(true);
}

void FileTypeDetails::enableExtButtons(int /*index*/)
{
  removeExtButton->setEnabled(true);
}

#include "filetypedetails.moc"
