#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qlistbox.h>

#include <dcopclient.h>
#include <kbuttonbox.h>
#include <kiconloaderdialog.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kservice.h>
#include <kservicetype.h>
#include <kmimetype.h>
#include <kuserprofile.h>
#include <kdesktopfile.h>
#include <kopenwith.h>

#include "typeslistitem.h"
#include "newtypedlg.h"
#include "extensiondlg.h"

#include "filetypesview.h"

FileTypesView::FileTypesView(QWidget *p, const char *name)
  : KCModule(p, name)
{
  setButtons(Cancel|Apply|Ok);
  QHBoxLayout *topLayout = new QHBoxLayout(this, KDialog::marginHint(),
                                           KDialog::spacingHint());

  QGridLayout *leftLayout = new QGridLayout(2, 2);
  topLayout->addLayout(leftLayout, 2);

  typesLV = new QListView(this);
  typesLV->setRootIsDecorated(true);

  typesLV->addColumn(i18n("Known Types"));
  leftLayout->addMultiCellWidget(typesLV, 0, 0, 0, 1);
  connect(typesLV, SIGNAL(selectionChanged(QListViewItem *)),
          this, SLOT(updateDisplay(QListViewItem *)));

  QPushButton *addTypeB = new QPushButton(i18n("&Add..."), this);
  connect(addTypeB, SIGNAL(clicked()),
          this, SLOT(addType()));
  leftLayout->addWidget(addTypeB, 1, 0);

  QPushButton *removeTypeB = new QPushButton(i18n("&Remove"), this);
  connect(removeTypeB, SIGNAL(clicked()),
          this, SLOT(removeType()));
  leftLayout->addWidget(removeTypeB, 1, 1);

  QVBoxLayout *rightLayout = new QVBoxLayout();
  topLayout->addLayout(rightLayout, 3);

  QHBoxLayout *hBox = new QHBoxLayout();
  rightLayout->addLayout(hBox);

  iconButton = new KIconLoaderButton(this);
  iconButton->setIconType("mimetypes");
  connect(iconButton, SIGNAL(iconChanged(const QString &)),
          SLOT(updateIcon(const QString &)));

  iconButton->setFixedSize(50, 50);
  hBox->addWidget(iconButton);

  QGroupBox *gb = new QGroupBox(i18n("File Patterns"), this);
  hBox->addWidget(gb);

  QGridLayout *grid = new QGridLayout(gb, 3, 2, KDialog::marginHint(),
                                      KDialog::spacingHint());
  grid->addRowSpacing(0, gb->fontMetrics().height());

  extensionLB = new QListBox(gb);
  connect(extensionLB, SIGNAL(highlighted(int)), SLOT(enableExtButtons(int)));
  grid->addMultiCellWidget(extensionLB, 1, 2, 0, 0);

  addExtButton = new QPushButton(i18n("Add..."), gb);
  addExtButton->setEnabled(false);
  connect(addExtButton, SIGNAL(clicked()),
          this, SLOT(addExtension()));
  grid->addWidget(addExtButton, 1, 1);

  removeExtButton = new QPushButton(i18n("Remove"), gb);
  removeExtButton->setEnabled(false);
  connect(removeExtButton, SIGNAL(clicked()),
          this, SLOT(removeExtension()));
  grid->addWidget(removeExtButton, 2, 1);

  gb = new QGroupBox(i18n("Description"), this);
  rightLayout->addWidget(gb);

  gb->setColumnLayout(1, Qt::Horizontal);
  description = new KLineEdit(gb);
  connect(description, SIGNAL(textChanged(const QString &)),
          SLOT(updateDescription(const QString &)));

  gb = new QGroupBox(i18n("Application Preference Order"), this);
  rightLayout->addWidget(gb);

  grid = new QGridLayout(gb, 5, 2, KDialog::marginHint(),
                         KDialog::spacingHint());
  grid->addRowSpacing(0, gb->fontMetrics().height());
  grid->setRowStretch(3, 1);

  servicesLB = new QListBox(gb);
  connect(servicesLB, SIGNAL(highlighted(int)), SLOT(enableMoveButtons(int)));
  grid->addMultiCellWidget(servicesLB, 1, 3, 0, 0);

  servUpButton = new QPushButton(i18n("Move &Up"), gb);
  servUpButton->setEnabled(false);
  connect(servUpButton, SIGNAL(clicked()), SLOT(promoteService()));
  grid->addWidget(servUpButton, 1, 1);

  servDownButton = new QPushButton(i18n("Move &Down"), gb);
  servDownButton->setEnabled(false);
  connect(servDownButton, SIGNAL(clicked()), SLOT(demoteService()));
  grid->addWidget(servDownButton, 2, 1);

  servNewButton = new QPushButton(i18n("Add..."), gb);
  servNewButton->setEnabled(false);
  connect(servNewButton, SIGNAL(clicked()), SLOT(addService()));
  grid->addWidget(servNewButton, 3, 1);

  //rightLayout->addStretch(1);

  init();
}

FileTypesView::~FileTypesView()
{
}

void FileTypesView::init()
{
  KMimeType::List mimetypes = KMimeType::allMimeTypes();
  QValueListIterator<KMimeType::Ptr> it2(mimetypes.begin());
  for (; it2 != mimetypes.end(); ++it2) {
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
      new TypesListItem(i, (*it2));
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
}

void FileTypesView::updateDisplay(QListViewItem *item)
{
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
  extensionLB->insertStringList(tlitem->patterns());


  servicesLB->clear();
  if (tlitem->defaultServices().count() == 0) {
    servicesLB->insertItem("None");
    servicesLB->setEnabled(false);
    servUpButton->setEnabled(false);
    servDownButton->setEnabled(false);
  } else {
    servicesLB->insertStringList(tlitem->defaultServices());
    servicesLB->setEnabled(true);
  }
}

void FileTypesView::updateIcon(const QString &icon)
{
  if (extensionLB->currentItem() == 0)
    return;

  TypesListItem *tli = (TypesListItem *) typesLV->currentItem();
  tli->setIcon(icon);
}

void FileTypesView::updateDescription(const QString &desc)
{
  if (extensionLB->currentItem() == 0)
    return;

  TypesListItem *tli = (TypesListItem *) typesLV->currentItem();
  tli->setComment(desc);
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

  QString sel = servicesLB->text(selIndex);
  servicesLB->removeItem(selIndex);
  servicesLB->insertItem(sel, selIndex-1);
  servicesLB->setCurrentItem(selIndex - 1);
  updatePreferredServices();
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

  QString sel = servicesLB->text(selIndex);
  servicesLB->removeItem(selIndex);
  servicesLB->insertItem(sel, selIndex+1);
  servicesLB->setCurrentItem(selIndex + 1);
  updatePreferredServices();
}

void FileTypesView::updatePreferredServices()
{ 
  if (typesLV->currentItem() == 0)
    return;
  TypesListItem *tli = (TypesListItem *) typesLV->currentItem();
  QStringList sl;
  unsigned int count = servicesLB->count();

  for (unsigned int i = 0; i < count; i++)
    sl.append(servicesLB->text(i));

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
  return didIt;
}

void FileTypesView::enableMoveButtons(int index)
{
  if (servicesLB->count() == 1)
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
  servicesLB->insertItem(service->name());

  updatePreferredServices();
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
