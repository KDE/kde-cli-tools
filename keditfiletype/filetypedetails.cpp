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

#include "kservicelistwidget.h"
#include "filetypedetails.h"
#include "typeslistitem.h"

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

  serviceListWidget = new KServiceListWidget( KServiceListWidget::SERVICELIST_APPLICATIONS, this );
  connect( serviceListWidget, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
  rightLayout->addWidget(serviceListWidget, 1);
}

void FileTypeDetails::updateIcon(QString icon)
{
  if (!m_item)
    return;

  m_item->setIcon(icon);

  emit changed(true);
}

void FileTypeDetails::updateDescription(const QString &desc)
{
  if (!m_item)
    return;

  m_item->setComment(desc);

  emit changed(true);
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

      emit changed(true);
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

  emit changed(true);
}

void FileTypeDetails::setTypeItem( TypesListItem * tlitem )
{
  m_item = tlitem;
  if ( tlitem )
    iconButton->setIcon(tlitem->icon());
  else
    iconButton->resetIcon();
  description->setText(tlitem ? tlitem->comment() : QString::null);
  extensionLB->clear();
  addExtButton->setEnabled(true);
  removeExtButton->setEnabled(false);

  serviceListWidget->setTypeItem( tlitem );

  if ( tlitem )
    extensionLB->insertStringList(tlitem->patterns());
  else
    extensionLB->clear();
}

void FileTypeDetails::enableExtButtons(int /*index*/)
{
  removeExtButton->setEnabled(true);
}

#include "filetypedetails.moc"
