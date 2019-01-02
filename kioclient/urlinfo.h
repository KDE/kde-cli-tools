/* This file is part of the KDE project
   Copyright (C) 2015 Milian Wolff <mail@milianw.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef URLINFO_H
#define URLINFO_H

#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QUrl>

/**
 * Represents a file to be opened, consisting of its URL and the cursor to jump to.
 */
class UrlInfo
{
public:
    /**
     * Parses an argument and determines its line number and column and full path
     * @param pathOrUrl path passed on e.g. command line to parse into an URL or just an URL
     */
    UrlInfo(const QString& pathOrUrl)
        : line(0), column(0)
    {
        /**
         * first try: just check if the path is an existing file
         */
        if (QFile::exists(pathOrUrl)) {
            /**
             * create absolute file path, we will e.g. pass this over dbus to other processes
             * and then we are done, no cursor can be detected here!
             */
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(pathOrUrl));
            return;
        }

        /**
         * ok, the path as is, is no existing file, now, cut away :xx:yy stuff as cursor
         * this will make test:50 to test with line 50
         */
        QString pathOrUrl2 = pathOrUrl;
        const auto match = QRegularExpression(QStringLiteral(":(\\d+)(?::(\\d+))?:?$")).match(pathOrUrl2);
        if (match.isValid()) {
            /**
             * cut away the line/column specification from the path
             */
            pathOrUrl2.chop(match.capturedLength());

            /**
             * set right cursor position
             */
            line = match.capturedRef(1).toUInt();
            column = match.capturedRef(2).toUInt();
        }

        /**
         * construct url:
         *   - make relative paths absolute using the current working directory
         *   - do not prefer local file, to be able to open things like foo.com in browser
         */
        url = QUrl::fromUserInput(pathOrUrl2, QDir::currentPath(), QUrl::DefaultResolution);

        /**
         * in some cases, this will fail, e.g. if you have line/column specs like test.c:10:1
         * => fallback: assume a local file and just convert it to an url
         */
        if (!url.isValid()) {
            /**
             * create absolute file path, we will e.g. pass this over dbus to other processes
             */
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(pathOrUrl2));
        }
    }

    bool atStart() const {
        return (line == 0 || line == 1 )
            && (column == 0 || column == 1);
    }

    /**
     * url computed out of the passed path or URL
     */
    QUrl url;

    /**
     * initial cursor position, if any found inside the path as line/column specification at the end
     */
    unsigned line, column;
};

#endif // URLINFO_H
