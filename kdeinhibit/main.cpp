/********************************************************************
 This file is part of the KDE project.

Copyright (C) 2020 David Edmundson <davidedmundson@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

#include <KProcess>
#include <KLocalizedString>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("kdeinhibit"));
    QCommandLineParser parser;
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    parser.setApplicationDescription(i18n("Inhibit various desktop functions whilst a command runs"));

    QCommandLineOption powerOption(QStringLiteral("power"), i18n("Inhibit power management"));
    parser.addOption(powerOption);

    QCommandLineOption screenSaverOption(QStringLiteral("screenSaver"), i18n("Inhibit screensaver"));
    parser.addOption(screenSaverOption);

    QCommandLineOption colorCorrectOption(QStringLiteral("colorCorrect"), i18n("Inhibit colour correction (night mode)"));
    parser.addOption(colorCorrectOption);

    parser.addPositionalArgument(QStringLiteral("command..."), i18n("Command to run"));

    parser.addHelpOption();

    parser.process(app);

    QStringList command = parser.positionalArguments();

    if (command.isEmpty()) {
        parser.showHelp(-1);
    }

    auto warnOnError = [](const QDBusMessage &reply) {
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "Inhibit failed" << reply.errorMessage();
        }
    };

    if (parser.isSet(powerOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                                                                  QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"),
                                                                  QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                                                                  QStringLiteral("Inhibit"));
        inhibitCall.setArguments({i18nc("Script as in shell script", "Running Script"), command.first()});
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
        // ignore reply with the cookie as we don't really care if it failed
    }
    if (parser.isSet(screenSaverOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                                  QStringLiteral("/org/freedesktop/ScreenSaver"),
                                                                  QStringLiteral("org.freedesktop.ScreenSaver"),
                                                                  QStringLiteral("Inhibit"));
        inhibitCall.setArguments({i18nc("Script as in shell script", "Running Script"), command.first()});
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
    }
    if (parser.isSet(colorCorrectOption)) {
        QDBusMessage inhibitCall = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                                  QStringLiteral("/ColorCorrect"),
                                                                  QStringLiteral("org.kde.kwin.ColorCorrect"),
                                                                  QStringLiteral("inhibit"));
        // no arguments needed
        warnOnError(QDBusConnection::sessionBus().call(inhibitCall));
    }

    // finally run the script and exit when we're done
    KProcess::execute(command);

    // we don't explicitly uninhibit as closing our DBus connection will release everything automatically
}
