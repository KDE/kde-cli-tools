
#include <klistview.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include <kstddirs.h>

#include "typeslistitem.h"
#include "newtypedlg.h"

#include "filetypedetails.h"
#include "filetypesview.h"

FileTypesView::FileTypesView(QWidget *p, const char *name)
  : KCModule(p, name)
{
  QString wtstr;
  setButtons(Cancel|Apply|Ok);

  QHBoxLayout *topLayout = new QHBoxLayout(this, KDialog::marginHint(),
					   KDialog::spacingHint());

  QGridLayout *leftLayout = new QGridLayout(4, 2);
  topLayout->addLayout(leftLayout, 0);

  QLabel *patternFilterLBL = new QLabel( i18n("Find filename pattern"), this );
  leftLayout->addMultiCellWidget(patternFilterLBL, 0, 0, 0, 1);

  patternFilterLE = new QLineEdit(this);
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

  QPushButton *removeTypeB = new QPushButton(i18n("&Remove"), this);
  connect(removeTypeB, SIGNAL(clicked()),
          this, SLOT(removeType()));
  leftLayout->addWidget(removeTypeB, 3, 1);

  QWhatsThis::add( removeTypeB, i18n("Click here to remove the selected file type.") );

  m_details = new FileTypeDetails( this );
  connect( m_details, SIGNAL( dirty() ),
           this, SLOT( setDirty() ) );
  topLayout->addWidget( m_details, 100 );

  init();

  // Since we have filled in the list once and for all, set width correspondingly,
  // to avoid horizontal scrollbars (DF).
  typesLV->setColumnWidth(0, typesLV->sizeHint().width() );
  typesLV->setMinimumWidth( typesLV->sizeHint().width() );

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
        TypesListItem *tli = new TypesListItem(current, mimetype, true);
        if (!tli->parent()->isOpen())
          tli->parent()->setOpen(true);
        typesLV->setSelected(tli, true);
        break;
      }
    }
    if (!it.current()) {
      // insert at top level.
      TypesListItem *i = new TypesListItem(typesLV, mimetype);
      TypesListItem *tli = new TypesListItem(i, mimetype, true);
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


void FileTypesView::updateDisplay(QListViewItem *item)
{
  bool wasDirty = m_dirty;

  if (!item)
    return;

  TypesListItem *tlitem = (TypesListItem *) item;
  if (tlitem->isMeta()) // is a group
  {
    // Remove details - this is even more needed when we just
    // removed a mimetype (and the group gets activated)
    m_details->setTypeItem( 0L );
    return;
  }

  m_details->setTypeItem( tlitem );

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

