/*  This file is part of the KDE project
    Copyright (C) 2007, 2008 David Faure <faure@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mimetypewriter.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QXmlStreamWriter>

#include <kprocess.h>

class MimeTypeWriterPrivate
{
public:
    QString localFilePath() const;

    QString m_mimeType;
    QString m_comment;
    QString m_iconName;
    QStringList m_patterns;
    QString m_marker;
};

MimeTypeWriter::MimeTypeWriter(const QString& mimeType)
    : d(new MimeTypeWriterPrivate)
{
    d->m_mimeType = mimeType;
    Q_ASSERT(!mimeType.isEmpty());
}

MimeTypeWriter::~MimeTypeWriter()
{
    delete d;
}

void MimeTypeWriter::setComment(const QString& comment)
{
    d->m_comment = comment;
}

void MimeTypeWriter::setPatterns(const QStringList& patterns)
{
    d->m_patterns = patterns;
}

void MimeTypeWriter::setIconName(const QString& iconName)
{
    d->m_iconName = iconName;
}

void MimeTypeWriter::setMarker(const QString& marker)
{
    d->m_marker = marker;
}

bool MimeTypeWriter::write()
{
    const QString packageFileName = d->localFilePath();
    qDebug() << "writing" << packageFileName;
    QFile packageFile(packageFileName);
    if (!packageFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Couldn't open" << packageFileName << "for writing";
        return false;
    }
    QXmlStreamWriter writer(&packageFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    if (!d->m_marker.isEmpty()) {
        writer.writeComment(d->m_marker);
    }
    const QString nsUri = QStringLiteral("http://www.freedesktop.org/standards/shared-mime-info");
    writer.writeDefaultNamespace(nsUri);
    writer.writeStartElement(QStringLiteral("mime-info"));
    writer.writeStartElement(nsUri, QStringLiteral("mime-type"));
    writer.writeAttribute(QStringLiteral("type"), d->m_mimeType);

    if (!d->m_comment.isEmpty()) {
        writer.writeStartElement(nsUri, QStringLiteral("comment"));
        writer.writeCharacters(d->m_comment);
        writer.writeEndElement(); // comment
    }

    if (!d->m_iconName.isEmpty()) {
        // User-specified icon name
        writer.writeStartElement(nsUri, QStringLiteral("icon"));
        writer.writeAttribute(QStringLiteral("name"), d->m_iconName);
        writer.writeEndElement(); // icon
    }

    // Allow this local definition to override the global definition
    writer.writeStartElement(nsUri, QStringLiteral("glob-deleteall"));
    writer.writeEndElement(); // glob-deleteall

    foreach(const QString& pattern, d->m_patterns) {
        writer.writeStartElement(nsUri, QStringLiteral("glob"));
        writer.writeAttribute(QStringLiteral("pattern"), pattern);
        writer.writeEndElement(); // glob
    }

    writer.writeEndElement(); // mime-info
    writer.writeEndElement(); // mime-type
    writer.writeEndDocument();
    return true;
}

void MimeTypeWriter::runUpdateMimeDatabase()
{
    const QString localPackageDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/");
    Q_ASSERT(!localPackageDir.isEmpty());
    KProcess proc;
    proc << QStringLiteral("update-mime-database");
    proc << localPackageDir;
    const int exitCode = proc.execute();
    if (exitCode) {
        qWarning() << proc.program() << "exited with error code" << exitCode;
    }
}

QString MimeTypeWriterPrivate::localFilePath() const
{
    // XDG shared mime: we must write into a <kdehome>/share/mime/packages/ file...
    // To simplify our job, let's use one "input" file per mimetype, in the user's dir.
    // (this writes into $HOME/.local/share/mime by default)
    //
    // We could also use Override.xml, says the spec, but then we'd need to merge with other mimetypes,
    // and in ~/.local we don't really expect other packages to be installed anyway...
    QString baseName = m_mimeType;
    baseName.replace(QLatin1Char('/'), QLatin1Char('-'));
    QString packagesDirName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/") + QStringLiteral("packages/");
    // create the directory, the saving will fail if it doesn't exist (bug#356237)
    QDir(packagesDirName).mkpath(QStringLiteral("."));
    return packagesDirName + baseName + QStringLiteral(".xml");
}

static QString existingDefinitionFile(const QString& mimeType)
{
    QString baseName = mimeType;
    baseName.replace(QLatin1Char('/'), QLatin1Char('-'));
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("mime/") + QStringLiteral("packages/") + baseName + QStringLiteral(".xml") );
}

bool MimeTypeWriter::hasDefinitionFile(const QString& mimeType)
{
    return !existingDefinitionFile(mimeType).isEmpty();
}

void MimeTypeWriter::removeOwnMimeType(const QString& mimeType)
{
    const QString file = existingDefinitionFile(mimeType);
    Q_ASSERT(!file.isEmpty());
    QFile::remove(file);
    // We must also remove the generated XML file, update-mime-database doesn't do that, for unknown media types
    QString xmlFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("mime/") + mimeType + QStringLiteral(".xml") );
    QFile::remove(xmlFile);
}

/// WARNING: this code is duplicated between apps/nsplugins and runtime/filetypes
