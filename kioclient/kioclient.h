/* This file is part of the KDE project
   Copyright (C) 1999-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kioclient_h
#define __kioclient_h

#include <QApplication>
#include <kio/udsentry.h>

class QCommandLineParser;
class QUrl;
class KJob;
namespace KIO { class Job; }

class ClientApp : public QObject
{
    Q_OBJECT
public:
    ClientApp();

    /** Parse command-line arguments and "do it" */
    bool doIt(const QCommandLineParser& parser);

private Q_SLOTS:
    void slotPrintData(KIO::Job *job, const QByteArray &data);
    void slotEntries(KIO::Job* job, const KIO::UDSEntryList& );
    void slotResult( KJob * );
    void delayedQuit();
    void slotDialogCanceled();

private:
    bool kde_open( const QUrl& url, const QString& mimeType, bool allowExec );
    bool doCopy( const QStringList& urls );
    bool doMove( const QStringList& urls );
    bool doList( const QStringList& urls );
    bool doRemove( const QStringList& urls );

    static bool m_ok;
};

#endif
