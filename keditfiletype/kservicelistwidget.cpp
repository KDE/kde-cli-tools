#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kopenwith.h>
#include <kmimetype.h>
#include <knotifyclient.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kuserprofile.h>

#include "kservicelistwidget.h"
#include "kserviceselectdlg.h"
#include "typeslistitem.h"

class KServiceListItem : public QListBoxText
{
public:
	KServiceListItem( QString &desktopPath );

	QString desktopPath;
};

KServiceListItem::KServiceListItem( QString &_desktopPath )
    : QListBoxText(), desktopPath(_desktopPath)
{
    KService::Ptr pService = KService::serviceByDesktopPath( _desktopPath );

    ASSERT(pService);

    setText( pService->name() );
}

KServiceListWidget::KServiceListWidget(int kind, QWidget *parent, const char *name)
  : QGroupBox( kind == SERVICELIST_APPLICATIONS ? i18n("Application Preference Order")
               : i18n("Services Preference Order"), parent, name ),
    m_kind( kind ), m_item( 0L )
{
  QWidget * gb = this;
  QGridLayout * grid = new QGridLayout(gb, 6, 2, KDialog::marginHint(),
                                       KDialog::spacingHint());
  grid->addRowSpacing(0, fontMetrics().lineSpacing());
  grid->setRowStretch(1, 1);
  grid->setRowStretch(2, 1);
  grid->setRowStretch(3, 1);
  grid->setRowStretch(4, 1);
  grid->setRowStretch(5, 1);

  servicesLB = new QListBox(gb);
  connect(servicesLB, SIGNAL(highlighted(int)), SLOT(enableMoveButtons(int)));
  grid->addMultiCellWidget(servicesLB, 1, 5, 0, 0);

  QString wtstr =
    (kind == SERVICELIST_APPLICATIONS ?
     i18n("This is a list of applications associated with files of the selected"
          " file type. This list is shown in Konqueror's context menus when you select"
          " \"Open with...\". If more than one application is associated with this file type,"
          " then the list is ordered by priority with the uppermost item taking precedence"
          " over the others.") :
     i18n("This is a list of services associated with files of the selected"
          " file type. This list is shown in Konqueror's context menus when you select"
          " a \"Preview with...\" option. If more than one application is associated with this file type,"
          " then the list is ordered by priority with the uppermost item taking precedence"
          " over the others."));

  QWhatsThis::add( gb, wtstr );
  QWhatsThis::add( servicesLB, wtstr );

  servUpButton = new QPushButton(i18n("Move &Up"), gb);
  servUpButton->setEnabled(false);
  connect(servUpButton, SIGNAL(clicked()), SLOT(promoteService()));
  grid->addWidget(servUpButton, 1, 1);

  QWhatsThis::add( servUpButton, kind == SERVICELIST_APPLICATIONS ?
                   i18n("Assigns a higher priority to the selected\n"
                        "application, moving it up in the list. Note:  This\n"
                        "only affects the selected application if the file type is\n"
                        "associated with more than one application.") :
                   i18n("Assigns a higher priority to the selected\n"
                        "service, moving it up in the list."));

  servDownButton = new QPushButton(i18n("Move &Down"), gb);
  servDownButton->setEnabled(false);
  connect(servDownButton, SIGNAL(clicked()), SLOT(demoteService()));
  grid->addWidget(servDownButton, 2, 1);

  QWhatsThis::add( servDownButton, kind == SERVICELIST_APPLICATIONS ?
                   i18n("Assigns a lower priority to the selected\n"
                        "application, moving it down in the list.  Note:  This \n"
                        "only affects the selected application if the file type is\n"
                        "associated with more than one application."):
                   i18n("Assigns a lower priority to the selected\n"
                        "service, moving it down in the list."));

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

void KServiceListWidget::setTypeItem( TypesListItem * item )
{
  m_item = item;
  if ( servNewButton )
    servNewButton->setEnabled(true);
  // will need a selection
  servUpButton->setEnabled(false);
  servDownButton->setEnabled(false);
  if ( servRemoveButton )
    servRemoveButton->setEnabled(false);
  servicesLB->clear();
  servicesLB->setEnabled(false);

  if ( item )
  {
    QStringList services = ( m_kind == SERVICELIST_APPLICATIONS )
      ? item->appServices()
      : item->embedServices();

    if (services.count() == 0) {
      servicesLB->insertItem(i18n("None"));
    } else {
      for ( QStringList::Iterator it = services.begin();
            it != services.end(); it++ )
      {
        servicesLB->insertItem( new KServiceListItem(*it) );
      }
      servicesLB->setEnabled(true);
    }
  }
}

void KServiceListWidget::promoteService()
{
  if (!servicesLB->isEnabled()) {
    KNotifyClient::beep();
    return;
  }

  unsigned int selIndex = servicesLB->currentItem();
  if (selIndex == 0) {
    KNotifyClient::beep();
    return;
  }

  QListBoxItem *selItem = servicesLB->item(selIndex);
  servicesLB->takeItem(selItem);
  servicesLB->insertItem(selItem, selIndex-1);
  servicesLB->setCurrentItem(selIndex - 1);

  updatePreferredServices();

  emit changed(true);
}

void KServiceListWidget::demoteService()
{
  if (!servicesLB->isEnabled()) {
    KNotifyClient::beep();
    return;
  }

  unsigned int selIndex = servicesLB->currentItem();
  if (selIndex == servicesLB->count() - 1) {
    KNotifyClient::beep();
    return;
  }

  QListBoxItem *selItem = servicesLB->item(selIndex);
  servicesLB->takeItem(selItem);
  servicesLB->insertItem(selItem, selIndex+1);
  servicesLB->setCurrentItem(selIndex + 1);

  updatePreferredServices();

  emit changed(true);
}

void KServiceListWidget::addService()
{
  if (!m_item)
      return;

  KService::Ptr service = 0L;
  if ( m_kind == SERVICELIST_APPLICATIONS )
  {
      KOpenWithDlg dlg(m_item->name(), QString::null, 0L);
      if (dlg.exec() == false)
          return;

      service = dlg.service();

      ASSERT(service);
      if (!service)
          return; // Don't crash if KOpenWith wasn't able to create service.
  }
  else
  {
      KServiceSelectDlg dlg(m_item->name(), QString::null, 0L);
      if (dlg.exec() == false)
          return;
       service = dlg.service();
       ASSERT(service);
       if (!service)
           return;
  }

  // check if it is a duplicate entry
  for (unsigned int index = 0; index < servicesLB->count(); index++)
    if (servicesLB->text(index) == service->name())
      return;

  // if None is the only item, then there currently is no default
  if (servicesLB->text(0) == i18n("None")) {
      servicesLB->removeItem(0);
      servicesLB->setEnabled(true);
  }
  QString desktopPath = service->desktopEntryPath();

  servicesLB->insertItem( new KServiceListItem(desktopPath) );

  updatePreferredServices();

  emit changed(true);
}

void KServiceListWidget::removeService()
{
  int selected = servicesLB->currentItem();

  if ( selected >= 0 ) {
    servicesLB->removeItem( selected );
    updatePreferredServices();

    emit changed(true);
  }

  if ( servRemoveButton && servicesLB->currentItem() == -1 )
    servRemoveButton->setEnabled(false);

}

void KServiceListWidget::updatePreferredServices()
{
  if (!m_item)
    return;
  QStringList sl;
  unsigned int count = servicesLB->count();

  for (unsigned int i = 0; i < count; i++) {
    KServiceListItem *sli = (KServiceListItem *) servicesLB->item(i);
    sl.append( sli->desktopPath );
  }
  if ( m_kind == SERVICELIST_APPLICATIONS )
    m_item->setAppServices(sl);
  else
    m_item->setEmbedServices(sl);
}

void KServiceListWidget::enableMoveButtons(int index)
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

  if ( servRemoveButton )
    servRemoveButton->setEnabled(true);
}

#include "kservicelistwidget.moc"
