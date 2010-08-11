/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kcmdlineargs.h>
#include <kmimetypetrader.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <klocale.h>
#include <kservicetypetrader.h>

#include <stdio.h>

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "ktraderclient", 0, ki18n("KTraderClient"), "0.0" , ki18n("A command-line tool for querying the KDE trader system"));


  KCmdLineOptions options;

  options.add("mimetype <mimetype>", ki18n("A mimetype"));

  options.add("servicetype <servicetype>", ki18n("A servicetype, like KParts/ReadOnlyPart or KMyApp/Plugin"));

  options.add("constraint <constraint>", ki18n("A constraint expressed in the trader query language"));

  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false ); // no GUI

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  const QString mimetype = args->getOption( "mimetype" );
  QString servicetype = args->getOption( "servicetype" );
  const QString constraint = args->getOption( "constraint" );

  if ( mimetype.isEmpty() && servicetype.isEmpty() )
      KCmdLineArgs::usage();

  if ( !mimetype.isEmpty() )
      printf( "mimetype is : %s\n", qPrintable( mimetype ) );
  if ( !servicetype.isEmpty() )
      printf( "servicetype is : %s\n", qPrintable( servicetype ) );
  if ( !constraint.isEmpty() )
      printf( "constraint is : %s\n", qPrintable( constraint ) );

  KService::List offers;
  if ( !mimetype.isEmpty() ) {
      if ( servicetype.isEmpty() )
          servicetype = "Application";
     offers = KMimeTypeTrader::self()->query( mimetype, servicetype, constraint );
  }
  else
     offers = KServiceTypeTrader::self()->query( servicetype, constraint );

  printf("got %d offers.\n", offers.count());

  int i = 0;
  KService::List::ConstIterator it = offers.constBegin();
  const KService::List::ConstIterator end = offers.constEnd();
  for (; it != end; ++it, ++i )
  {
    printf("---- Offer %d ----\n", i);
    QStringList props = (*it)->propertyNames();
    QStringList::ConstIterator propIt = props.constBegin();
    QStringList::ConstIterator propEnd = props.constEnd();
    for (; propIt != propEnd; ++propIt )
    {
      QVariant prop = (*it)->property( *propIt );

      if ( !prop.isValid() )
      {
        printf("Invalid property %s\n", (*propIt).toLocal8Bit().data());
	continue;
      }

      QString outp = *propIt;
      outp += " : '";

      switch ( prop.type() )
      {
        case QVariant::StringList:
          outp += prop.toStringList().join(" - ");
        break;
        case QVariant::Bool:
          outp += prop.toBool() ? "TRUE" : "FALSE";
          break;
        default:
          outp += prop.toString();
        break;
      }

      if ( !outp.isEmpty() )
        printf("%s'\n", outp.toLocal8Bit().data());
    }
  }
  return 0;
}

