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

#include "kserviceselectdlg.h"
#include "kserviceselectdlg.moc"
#include "kservicelistwidget.h"
#include <qvbox.h>
#include <qlayout.h>

KServiceSelectDlg::KServiceSelectDlg( const QString& /*serviceType*/, const QString& /*value*/, QWidget *parent )
    : KDialogBase( parent, "serviceSelectDlg", true,
                   /* TODO caption */ QString::null, Ok|Cancel, Ok )
{
    QVBox *topcontents = new QVBox ( this );
    topcontents->setSpacing(KDialog::spacingHint());
    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint());

    lay->addStretch(1);

    m_listbox=new KListBox( topcontents );

    // Can't make a KTrader query since we don't have a servicetype to give,
    // we want all services that are not applications.......
    // So we have to do it the slow way
    // ### Why can't we query for KParts/ReadOnlyPart as the servicetype? Should work fine!
    KService::List allServices = KService::allServices();
    QValueListIterator<KService::Ptr> it(allServices.begin());
    for ( ; it != allServices.end() ; ++it )
      if ( (*it)->hasServiceType( "KParts/ReadOnlyPart" ) )
      {
          m_listbox->insertItem( new KServiceListItem( (*it)->desktopEntryPath(), KServiceListWidget::SERVICELIST_SERVICES ) );
      }

    m_listbox->sort();
    m_listbox->setMinimumHeight(350);
    m_listbox->setMinimumWidth(300);
    connect(m_listbox,SIGNAL(doubleClicked ( QListBoxItem * )),SLOT(slotOk()));
    setMainWidget(topcontents);
}

KServiceSelectDlg::~KServiceSelectDlg()
{
}

KService::Ptr KServiceSelectDlg::service()
{
    unsigned int selIndex = m_listbox->currentItem();
    KServiceListItem *selItem = static_cast<KServiceListItem *>(m_listbox->item(selIndex));
    return KService::serviceByDesktopPath( selItem->desktopPath );
}
