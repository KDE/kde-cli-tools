/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "filetypedetails.h"
#include "typeslistitem.h"
#include "keditfiletype.h"

#include <dcopclient.h>
#include <kapp.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <klocale.h>

FileTypeDialog::FileTypeDialog( KMimeType::Ptr mime )
  : KDialogBase( 0L, 0, true, QString::null, /* Help | */ Cancel | Apply | Ok,
                 Ok, false )
{
  m_details = new FileTypeDetails( this );
  QListView * dummyListView = new QListView( m_details );
  dummyListView->hide();
  m_item = new TypesListItem( dummyListView, mime );
  m_details->setTypeItem( m_item );

  // This code is very similar to kcdialog.cpp
  setMainWidget( m_details );
  connect(m_details, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
  // TODO setHelp()
  enableButton(Apply, false);
}

void FileTypeDialog::save()
{
  if (m_item->isDirty()) {
    m_item->sync();
    DCOPClient *dcc = kapp->dcopClient();
    dcc->send("kded", "kbuildsycoca", "recreate()", QByteArray());
  }
}

void FileTypeDialog::slotApply()
{
  save();
}

void FileTypeDialog::slotOk()
{
  save();
  accept();
}

void FileTypeDialog::clientChanged(bool state)
{
  // enable/disable buttons
  enableButton(User1, state);
  enableButton(Apply, state);
}

#include "keditfiletype.moc"

static KCmdLineOptions options[] =
{
  { "+mimetype",   I18N_NOOP("File type to edit (e.g. text/html)"), 0 },
  { 0, 0, 0}
};

int main(int argc, char ** argv)
{
  KServiceTypeProfile::setConfigurationMode();
  KLocale::setMainCatalogue("filetypes");
  KAboutData aboutData( "keditfiletype", I18N_NOOP("KEditFileType"), "1.0",
                        I18N_NOOP("KDE file type editor - simplified version for editing a single file type"),
                        KAboutData::License_GPL,
                        I18N_NOOP("(c) 2000, KDE developers") );
  aboutData.addAuthor("Preston Brown",0, "pbrown@kde.org");
  aboutData.addAuthor("David Faure",0, "faure@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication app;
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  KGlobal::locale()->insertCatalogue("filetypes");

  if (args->count() == 0)
    KCmdLineArgs::usage();
  KMimeType::Ptr mime = KMimeType::mimeType( args->arg(0) );
  if (!mime)
    kdFatal() << "Mimetype " << args->arg(0) << " not found" << endl;

  args->clear();
  FileTypeDialog * dlg = new FileTypeDialog( mime );
  dlg->setCaption( i18n("Edit File Type %1").arg(mime->name()) );
  dlg->exec();
  delete dlg;

  return 0;
}

