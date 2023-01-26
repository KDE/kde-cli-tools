/*
 * kstart.C. Part of the KDE project.
 *
 * SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
 * SPDX-FileCopyrightText: David Faure <faure@kde.org>
 * SPDX-FileCopyrightText: Richard Moore <rich@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <config-kde-cli-tools.h>

#include "kstart.h"
#include <ktoolinvocation.h>

#include <fcntl.h>
#include <iostream>
#include <stdlib.h>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDesktopWidget>
#include <QRegExp>
#include <QScreen>
#include <QTimer>
#include <QUrl>

#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kprocess.h>
#include <kwindowsystem.h>

#include <kxmessages.h>

#include <KIO/ApplicationLauncherJob>
#include <KIO/CommandLauncherJob>

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <netwm.h>

// some globals

static QString servicePath; // TODO KF6 remove
static QString serviceName;
static QString exe;
static QStringList exeArgs;
static QString url;
static QString windowtitle;
static QString windowclass;
static int desktop = 0;
static bool activate = false;
static bool iconify = false;
static bool fullscreen = false;
static NET::States state = {};
static NET::States mask = {};
static NET::WindowType windowtype = NET::Unknown;

KStart::KStart()
    : QObject()
{
    // connect to window add to get the NEW windows
    connect(KWindowSystem::self(), &KWindowSystem::windowAdded, this, &KStart::windowAdded);

    // finally execute the comand
    if (!servicePath.isEmpty()) { // TODO KF6 remove
        QString error;
        QString dbusService;
        int pid;
        if (KToolInvocation::startServiceByDesktopPath(exe, url, &error, &dbusService, &pid) == 0) {
            printf("%s\n", qPrintable(dbusService));
        } else {
            qCritical() << error;
        }
    } else if (!serviceName.isEmpty()) {
        KService::Ptr service = KService::serviceByDesktopName(serviceName);
        if (!service) {
            qCritical() << "No such service" << exe;
        } else {
            auto *job = new KIO::ApplicationLauncherJob(service);
            if (!url.isEmpty()) {
                job->setUrls({QUrl(url)}); // TODO use QUrl::fromUserInput(PreferLocalFile)?
            }
            job->exec();
            if (job->error()) {
                qCritical() << job->errorString();
            }
        }
    } else {
        auto *job = new KIO::CommandLauncherJob(exe, exeArgs);
        job->exec();
    }

    QTimer::singleShot(120 * 1000, qApp, SLOT(quit()));
}

const NET::WindowTypes SUPPORTED_WINDOW_TYPES_MASK = NET::NormalMask | NET::DesktopMask | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
    | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask;

void KStart::windowAdded(WId w)
{
    KWindowInfo info(w, NET::WMWindowType | NET::WMName);

    // always ignore these window types
    if (info.windowType(SUPPORTED_WINDOW_TYPES_MASK) == NET::TopMenu || info.windowType(SUPPORTED_WINDOW_TYPES_MASK) == NET::Toolbar
        || info.windowType(SUPPORTED_WINDOW_TYPES_MASK) == NET::Desktop) {
        return;
    }

    if (!windowtitle.isEmpty()) {
        QString title = info.name().toLower();
        QRegExp r(windowtitle.toLower());
        if (!r.exactMatch(title)) {
            return; // no match
        }
    }
    if (windowtitle.isEmpty() && windowclass.isEmpty()) {
        // accept only "normal" windows
        if (info.windowType(SUPPORTED_WINDOW_TYPES_MASK) != NET::Unknown && info.windowType(SUPPORTED_WINDOW_TYPES_MASK) != NET::Normal
            && info.windowType(SUPPORTED_WINDOW_TYPES_MASK) != NET::Dialog) {
            return;
        }
    }
    applyStyle(w);
    QApplication::exit();
}

void KStart::applyStyle(WId w)
{
    if (state || iconify || windowtype != NET::Unknown || desktop >= 1) {
        XWithdrawWindow(QX11Info::display(), w, QX11Info::appScreen());
    }

    NETWinInfo info(QX11Info::connection(), w, QX11Info::appRootWindow(), NET::WMState, NET::Properties2());

    if ((desktop > 0 && desktop <= KWindowSystem::numberOfDesktops()) || desktop == NETWinInfo::OnAllDesktops) {
        info.setDesktop(desktop);
    }

    if (iconify) {
        XWMHints *hints = XGetWMHints(QX11Info::display(), w);
        if (hints) {
            hints->flags |= StateHint;
            hints->initial_state = IconicState;
            XSetWMHints(QX11Info::display(), w, hints);
            XFree(hints);
        }
    }

    if (windowtype != NET::Unknown) {
        info.setWindowType(windowtype);
    }

    if (state) {
        info.setState(state, mask);
    }

    if (fullscreen) {
        QRect r = QGuiApplication::primaryScreen()->geometry();
        XMoveResizeWindow(QX11Info::display(), w, r.x(), r.y(), r.width(), r.height());
    }

    XSync(QX11Info::display(), False);

    XMapWindow(QX11Info::display(), w);
    XSync(QX11Info::display(), False);

    if (activate) {
        KWindowSystem::forceActiveWindow(w);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kstart5");

    KAboutData aboutData(QStringLiteral("kstart"),
                         i18n("KStart"),
                         QString::fromLatin1(PROJECT_VERSION),
                         i18n(""
                              "Utility to launch applications with special window properties \n"
                              "such as iconified, maximized, a certain virtual desktop, a special decoration\n"
                              "and so on."),
                         KAboutLicense::GPL,
                         i18n("(C) 1997-2000 Matthias Ettrich (ettrich@kde.org)"));

    aboutData.addAuthor(i18n("Matthias Ettrich"), QString(), QStringLiteral("ettrich@kde.org"));
    aboutData.addAuthor(i18n("David Faure"), QString(), QStringLiteral("faure@kde.org"));
    aboutData.addAuthor(i18n("Richard J. Moore"), QString(), QStringLiteral("rich@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("!+command"), i18n("Command to execute")));
    // TODO KF6 remove
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("service"),
                           i18n("Alternative to <command>: desktop file path to start. D-Bus service will be printed to stdout. Deprecated: use --application"),
                           QLatin1String("desktopfile")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("application"),
                                        i18n("Alternative to <command>: desktop file to start."),
                                        QLatin1String("desktopfile")));
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("url"), i18n("Optional URL to pass <desktopfile>, when using --service"), QLatin1String("url")));
    // "!" means: all options after command are treated as arguments to the command
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("window"), i18n("A regular expression matching the window title"), QLatin1String("regexp")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("windowclass"),
                                        i18n("A string matching the window class (WM_CLASS property)\n"
                                             "The window class can be found out by running\n"
                                             "'xprop | grep WM_CLASS' and clicking on a window\n"
                                             "(use either both parts separated by a space or only the right part).\n"
                                             "NOTE: If you specify neither window title nor window class,\n"
                                             "then the very first window to appear will be taken;\n"
                                             "omitting both options is NOT recommended."),
                                        QLatin1String("class")));
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("desktop"), i18n("Desktop on which to make the window appear"), QLatin1String("number")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("currentdesktop"),
                                        i18n("Make the window appear on the desktop that was active\nwhen starting the application")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("alldesktops"), i18n("Make the window appear on all desktops")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("iconify"), i18n("Iconify the window")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("maximize"), i18n("Maximize the window")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("maximize-vertically"), i18n("Maximize the window vertically")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("maximize-horizontally"), i18n("Maximize the window horizontally")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("fullscreen"), i18n("Show window fullscreen")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("type"),
                                        i18n("The window type: Normal, Desktop, Dock, Toolbar, \nMenu, Dialog, TopMenu or Override"),
                                        QLatin1String("type")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("activate"),
                                        i18n("Jump to the window even if it is started on a \n"
                                             "different virtual desktop")));
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("ontop") << QLatin1String("keepabove"), i18n("Try to keep the window above other windows")));
    parser.addOption(
        QCommandLineOption(QStringList() << QLatin1String("onbottom") << QLatin1String("keepbelow"), i18n("Try to keep the window below other windows")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("skiptaskbar"), i18n("The window does not get an entry in the taskbar")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("skippager"), i18n("The window does not get an entry on the pager")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.isSet(QStringLiteral("service"))) {
        servicePath = parser.value(QStringLiteral("service"));
        url = parser.value(QStringLiteral("url"));
    } else if (parser.isSet(QStringLiteral("application"))) {
        serviceName = parser.value(QStringLiteral("application"));
        url = parser.value(QStringLiteral("url"));
    } else {
        QStringList positionalArgs = parser.positionalArguments();
        if (positionalArgs.isEmpty()) {
            qCritical() << i18n("No command specified");
            parser.showHelp(1);
        }

        exe = positionalArgs.takeFirst();
        exeArgs = positionalArgs;
    }

    desktop = parser.value(QStringLiteral("desktop")).toInt();
    if (parser.isSet(QStringLiteral("alldesktops"))) {
        desktop = NETWinInfo::OnAllDesktops;
    }
    if (parser.isSet(QStringLiteral("currentdesktop"))) {
        desktop = KWindowSystem::currentDesktop();
    }

    windowtitle = parser.value(QStringLiteral("window"));
    windowclass = parser.value(QStringLiteral("windowclass"));
    if (!windowclass.isEmpty()) {
        windowclass = windowclass.toLower();
    }

    if (windowtitle.isEmpty() && windowclass.isEmpty()) {
        qWarning() << "Omitting both --window and --windowclass arguments is not recommended";
    }

    QString s = parser.value(QStringLiteral("type"));
    if (!s.isEmpty()) {
        s = s.toLower();
        if (s == QLatin1String("desktop")) {
            windowtype = NET::Desktop;
        } else if (s == QLatin1String("dock")) {
            windowtype = NET::Dock;
        } else if (s == QLatin1String("toolbar")) {
            windowtype = NET::Toolbar;
        } else if (s == QLatin1String("menu")) {
            windowtype = NET::Menu;
        } else if (s == QLatin1String("dialog")) {
            windowtype = NET::Dialog;
        } else if (s == QLatin1String("override")) {
            windowtype = NET::Override;
        } else if (s == QLatin1String("topmenu")) {
            windowtype = NET::TopMenu;
        } else {
            windowtype = NET::Normal;
        }
    }

    if (parser.isSet(QStringLiteral("keepabove"))) {
        state |= NET::KeepAbove;
        mask |= NET::KeepAbove;
    } else if (parser.isSet(QStringLiteral("keepbelow"))) {
        state |= NET::KeepBelow;
        mask |= NET::KeepBelow;
    }

    if (parser.isSet(QStringLiteral("skiptaskbar"))) {
        state |= NET::SkipTaskbar;
        mask |= NET::SkipTaskbar;
    }

    if (parser.isSet(QStringLiteral("skippager"))) {
        state |= NET::SkipPager;
        mask |= NET::SkipPager;
    }

    activate = parser.isSet(QStringLiteral("activate"));

    if (parser.isSet(QStringLiteral("maximize"))) {
        state |= NET::Max;
        mask |= NET::Max;
    }
    if (parser.isSet(QStringLiteral("maximize-vertically"))) {
        state |= NET::MaxVert;
        mask |= NET::MaxVert;
    }
    if (parser.isSet(QStringLiteral("maximize-horizontally"))) {
        state |= NET::MaxHoriz;
        mask |= NET::MaxHoriz;
    }

    iconify = parser.isSet(QStringLiteral("iconify"));
    if (parser.isSet(QStringLiteral("fullscreen"))) {
        NETRootInfo i(QX11Info::connection(), NET::Supported);
        if (i.isSupported(NET::FullScreen)) {
            state |= NET::FullScreen;
            mask |= NET::FullScreen;
        } else {
            windowtype = NET::Override;
            fullscreen = true;
        }
    }

    fcntl(XConnectionNumber(QX11Info::display()), F_SETFD, 1);

    KStart start;

    return app.exec();
}
