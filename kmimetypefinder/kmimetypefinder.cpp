/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
 *  Copyright (C) 2008 Pino Toscano  <pino@kde.org>
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

#include <KLocalizedString>
#include <KAboutData>

#include <QFile>
#include <QCoreApplication>
#include <QMimeDatabase>
#include <QMimeType>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("kmimetypefinder5");

    KAboutData aboutData( QLatin1String("kmimetypefinder"), i18n("MIME Type Finder"), QLatin1String(PROJECT_VERSION ));
    aboutData.setShortDescription(i18n("Gives the MIME type for a given file"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("c") << QLatin1String("content"), i18n("Use only the file content for determining the MIME type.")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("f") << QLatin1String("filename-only"), i18n("Whether use the file name only for determining the MIME type. Not used if -c is specified.")));
    parser.addPositionalArgument(QLatin1String("filename"), i18n("The filename to test. '-' to read from stdin."));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if( parser.positionalArguments().count() < 1 ) {
        printf( "No filename specified\n" );
        return 1;
    }
    const QString fileName = parser.positionalArguments().at( 0 );
    QMimeDatabase db;
    QMimeType mime;
    if (fileName == QLatin1String("-")) {
        QFile qstdin;
        qstdin.open(stdin, QIODevice::ReadOnly);
        const QByteArray data = qstdin.readAll();
        mime = db.mimeTypeForData(data);
    } else if (parser.isSet(QStringLiteral("c"))) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchContent);
    } else if (parser.isSet(QStringLiteral("f"))) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
    } else {
        mime = db.mimeTypeForFile(fileName);
    }
    if ( !mime.isDefault() ) {
        printf("%s\n", mime.name().toLatin1().constData());
    } else {
        return 1; // error
    }

    return 0;
}
