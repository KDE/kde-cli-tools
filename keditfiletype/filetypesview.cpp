
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtimer.h>
#include <qwidgetstack.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <klistview.h>
#include <klocale.h>
#include <kstddirs.h>

#include "newtypedlg.h"
#include "filetypedetails.h"
#include "filegroupdetails.h"
#include "filetypesview.h"

FileTypesView::FileTypesView(QWidget *p, const char *name)
  : KCModule(p, name)
{

  KServiceTypeProfile::setConfigurationMode();

  QString wtstr;

  QHBoxLayout *topLayout = new QHBoxLayout(this, KDialog::marginHint(),
                                KDialog::spacingHint());

  QGridLayout *leftLayout = new QGridLayout(4, 2);
  topLayout->addLayout(leftLayout, 0);

  QLabel *patternFilterLBL = new QLabel( i18n("F&ind filename pattern"), this );
  leftLayout->addMultiCellWidget(patternFilterLBL, 0, 0, 0, 1);

  patternFilterLE = new QLineEdit(this);
  patternFilterLBL->setBuddy( patternFilterLE );
  leftLayout->addMultiCellWidget(patternFilterLE, 1, 1, 0, 1);

  connect(patternFilterLE, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotFilter(const QString &)));

  wtstr = i18n("Enter a part of a filename pattern. Only file types with a "
               "matching file pattern will appear in the list.");

  QWhatsThis::add( patternFilterLE, wtstr );
  QWhatsThis::add( patternFilterLBL, wtstr );

  typesLV = new KListView(this);
  typesLV->setRootIsDecorated(true);

  typesLV->addColumn(i18n("Known Types"));
  leftLayout->addMultiCellWidget(typesLV, 2, 2, 0, 1);
  connect(typesLV, SIGNAL(selectionChanged(QListViewItem *)),
          this, SLOT(updateDisplay(QListViewItem *)));
  connect(typesLV, SIGNAL(doubleClicked(QListViewItem *)),
          this, SLOT(slotDoubleClicked(QListViewItem *)));

  QWhatsThis::add( typesLV, i18n("Here you can see a hierarchical list of"
    " the file types which are known on your system. Click on the '+' sign"
    " to expand a category, or the '-' sign to collapse it. Select a file type"
    " (e.g. text/html for HTML files) to view/edit the information for that"
    " file type using the controls on the right.") );

  QPushButton *addTypeB = new QPushButton(i18n("&Add..."), this);
  connect(addTypeB, SIGNAL(clicked()),
          this, SLOT(addType()));
  leftLayout->addWidget(addTypeB, 3, 0);

  QWhatsThis::add( addTypeB, i18n("Click here to add a new file type.") );

  m_removeTypeB = new QPushButton(i18n("&Remove"), this);
  connect(m_removeTypeB, SIGNAL(clicked()),
          this, SLOT(removeType()));
  leftLayout->addWidget(m_removeTypeB, 3, 1);
  m_removeTypeB->setEnabled(false);

  QWhatsThis::add( m_removeTypeB, i18n("Click here to remove the selected file type.") );

  // For the right panel, prepare a widget stack
  m_widgetStack = new QWidgetStack( this );

  // File Type Details
  m_details = new FileTypeDetails( m_widgetStack );
  connect( m_details, SIGNAL( changed(bool) ),
           this, SLOT( setDirty(bool) ) );
  m_widgetStack->addWidget( m_details, 1 /*id*/ );

  // File Group Details
  m_groupDetails = new FileGroupDetails( m_widgetStack );
  connect( m_groupDetails, SIGNAL( changed(bool) ),
           this, SLOT( setDirty(bool) ) );
  m_widgetStack->addWidget( m_groupDetails, 2 /*id*/ );

  // Widget shown on startup
  m_emptyWidget = new QLabel( i18n("Select a file type by name or by extension"), m_widgetStack);
  m_emptyWidget->setAlignment(AlignCenter);

  m_widgetStack->addWidget( m_emptyWidget, 3 /*id*/ );

  m_widgetStack->raiseWidget( m_emptyWidget );

  topLayout->addWidget( m_widgetStack, 100 );

  qApp->processEvents(); // let's show up
  QTimer::singleShot( 0, this, SLOT( init() ) ); // this takes some time
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
  setEnabled( false );
  setCursor( KCursor::waitCursor() );

  readFileTypes();
  // Since we have filled in the list once and for all, set width correspondingly,
  // to avoid horizontal scrollbars (DF).
  typesLV->setColumnWidth(0, typesLV->sizeHint().width() );
  typesLV->setMinimumWidth( typesLV->sizeHint().width() );

  setDirty(false);
  setEnabled( true );
  unsetCursor();

}

// only call this method once on startup, then never again! Otherwise, newly
// added Filetypes will be lost.
void FileTypesView::readFileTypes()
{
    typesLV->clear();
    m_majorMap.clear();
    m_itemList.clear();

    TypesListItem *groupItem;
    KMimeType::List mimetypes = KMimeType::allMimeTypes();
    QValueListIterator<KMimeType::Ptr> it2(mimetypes.begin());
    for (; it2 != mimetypes.end(); ++it2) {
	QString mimetype = (*it2)->name();
	int index = mimetype.find("/");
	QString maj = mimetype.left(index);
	QString min = mimetype.right(mimetype.length() - index+1);

	QMapIterator<QString,TypesListItem*> mit = m_majorMap.find( maj );
	if ( mit == m_majorMap.end() ) {
	    groupItem = new TypesListItem( typesLV, maj );
	    m_majorMap.insert( maj, groupItem );
	}
	else
	    groupItem = mit.data();

	TypesListItem *item = new TypesListItem(groupItem, (*it2));
	m_itemList.append( item );
    }
}

void FileTypesView::slotFilter(const QString & patternFilter)
{
    // one of the few ways to clear a listview without destroying the
    // listviewitems and without making QListView crash.
    QListViewItem *item;
    while ( (item = typesLV->firstChild()) ) {
	while ( item->firstChild() )
	    item->takeItem( item->firstChild() );

	typesLV->takeItem( item );
    }

    // insert all items and their group that match the filter
    QListIterator<TypesListItem> it( m_itemList );
    while ( it.current() ) {
	if ( patternFilter.isEmpty() ||
	     !((*it)->patterns().grep( patternFilter, false )).isEmpty() ) {

	    TypesListItem *group = m_majorMap[ (*it)->majorType() ];
	    // QListView makes sure we don't insert a group-item more than once
	    typesLV->insertItem( group );
	    group->insertItem( *it );
	}
	++it;
    }
}

void FileTypesView::addType()
{
  QStringList allGroups;
  QMapIterator<QString,TypesListItem*> it = m_majorMap.begin();
  while ( it != m_majorMap.end() ) {
      allGroups.append( it.key() );
      ++it;
  }

  NewTypeDialog m(allGroups, this);

  if (m.exec()) {
    QListViewItemIterator it(typesLV);
    QString loc = m.group() + "/" + m.text() + ".desktop";
    loc = locateLocal("mime", loc);
    KMimeType::Ptr mimetype = new KMimeType(loc,
                                            m.group() + "/" + m.text(),
                                            QString(), QString(),
                                            QStringList());

    TypesListItem *group = m_majorMap[ m.group() ];

    // find out if our group has been filtered out -> insert if necessary
    QListViewItem *item = typesLV->firstChild();
    bool insert = true;
    while ( item ) {
	if ( item == group ) {
	    insert = false;
	    break;
	}
	item = item->nextSibling();
    }
    if ( insert )
	typesLV->insertItem( group );

    TypesListItem *tli = new TypesListItem(group, mimetype, true);
    m_itemList.append( tli );

    group->setOpen(true);
    typesLV->setSelected(tli, true);

    setDirty(true);
  }
}

void FileTypesView::removeType()
{
  TypesListItem *current = (TypesListItem *) typesLV->currentItem();
  QListViewItem *li = 0L;

  if ( !current )
      return;

  if ( !current->isMeta() )
  {
    li = current->itemAbove();
    if (!li)
      li = current->itemBelow();
    if (!li)
      li = current->parent();
    removedList.append(current->name());
    current->parent()->takeItem(current);
    m_itemList.removeRef( current );
    setDirty(true);
  }

  if ( li )
      typesLV->setSelected(li, true);
}

void FileTypesView::slotDoubleClicked(QListViewItem *item)
{
  item->setOpen( !item->isOpen() );
}

void FileTypesView::updateDisplay(QListViewItem *item)
{
  if (!item)
  {
    m_widgetStack->raiseWidget( m_emptyWidget );
    m_removeTypeB->setEnabled(false);
    return;
  }

  bool wasDirty = m_dirty;

  TypesListItem *tlitem = (TypesListItem *) item;
  if (tlitem->isMeta()) // is a group
  {
    m_widgetStack->raiseWidget( m_groupDetails );
    m_groupDetails->setTypeItem( tlitem );
    m_removeTypeB->setEnabled(false);
  }
  else
  {
    m_widgetStack->raiseWidget( m_details );
    m_details->setTypeItem( tlitem );
    m_removeTypeB->setEnabled(true);
  }

  // Updating the display indirectly called change(true)
  if ( !wasDirty )
    setDirty(false);
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
      kdDebug() << "Entry " << tli->name() << " is dirty. Saving." << endl;
      tli->sync();
      didIt = true;
    }
  }
  setDirty(false);
  return didIt;
}

void FileTypesView::load()
{
}

void FileTypesView::save()
{
  // only send dcop signal if sync() was necessary.
  if (sync()) {
    DCOPClient *dcc = kapp->dcopClient();
    if ( !dcc->isAttached() )
	dcc->attach();
    dcc->send("kded", "kbuildsycoca", "recreate()", QByteArray());
  }
}

void FileTypesView::defaults()
{
}

QString FileTypesView::quickHelp() const
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

