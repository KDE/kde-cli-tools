#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qwhatsthis.h>
#include <qlabel.h>

#include <dcopclient.h>
#include <krun.h>
#include <kbuttonbox.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kmimetype.h>
#include <kuserprofile.h>
#include <kdesktopfile.h>
#include <kopenwith.h>
#include <kdebug.h>

#include "typeslistitem.h"
#include "newtypedlg.h"
#include "extensiondlg.h"

#include "filetypesview.h"

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

FileTypesView::FileTypesView(QWidget *p, const char *name)
  : KCModule(p, name)
{
  QString wtstr;
  setButtons(Cancel|Apply|Ok);

  QHBoxLayout *topLayout = new QHBoxLayout(this, KDialog::marginHint(),
					   KDialog::spacingHint());

  QGridLayout *leftLayout = new QGridLayout(3, 2);
  topLayout->addLayout(leftLayout, 2);

  QLabel *patternFilterLBL = new QLabel( i18n("Find filename pattern"), this );
  leftLayout->addWidget(patternFilterLBL, 0, 0);
  
  patternFilterLE = new QLineEdit(this);
  leftLayout->addWidget(patternFilterLE, 0, 1);
  
  connect(patternFilterLE, SIGNAL(textChanged(const QString &)),
	  this, SLOT(slotFilter(const QString &)));
  
  wtstr = i18n("Enter a part of a filename pattern. Only file types with a "
	       "matching file pattern will appear in the list.");
  
  QWhatsThis::add( patternFilterLE, wtstr );
  QWhatsThis::add( patternFilterLBL, wtstr );

  typesLV = new QListView(this);
  typesLV->setRootIsDecorated(true);

  typesLV->addColumn(i18n("Known Types"));
  leftLayout->addMultiCellWidget(typesLV, 1, 1, 0, 1);
  connect(typesLV, SIGNAL(selectionChanged(QListViewItem *)),
          this, SLOT(updateDisplay(QListViewItem *)));

  QWhatsThis::add( typesLV, i18n("Here you can see a hierarchical list of"
    " the file types which are known on your system. Click on the '+' sign"
    " to expand a category, or the '-' sign to collapse it. Select a file type"
    " (e.g. text/html for HTML files) to view/edit the information for that"
    " file type using the controls on the right.") );

  QPushButton *addTypeB = new QPushButton(i18n("&Add..."), this);
  connect(addTypeB, SIGNAL(clicked()),
          this, SLOT(addType()));
  leftLayout->addWidget(addTypeB, 2, 0);

  QWhatsThis::add( addTypeB, i18n("Click here to add a new file type.") );

  QPushButton *removeTypeB = new QPushButton(i18n("&Remove"), this);
  connect(removeTypeB, SIGNAL(clicked()),
          this, SLOT(removeType()));
  leftLayout->addWidget(removeTypeB, 2, 1);

  QWhatsThis::add( removeTypeB, i18n("Click here to remove the selected file type.") );

  QVBoxLayout *rightLayout = new QVBoxLayout();
  topLayout->addLayout(rightLayout, 3);

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
  
  init();
  setDirty(false);
}

FileTypesView::~FileTypesView()
{
}

void FileTypesView::setDirty(bool state)
{
	emit changed(state);
	m_dirty = state;
}

void FileTypesView::init()
{
  readFileTypes();
}

void FileTypesView::slotFilter(const QString &patternFilter)
{
  readFileTypes(patternFilter);
}

void FileTypesView::readFileTypes(const QString &patternFilter)
{
    typesLV->clear();
    
    KMimeType::List mimetypes = KMimeType::allMimeTypes();
    QValueListIterator<KMimeType::Ptr> it2(mimetypes.begin());
    for (; it2 != mimetypes.end(); ++it2) {
	bool add = true;
	
	if ( !patternFilter.isEmpty() ) {
	    QStringList matches = (*it2)->patterns().grep( patternFilter, 
							   false );
	    add = !matches.isEmpty();
	}
	
	if ( add ) {
	    QString mimetype = (*it2)->name();
	    int index = mimetype.find("/");
	    QString maj = mimetype.left(index);
	    QString min = mimetype.right(mimetype.length() - index+1);
	    
	    QListViewItemIterator it(typesLV);
	    for (; it.current(); ++it) {
		TypesListItem *current = (TypesListItem *) it.current();
		if (current->majorType() == maj) {
		    new TypesListItem(current, (*it2));
		    break;
		}
	    }
	    if (!it.current()) {
		// insert at top level.
		TypesListItem *i = new TypesListItem(typesLV, (*it2));
		
		if ( !patternFilter.isEmpty() )
		    i->setOpen(true);
		
		new TypesListItem(i, (*it2));
	    }
	}
    }
}

void FileTypesView::addType()
{
  QStringList groups;
  QListViewItemIterator it(typesLV);
  for (; it.current(); ++it) {
    TypesListItem *current = (TypesListItem *) it.current();
    if (!groups.contains(current->majorType()))
      groups.append(current->majorType());
  }

  NewTypeDialog m(groups, this);

  if (m.exec()) {
    QListViewItemIterator it(typesLV);
    QString loc = m.group() + "/" + m.text() + ".desktop";
    loc = locateLocal("mime", loc);
    KMimeType::Ptr mimetype = new KMimeType(loc,
                                            m.group() + "/" + m.text(),
                                            QString(), QString(),
                                            QStringList());

    for (; it.current(); ++it) {
      TypesListItem *current = (TypesListItem *) it.current();
      if (current->majorType() == m.group()) {
        TypesListItem *tli = new TypesListItem(current, mimetype);
      if (!tli->parent()->isOpen())
        tli->parent()->setOpen(true);
        typesLV->setSelected(tli, true);
        break;
      }
    }
    if (!it.current()) {
      // insert at top level.
      TypesListItem *i = new TypesListItem(typesLV, mimetype);
      TypesListItem *tli = new TypesListItem(i, mimetype);
      if (!tli->parent()->isOpen())
        tli->parent()->setOpen(true);
      typesLV->setSelected(tli, true);
    }

    setDirty(true);
  }
}

void FileTypesView::removeType()
{
  TypesListItem *current = (TypesListItem *) typesLV->currentItem();
  QListViewItem *li = 0L;

  if (current->isMeta())
    kapp->beep();
  else {
    li = current->itemAbove();
    if (!li)
      li = current->itemBelow();
    if (!li)
      li = current->parent();
    removedList.append(current->name());
    current->parent()->takeItem(current);
  }
  typesLV->setSelected(li, true);

  setDirty(true);
}

void FileTypesView::addExtension()
{
  ExtensionDialog m(this);
  if (m.exec()) {
    if (!m.text().isEmpty()) {
      extensionLB->insertItem(m.text());
      TypesListItem *tli = (TypesListItem *) typesLV->selectedItem();
      QStringList patt = tli->patterns();
      patt += m.text();
      tli->setPatterns(patt);

      setDirty(true);
    }
  }
}

void FileTypesView::removeExtension()
{
  if (extensionLB->currentItem() == -1)
    return;
  TypesListItem *tli = (TypesListItem *) typesLV->selectedItem();
  QStringList patt = tli->patterns();
  patt.remove(extensionLB->text(extensionLB->currentItem()));
  tli->setPatterns(patt);
  extensionLB->removeItem(extensionLB->currentItem());

  setDirty(true);
}

void FileTypesView::updateDisplay(QListViewItem *item)
{
  bool wasDirty = m_dirty;

  if (!item)
    return;

  TypesListItem *tlitem = (TypesListItem *) item;
  if (tlitem->isMeta())
    return;

  iconButton->setIcon(tlitem->icon());
  description->setText(tlitem->comment());
  extensionLB->clear();
  removeExtButton->setEnabled(false);
  addExtButton->setEnabled(true);
  servNewButton->setEnabled(true);
  servRemoveButton->setEnabled(false);
  extensionLB->insertStringList(tlitem->patterns());


  servicesLB->clear();
  
  QStringList services = tlitem->defaultServices();
  
  if (services.count() == 0) {
    servicesLB->insertItem("None");
    servicesLB->setEnabled(false);
    servUpButton->setEnabled(false);
    servDownButton->setEnabled(false);
  } else {
    for ( QStringList::Iterator it = services.begin();
          it != services.end(); it++ ) 
    {
      servicesLB->insertItem( new ServiceListItem(*it) );
    }
    servicesLB->setEnabled(true);
  }

  // Updating the display indirectly called change(true)
  if ( !wasDirty )
    setDirty(false);
}

void FileTypesView::updateIcon(QString icon)
{
  if (extensionLB->currentItem() == 0)
    return;

  TypesListItem *tli = (TypesListItem *) typesLV->currentItem();
  tli->setIcon(icon);

  setDirty(true);
}

void FileTypesView::updateDescription(const QString &desc)
{
  if (extensionLB->currentItem() == 0)
    return;

  TypesListItem *tli = (TypesListItem *) typesLV->currentItem();
  tli->setComment(desc);

  setDirty(true);
}

void FileTypesView::promoteService()
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

  setDirty(true);
}

void FileTypesView::demoteService()
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
  
  setDirty(true);
}

void FileTypesView::updatePreferredServices()
{
  if (typesLV->currentItem() == 0)
    return;
  TypesListItem *tli = (TypesListItem *) typesLV->currentItem();
  QStringList sl;
  unsigned int count = servicesLB->count();

  for (unsigned int i = 0; i < count; i++) {
    ServiceListItem *sli = (ServiceListItem *) servicesLB->item(i);
    sl.append( sli->desktopPath );
  }
  tli->setDefaultServices(sl);
}

bool FileTypesView::sync()
{
  bool didIt = false;
  // first, remove those items which we are asked to remove.
  QStringList::Iterator it(removedList.begin());
  QString loc;

  for (; it != removedList.end(); ++it) {
    didIt = true;
    KMimeType::Ptr m_ptr = KMimeType::mimeType(*it);

    loc = *it + ".desktop";
    loc = locateLocal("mime", loc);

    KDesktopFile config(loc, false, "mime");
    config.writeEntry("Type", "MimeType");
    config.writeEntry("MimeType", m_ptr->name());
    config.writeEntry("Hidden", true);
  }

  // now go through all entries and sync those which are dirty.
  QListViewItemIterator it2(typesLV);
  for (; it2.current(); ++it2) {
    TypesListItem *tli = (TypesListItem *) it2.current();
    if (tli->isDirty()) {
      tli->sync();
      didIt = true;
    }
  }
  setDirty(false);
  return didIt;
}

void FileTypesView::enableMoveButtons(int index)
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

void FileTypesView::enableExtButtons(int /*index*/)
{
  removeExtButton->setEnabled(true);
}

void FileTypesView::addService()
{
  TypesListItem *item = (TypesListItem *)typesLV->currentItem();
  if (!item)
      return;

  QStringList list;
  list.append(i18n("File Type: %1").arg(item->name()));
  KOpenWithDlg dlg(list, i18n("Add Application"), QString::null, (QWidget*)0);
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

  setDirty(true);
}

void FileTypesView::removeService()
{
	int selected = servicesLB->currentItem(); 

	if ( selected >= 0 ) {
		servicesLB->removeItem( selected );
		updatePreferredServices();

		setDirty(true);
	}

	if ( servicesLB->currentItem() == -1 ) 
		servRemoveButton->setEnabled(false);

}

void FileTypesView::load()
{
}

void FileTypesView::save()
{
  // only send dcop signal if sync() was necessary.
  if (sync()) {
    DCOPClient *dcc = kapp->dcopClient();
    dcc->send("kded", "kbuildsycoca", "recreate()", QByteArray());
  }
}

void FileTypesView::defaults()
{
}

QString FileTypesView::quickHelp()
{
  return i18n("<h1>File Associations</h1>"
    " This module allows you to choose which applications are associated"
    " with a given type of file. File types are also referred to MIME types"
    " (MIME is an acronym which stands for \"Multipurpose Internet Mail"
    " Extensions\".)<p> A file association consists of the following:"
    " <ul><li>Rules for determining the MIME-type of a file. For example,"
    " the filename pattern *.kwd, which means 'all files with names that end"
    " in .kwd', is associated with the MIME type \"x-kword\".</li>"
    " <li>A short description of the MIME-type. For example, the description"
    " of the MIME type \"x-kword\" is simply 'KWord document'.)</li>"
    " <li>An icon to be used for displaying files of the given MIME-type,"
    " so that you can easily identify the type of file, say in a Konqueror"
    " view (at least for the types you use often!)</li>"
    " <li>A list of the applications which can be used to open files of the"
    " given MIME-type. If more than one application can be used, then the"
    " list is ordered by priority.</li></ul>"
    " You may be surprised to find that some MIME types have no associated"
    " filename patterns! In these cases, Konqueror is able to determine the"
    " MIME-type by directly examining the contents of the file.");
}

#include "filetypesview.moc"

extern "C"
{
  KCModule *create_filetypes(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("filetypes");
    return new FileTypesView(parent, name);
  }

  void init_filetypes()
  {
  }
}

