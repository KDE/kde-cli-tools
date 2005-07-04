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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "filetypedetails.h"
#include "typeslistitem.h"
#include "keditfiletype.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <ksycoca.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

FileTypeDialog::FileTypeDialog( KMimeType::Ptr mime )
  : KDialogBase( 0L, 0, false, QString::null, /* Help | */ Cancel | Apply | Ok,
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

  connect( KSycoca::self(), SIGNAL( databaseChanged() ), SLOT( slotDatabaseChanged() ) );
}

void FileTypeDialog::save()
{
  if (m_item->isDirty()) {
    m_item->sync();
    KService::rebuildKSycoca(this);
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

void FileTypeDialog::slotDatabaseChanged()
{
  if ( KSycoca::self()->isChanged( "mime" ) )
  {
      m_item->refresh();
  }
}

#include "keditfiletype.moc"

static KCmdLineOptions options[] =
{
  { "parent <winid>", I18N_NOOP("Makes the dialog transient for the window specified by winid"), 0 },
  { "+mimetype",   I18N_NOOP("File type to edit (e.g. text/html)"), 0 },
  KCmdLineLastOption
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

  if (args->count() == 0)
    KCmdLineArgs::usage();
  KMimeType::Ptr mime = KMimeType::mimeType( args->arg(0) );
  if (!mime)
    kdFatal() << "Mimetype " << args->arg(0) << " not found" << endl;

  FileTypeDialog dlg( mime );
#if defined Q_WS_X11
  if( args->isSet( "parent" )) {
    bool ok;
    long id = args->getOption("parent").toLong(&ok);
    if (ok)
      XSetTransientForHint( qt_xdisplay(), dlg.winId(), id );
  }
#endif
  args->clear();
  dlg.setCaption( i18n("Edit File Type %1").arg(mime->name()) );
  app.setMainWidget( &dlg );
  dlg.show(); // non-modal

  return app.exec();
}

