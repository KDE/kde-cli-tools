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
#include <qvbox.h>
#include <qlayout.h>
#include <klistbox.h>

KServiceSelectDlg::KServiceSelectDlg( const QString& /*serviceType*/, const QString& /*value*/, QWidget *parent )
    : KDialogBase( parent, "serviceSelectDlg", true,
                   /* TODO caption */ QString::null,
                   KDialogBase::Ok | KDialogBase::Cancel,
                   KDialogBase::Ok, true )
{
    QVBox *topcontents = new QVBox ( this );
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint()*2);
    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint()*2);

    lay->addStretch(1);

    m_listbox=new KListBox( topcontents );
    QStringList strList;

    // Can't make a KTrader query since we don't have a servicetype to give,
    // we want all services that are not applications.......
    // So we have to do it the slow way
    KService::List allServices = KService::allServices();
    QValueListIterator<KService::Ptr> it(allServices.begin());
    for ( ; it != allServices.end() ; ++it )
      if ( (*it)->hasServiceType( "KParts/ReadOnlyPart" ) )
        strList += (*it)->name();

    strList.sort();
    m_listbox->insertStringList( strList );
    m_listbox->setMinimumHeight(350);
    m_listbox->setMinimumWidth(300);

    setMainWidget(topcontents);
    enableButtonSeparator(false);
}

KServiceSelectDlg::~KServiceSelectDlg()
{
}

KService::Ptr KServiceSelectDlg::service()
{
    // If we have a problem with duplicates, we'll
    // need a QStringList of desktop entry paths and use
    // the current item in it
    QString text = m_listbox->currentText();
    return KService::serviceByName( text );
}
