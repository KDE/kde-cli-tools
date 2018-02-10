/*
* Copyright 2016 Kai Uwe Broulik <kde@privat.broulik.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>

#include <KAboutData>
#include <KLocalizedString>

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kbroadcastnotification");

    KAboutData aboutData("kbroadcastnotification", i18n("Broadcast Notifications"),
                         PROJECT_VERSION,
                         i18n("A tool that emits a notification for all users by sending it on the system DBus"),
                         KAboutLicense::GPL,
                         i18n("(c) 2016 Kai Uwe Broulik"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption applicationNameOption(QStringLiteral("application"), i18n("Name of the application that should be associated with this notification"), QStringLiteral("application"));
    parser.addOption(applicationNameOption);
    QCommandLineOption summaryOption(QStringLiteral("summary"), i18n("A brief one-line summary of the notification"), QStringLiteral("summary"));
    parser.addOption(summaryOption);
    QCommandLineOption iconOption(QStringLiteral("icon"), i18n("Icon for the notification"), QStringLiteral("icon"));
    parser.addOption(iconOption);
    QCommandLineOption uidsOption(QStringLiteral("uids"), i18n("A comma-separated list of user IDs this notification should be sent to. If omitted, the notification will be sent to all users."), QStringLiteral("uids"));
    parser.addOption(uidsOption);
    QCommandLineOption timeoutOption(QStringLiteral("timeout"), i18n("Timeout for the notification"), QStringLiteral("timeout"));
    parser.addOption(timeoutOption);
    QCommandLineOption persistentOption(QStringLiteral("persistent"), i18n("Keep the notification in the history until the user closes it"));
    parser.addOption(persistentOption);

    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument(QStringLiteral("body"), i18n("The actual notification body text"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    QVariantMap properties;
    if (parser.isSet(applicationNameOption)) {
        properties.insert(QStringLiteral("appName"), parser.value(applicationNameOption));
    }
    if (parser.isSet(summaryOption)) {
        properties.insert(QStringLiteral("summary"), parser.value(summaryOption));
    }
    if (parser.isSet(iconOption)) {
        properties.insert(QStringLiteral("appIcon"), parser.value(iconOption));
    }

    if (parser.isSet(uidsOption)) {
        const QStringList &uids = parser.value(uidsOption).split(QLatin1Char(','), QString::SkipEmptyParts);
        if (!uids.isEmpty()) {
            properties.insert(QStringLiteral("uids"), uids);
        }
    }

    if (parser.isSet(timeoutOption)) {
        bool ok;
        const int timeout = parser.value(timeoutOption).toInt(&ok);
        if (ok) {
            properties.insert(QStringLiteral("timeout"), timeout);
        }
    }
    if (parser.isSet(persistentOption)) { // takes precedence over timeout if both are set
        properties.insert(QStringLiteral("timeout"), 0); // 0 = persistent, -1 = server default
    }

    const auto &positionalArgs = parser.positionalArguments();
    if (positionalArgs.count() == 1) {
        properties.insert(QStringLiteral("body"), positionalArgs.first());
    }

    if (properties.isEmpty()) {
        parser.showHelp(1); // never returns
    }

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/org/kde/kbroadcastnotification"),
                                                      QStringLiteral("org.kde.BroadcastNotifications"),
                                                      QStringLiteral("Notify"));
    message.setArguments({properties});
    QDBusConnection::systemBus().send(message);

    // FIXME can we detect that the message was sent to the bus?
    QTimer::singleShot(500, &app, QCoreApplication::quit);

    return app.exec();
}
