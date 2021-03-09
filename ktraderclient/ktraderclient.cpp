/*
 *  SPDX-FileCopyrightText: 2002, 2003 David Faure <faure@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-only
 */

#include <kmimetypetrader.h>
#include <kservicetypetrader.h>

#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <stdio.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("ktraderclient5");
    KAboutData aboutData(QLatin1String("ktraderclient"), i18n("KTraderClient"), QLatin1String(PROJECT_VERSION));
    aboutData.addAuthor(i18n("David Faure"), QString(), QStringLiteral("faure@kde.org"));

    aboutData.setShortDescription(i18n("A command-line tool for querying the KDE trader system"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("mimetype"), i18n("A MIME type"), QLatin1String("mimetype")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("servicetype"),
                                        i18n("A servicetype, like KParts/ReadOnlyPart or KMyApp/Plugin"),
                                        QLatin1String("servicetype")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("constraint"),
                                        i18n("A constraint expressed in the trader query language"),
                                        QLatin1String("constraint")));

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("short"), i18n("Output only paths to desktop files")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    const QString mimetype = parser.value(QStringLiteral("mimetype"));
    QString servicetype = parser.value(QStringLiteral("servicetype"));
    const QString constraint = parser.value(QStringLiteral("constraint"));
    const bool outputProperties = !parser.isSet(QStringLiteral("short"));

    if (mimetype.isEmpty() && servicetype.isEmpty()) {
        parser.showHelp();
    }

    if (!mimetype.isEmpty()) {
        printf("mimetype is : %s\n", qPrintable(mimetype));
    }
    if (!servicetype.isEmpty()) {
        printf("servicetype is : %s\n", qPrintable(servicetype));
    }
    if (!constraint.isEmpty()) {
        printf("constraint is : %s\n", qPrintable(constraint));
    }

    KService::List offers;
    if (!mimetype.isEmpty()) {
        if (servicetype.isEmpty()) {
            servicetype = QStringLiteral("Application");
        }
        offers = KMimeTypeTrader::self()->query(mimetype, servicetype, constraint);
    } else {
        offers = KServiceTypeTrader::self()->query(servicetype, constraint);
    }

    printf("got %d offers.\n", offers.count());

    int i = 0;
    KService::List::ConstIterator it = offers.constBegin();
    const KService::List::ConstIterator end = offers.constEnd();
    for (; it != end; ++it, ++i) {
        if (outputProperties) {
            printf("---- Offer %d ----\n", i);
            QStringList props = (*it)->propertyNames();
            QStringList::ConstIterator propIt = props.constBegin();
            QStringList::ConstIterator propEnd = props.constEnd();
            for (; propIt != propEnd; ++propIt) {
                QVariant prop = (*it)->property(*propIt);

                if (!prop.isValid()) {
                    printf("Invalid property %s\n", (*propIt).toLocal8Bit().data());
                    continue;
                }

                QString outp = *propIt;
                outp += QStringLiteral(" : '");

                switch (prop.type()) {
                case QVariant::StringList:
                    outp += prop.toStringList().join(QStringLiteral(" - "));
                    break;
                case QVariant::Bool:
                    outp += prop.toBool() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
                    break;
                default:
                    outp += prop.toString();
                    break;
                }

                if (!outp.isEmpty()) {
                    printf("%s'\n", outp.toLocal8Bit().constData());
                }
            }
        } else {
            printf("%s\n", (*it)->entryPath().toLocal8Bit().constData());
        }
    }
    return 0;
}
