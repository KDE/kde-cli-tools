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
#ifndef _FILETYPEEMBEDCFG_H
#define _FILETYPEEMBEDCFG_H

#include <qwidget.h>
class TypesListItem;
class KIconButton;
class QLineEdit;
class QListBox;

/**
 * This widget contains the second page of the file configuration
 * dialog. It is implemented as a separate class so that it can be
 * used by the keditfiletype program to show the details of a single
 * mimetype.
 */
class FileTypeEmbedCfg : public QWidget
{
  Q_OBJECT
public:
  FileTypeEmbedCfg(QWidget *parent = 0, const char *name = 0);

  void setTypeItem( TypesListItem * item );

signals:
  void changed(bool);

protected slots:

protected:

private:
  KServiceListWidget *serviceListWidget;
  TypesListItem * m_item;
};

#endif
