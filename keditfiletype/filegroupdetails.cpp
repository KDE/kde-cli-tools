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
#include "filegroupdetails.h"
#include "typeslistitem.h"
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <kdialog.h>
#include <klocale.h>

FileGroupDetails::FileGroupDetails(QWidget *parent, const char *name )
    : QWidget( parent, name )
{
  QWidget * parentWidget = this;
  QVBoxLayout *secondLayout = new QVBoxLayout(parentWidget, KDialog::marginHint(),
                                              KDialog::spacingHint());

  m_autoEmbed = new QButtonGroup( i18n("Left click action"), parentWidget );
  secondLayout->addWidget( m_autoEmbed, 1 );
  secondLayout->addWidget( new QWidget( parentWidget ), 100 );
  QVBoxLayout *bgLay = new QVBoxLayout(m_autoEmbed, KDialog::marginHint(),
                                       KDialog::spacingHint());
  bgLay->addSpacing(10);
  // The order of those three items is very important. If you change it, fix typeslistitem.cpp !
  bgLay->addWidget( new QRadioButton( i18n("Show file in embedded viewer"), m_autoEmbed ) );
  bgLay->addWidget( new QRadioButton( i18n("Show file in separate viewer"), m_autoEmbed ) );
  connect(m_autoEmbed, SIGNAL( clicked( int ) ), SLOT( slotAutoEmbedClicked( int ) ));

  QWhatsThis::add( m_autoEmbed, i18n("Here you can configure what the Konqueror file manager"
    " will do when you click on a file belonging to this group. Konqueror can display the file in"
    " an embedded viewer or start up a separate application. You can change this setting for a"
    " specific file type in the 'Embedding' tab of the file type configuration.") );

  secondLayout->addSpacing(10);
}

void FileGroupDetails::setTypeItem( TypesListItem * item )
{
  ASSERT( item->isMeta() );
  m_item = item;
  m_autoEmbed->setButton( item ? item->autoEmbed() : -1 );
}

void FileGroupDetails::slotAutoEmbedClicked(int button)
{
  if ( !m_item )
    return;
  m_item->setAutoEmbed( button );
  emit changed(true);
}

#include "filegroupdetails.moc"
