/*
 *   SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KAboutData>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("plasma-open-settings");

    KAboutData aboutData(QStringLiteral("plasma-open-settings"), //
                         i18n("App to open Settings app"),
                         QLatin1String(PROJECT_VERSION),
                         i18n("A tool to start system settings"),
                         KAboutLicense::GPL,
                         i18n("(c) 2021, The KDE Developers"));

    aboutData.addAuthor(QStringLiteral("Aleix Pol i Gonzalez"), {}, QStringLiteral("aleixpol@kde.org"));

    const QUrl url(app.arguments().constLast());
    QString moduleName = url.host().isEmpty() ? url.path() : url.host();
    if (moduleName.startsWith('/')) {
        moduleName = moduleName.mid(1);
    }

    QString args;
    if (int idx = moduleName.indexOf('/'); idx > 0) {
        args = moduleName.mid(idx + 1);
        moduleName = moduleName.left(idx);
    } else {
        args = url.path();
        args = args.mid(1);
    }

    KIO::CommandLauncherJob *job = nullptr;
    int ret = 0;
    if (!QStandardPaths::findExecutable("systemsettings").isEmpty()) {
        job = new KIO::CommandLauncherJob(QStringLiteral("systemsettings"), {moduleName, QStringLiteral("--args"), args});
        job->setDesktopName(QStringLiteral("org.kde.systemsettings"));
    } else if (!QStandardPaths::findExecutable("plasma-settings").isEmpty()) {
        // TODO needs --args support in plasma-settings
        job = new KIO::CommandLauncherJob(QStringLiteral("plasma-settings"), {"-m", moduleName});
        job->setDesktopName(QStringLiteral("org.kde.plasma.settings"));
    } else if (!QStandardPaths::findExecutable("kcmshell5").isEmpty()) {
        job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell5"), {moduleName, QStringLiteral("--args"), args});
    } else if (!QStandardPaths::findExecutable("kdialog").isEmpty()) {
        job = new KIO::CommandLauncherJob(QStringLiteral("kdialog"), {"--error", i18n("Could not open: %1", moduleName)});
        ret = 1;
    } else {
        QTextStream err(stderr);
        err << "Could not open:" << moduleName << url.toString() << Qt::endl;
        return 32;
    }

    if (!qEnvironmentVariableIsEmpty("XDG_ACTIVATION_TOKEN")) {
        job->setStartupId(qgetenv("XDG_ACTIVATION_TOKEN"));
    }
    return !job->exec() + ret;
}
