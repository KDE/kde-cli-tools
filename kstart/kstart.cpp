/*
 * kstart.C. Part of the KDE project.
 *
 * Copyright (C) 1997-2000 Matthias Ettrich <ettrich@kde.org>
 *
 * First port to NETWM by David Faure <faure@kde.org>
 * Send to system tray by Richard Moore <rich@kde.org>

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ktoolinvocation.h>
#include "kstart.moc"

#include <fcntl.h>
#include <stdlib.h>

#include <QRegExp>
#include <QTimer>
#include <QDesktopWidget>
#include <QApplication>

#include <kdebug.h>
#include <kprocess.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kwindowsystem.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>
#include <kxmessages.h>
#include <kdeversion.h>

#include <netwm.h>
#include <QX11Info>


// some globals

static KProcess* proc = 0;
static QString exe;
static QString url;
static QString windowtitle;
static QString windowclass;
static int desktop = 0;
static bool activate = false;
static bool iconify = false;
static bool fullscreen = false;
static unsigned long state = 0;
static unsigned long mask = 0;
static NET::WindowType windowtype = NET::Unknown;

KStart::KStart()
    :QObject()
{
    NETRootInfo i( QX11Info::display(), NET::Supported );
    bool useRule = i.isSupported( NET::WM2KDETemporaryRules );

    if( useRule )
        sendRule();
    else {
        // connect to window add to get the NEW windows
        connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), SLOT(windowAdded(WId)));
    }
    // propagate the app startup notification info to the started app
    // We are not using KApplication, so the env remained set
    KStartupInfoId id = KStartupInfo::currentStartupIdEnv();

    //finally execute the comand
    if (proc) {
        if( int pid = proc->startDetached() ) {
            KStartupInfoData data;
            data.addPid( pid );
            data.setName( exe );
            data.setBin( exe.mid( exe.lastIndexOf( '/' ) + 1 ));
            KStartupInfo::sendChange( id, data );
        }
        else
            KStartupInfo::sendFinish( id ); // failed to start
    } else {
        QString error;
        QString dbusService;
        int pid;
        if (KToolInvocation::startServiceByDesktopPath(exe, url, &error, &dbusService, &pid) == 0) {
            printf("%s\n", qPrintable(dbusService));
        } else {
            kError() << error;
        }
    }

  QTimer::singleShot( useRule ? 0 : 120 * 1000, qApp, SLOT( quit()));
}

void KStart::sendRule() {
    KXMessages msg;
    QString message;
    if( !windowtitle.isEmpty() )
        message += "title=" + windowtitle + "\ntitlematch=3\n"; // 3 = regexp match
    if( !windowclass.isEmpty() )
        message += "wmclass=" + windowclass + "\nwmclassmatch=1\n" // 1 = exact match
            + "wmclasscomplete="
            // if windowclass contains a space (i.e. 2 words, use whole WM_CLASS)
            + ( windowclass.contains( ' ' ) ? "true" : "false" ) + '\n';
    if( (!windowtitle.isEmpty()) || (!windowclass.isEmpty()) ) {
        // always ignore these window types
        message += "types=" + QString().setNum( -1U &
            ~( NET::TopMenuMask | NET::ToolbarMask | NET::DesktopMask | NET::SplashMask | NET::MenuMask )) + '\n';
    } else {
        // accept only "normal" windows
        message += "types=" + QString().setNum( NET::NormalMask | NET::DialogMask ) + '\n';
    }
    if ( ( desktop > 0 && desktop <= KWindowSystem::numberOfDesktops() )
         || desktop == NETWinInfo::OnAllDesktops ) {
	message += "desktop=" + QString().setNum( desktop ) + "\ndesktoprule=3\n";
    }
    if (activate)
        message += "fsplevel=0\nfsplevelrule=2\n";
    if (iconify)
        message += "minimize=true\nminimizerule=3\n";
    if ( windowtype != NET::Unknown ) {
        message += "type=" + QString().setNum( windowtype ) + "\ntyperule=2";
    }
    if ( state ) {
        if( state & NET::KeepAbove )
            message += "above=true\naboverule=3\n";
        if( state & NET::KeepBelow )
            message += "below=true\nbelowrule=3\n";
        if( state & NET::SkipTaskbar )
            message += "skiptaskbar=true\nskiptaskbarrule=3\n";
        if( state & NET::SkipPager )
            message += "skippager=true\nskippagerrule=3\n";
        if( state & NET::MaxVert )
            message += "maximizevert=true\nmaximizevertrule=3\n";
        if( state & NET::MaxHoriz )
            message += "maximizehoriz=true\nmaximizehorizrule=3\n";
        if( state & NET::FullScreen )
            message += "fullscreen=true\nfullscreenrule=3\n";
    }

    msg.broadcastMessage( "_KDE_NET_WM_TEMPORARY_RULES", message, -1, false );
    qApp->flush();
}

const int SUPPORTED_WINDOW_TYPES_MASK = NET::NormalMask | NET::DesktopMask | NET::DockMask
    | NET::ToolbarMask | NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::TopMenuMask
    | NET::UtilityMask | NET::SplashMask;

void KStart::windowAdded(WId w){

    KWindowInfo info = KWindowSystem::windowInfo( w, NET::WMWindowType | NET::WMName );

    // always ignore these window types
    if( info.windowType( SUPPORTED_WINDOW_TYPES_MASK ) == NET::TopMenu
        || info.windowType( SUPPORTED_WINDOW_TYPES_MASK ) == NET::Toolbar
        || info.windowType( SUPPORTED_WINDOW_TYPES_MASK ) == NET::Desktop )
        return;

    if ( !windowtitle.isEmpty() ) {
	QString title = info.name().toLower();
	QRegExp r( windowtitle.toLower());
	if ( !r.exactMatch(title) )
	    return; // no match
    }
    if ( !windowclass.isEmpty() ) {
#ifdef __GNUC__
#warning "Porting required"
#endif
#if 0
        XClassHint hint;
        if( !XGetClassHint( QX11Info::display(), w, &hint ))
            return;
        Q3CString cls = windowclass.contains( ' ' )
            ? Q3CString( hint.res_name ) + ' ' + hint.res_class : Q3CString( hint.res_class );
        cls = cls.toLower();
        XFree( hint.res_name );
	XFree( hint.res_class );
        if( cls != windowclass )
            return;
#endif
    }
    if( windowtitle.isEmpty() && windowclass.isEmpty() ) {
        // accept only "normal" windows
        if( info.windowType( SUPPORTED_WINDOW_TYPES_MASK ) != NET::Unknown
            && info.windowType( SUPPORTED_WINDOW_TYPES_MASK ) != NET::Normal
            && info.windowType( SUPPORTED_WINDOW_TYPES_MASK ) != NET::Dialog )
            return;
    }
    applyStyle( w );
    QApplication::exit();
}


//extern Atom qt_wm_state; // defined in qapplication_x11.cpp
static bool wstate_withdrawn( WId winid )
{
    Q_UNUSED(winid);

#ifdef __GNUC__
#warning "Porting required."
#endif
//Porting info: The Qt4 equivalent for qt_wm_state is qt_x11Data->atoms[QX11Data::WM_STATE]
//which can be accessed via the macro ATOM(WM_STATE). Unfortunately, neither of these seem
//to be exported out of the Qt environment. This value may have to be acquired from somewhere else.
/*
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    int r = XGetWindowProperty( QX11Info::display(), winid, qt_wm_state, 0, 2,
				false, AnyPropertyType, &type, &format,
				&length, &after, &data );
    bool withdrawn = true;
    if ( r == Success && data && format == 32 ) {
	quint32 *wstate = (quint32*)data;
	withdrawn  = (*wstate == WithdrawnState );
	XFree( (char *)data );
    }
    return withdrawn;
*/
	return true;
}


void KStart::applyStyle(WId w ) {

    if ( state || iconify || windowtype != NET::Unknown || desktop >= 1 ) {

	QX11Info info;
	XWithdrawWindow(QX11Info::display(), w, info.screen());
	QApplication::flush();

	while ( !wstate_withdrawn(w) )
	    ;
    }

    NETWinInfo info( QX11Info::display(), w, QX11Info::appRootWindow(), NET::WMState );

    if ( ( desktop > 0 && desktop <= KWindowSystem::numberOfDesktops() )
         || desktop == NETWinInfo::OnAllDesktops )
	info.setDesktop( desktop );

    if (iconify) {
	XWMHints * hints = XGetWMHints(QX11Info::display(), w );
	if (hints ) {
	    hints->flags |= StateHint;
	    hints->initial_state = IconicState;
	    XSetWMHints( QX11Info::display(), w, hints );
	    XFree(hints);
	}
    }

    if ( windowtype != NET::Unknown ) {
	info.setWindowType( windowtype );
    }

    if ( state )
	info.setState( state, mask );

    if ( fullscreen ) {
	QRect r = QApplication::desktop()->screenGeometry();
	XMoveResizeWindow( QX11Info::display(), w, r.x(), r.y(), r.width(), r.height() );
    }


    XSync(QX11Info::display(), False);

    XMapWindow(QX11Info::display(), w );
    XSync(QX11Info::display(), False);

    if (activate)
      KWindowSystem::forceActiveWindow( w );

    QApplication::flush();
}

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "kstart", 0, ki18n("KStart"), KDE_VERSION_STRING,
      ki18n(""
       "Utility to launch applications with special window properties \n"
       "such as iconified, maximized, a certain virtual desktop, a special decoration\n"
       "and so on." ),
      KAboutData::License_GPL,
       ki18n("(C) 1997-2000 Matthias Ettrich (ettrich@kde.org)") );

  aboutData.addAuthor( ki18n("Matthias Ettrich"), KLocalizedString(), "ettrich@kde.org" );
  aboutData.addAuthor( ki18n("David Faure"), KLocalizedString(), "faure@kde.org" );
  aboutData.addAuthor( ki18n("Richard J. Moore"), KLocalizedString(), "rich@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );


  KCmdLineOptions options;

  options.add("!+command", ki18n("Command to execute"));
  options.add("service <desktopfile>", ki18n("Alternative to <command>: desktop file to start. D-Bus service will be printed to stdout"));
  options.add("url <url>", ki18n("Optional URL to pass <desktopfile>, when using --service"));
  // "!" means: all options after command are treated as arguments to the command
  options.add("window <regexp>", ki18n("A regular expression matching the window title"));
  options.add("windowclass <class>", ki18n("A string matching the window class (WM_CLASS property)\n"
                  "The window class can be found out by running\n"
                  "'xprop | grep WM_CLASS' and clicking on a window\n"
                  "(use either both parts separated by a space or only the right part).\n"
                  "NOTE: If you specify neither window title nor window class,\n"
                  "then the very first window to appear will be taken;\n"
                  "omitting both options is NOT recommended."));
  options.add("desktop <number>", ki18n("Desktop on which to make the window appear"));
  options.add("currentdesktop", ki18n("Make the window appear on the desktop that was active\nwhen starting the application"));
  options.add("alldesktops", ki18n("Make the window appear on all desktops"));
  options.add("iconify", ki18n("Iconify the window"));
  options.add("maximize", ki18n("Maximize the window"));
  options.add("maximize-vertically", ki18n("Maximize the window vertically"));
  options.add("maximize-horizontally", ki18n("Maximize the window horizontally"));
  options.add("fullscreen", ki18n("Show window fullscreen"));
  options.add("type <type>", ki18n("The window type: Normal, Desktop, Dock, Toolbar, \nMenu, Dialog, TopMenu or Override"));
  options.add("activate", ki18n("Jump to the window even if it is started on a \n"
                         "different virtual desktop"));
  options.add("ontop");
  options.add("keepabove", ki18n("Try to keep the window above other windows"));
  options.add("onbottom");
  options.add("keepbelow", ki18n("Try to keep the window below other windows"));
  options.add("skiptaskbar", ki18n("The window does not get an entry in the taskbar"));
  options.add("skippager", ki18n("The window does not get an entry on the pager"));
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KComponentData componentData( &aboutData );
  QApplication app( KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if (args->isSet("service")) {
      exe = args->getOption("service");
      url = args->getOption("url");
  } else {
      if ( args->count() == 0 )
          KCmdLineArgs::usageError(i18n("No command specified"));

      exe = args->arg(0);
      proc = new KProcess;
      for(int i=0; i < args->count(); i++)
          (*proc) << args->arg(i);
  }

  desktop = args->getOption( "desktop" ).toInt();
  if ( args->isSet ( "alldesktops")  )
      desktop = NETWinInfo::OnAllDesktops;
  if ( args->isSet ( "currentdesktop")  )
      desktop = KWindowSystem::currentDesktop();

  windowtitle = args->getOption( "window" );
  windowclass = args->getOption( "windowclass" );
  if( !windowclass.isEmpty() )
      windowclass = windowclass.toLower();

  if( windowtitle.isEmpty() && windowclass.isEmpty())
      kWarning() << "Omitting both --window and --windowclass arguments is not recommended" ;

  QString s = args->getOption( "type" );
  if ( !s.isEmpty() ) {
      s = s.toLower();
      if ( s == "desktop" )
	  windowtype = NET::Desktop;
      else if ( s == "dock" )
	  windowtype = NET::Dock;
      else if ( s == "toolbar" )
	  windowtype = NET::Toolbar;
      else if ( s == "menu" )
	  windowtype = NET::Menu;
      else if ( s == "dialog" )
	  windowtype = NET::Dialog;
      else if ( s == "override" )
	  windowtype = NET::Override;
      else if ( s == "topmenu" )
	  windowtype = NET::TopMenu;
      else
	  windowtype = NET::Normal;
  }

  if ( args->isSet( "keepabove" ) ) {
      state |= NET::KeepAbove;
      mask |= NET::KeepAbove;
  } else if ( args->isSet( "keepbelow" ) ) {
      state |= NET::KeepBelow;
      mask |= NET::KeepBelow;
  }

  if ( args->isSet( "skiptaskbar" ) ) {
      state |= NET::SkipTaskbar;
      mask |= NET::SkipTaskbar;
  }

  if ( args->isSet( "skippager" ) ) {
      state |= NET::SkipPager;
      mask |= NET::SkipPager;
  }

  activate = args->isSet("activate");

  if ( args->isSet("maximize") ) {
      state |= NET::Max;
      mask |= NET::Max;
  }
  if ( args->isSet("maximize-vertically") ) {
      state |= NET::MaxVert;
      mask |= NET::MaxVert;
  }
  if ( args->isSet("maximize-horizontally") ) {
      state |= NET::MaxHoriz;
      mask |= NET::MaxHoriz;
  }

  iconify = args->isSet("iconify");
  if ( args->isSet("fullscreen") ) {
      NETRootInfo i( QX11Info::display(), NET::Supported );
      if( i.isSupported( NET::FullScreen )) {
          state |= NET::FullScreen;
          mask |= NET::FullScreen;
      } else {
          windowtype = NET::Override;
          fullscreen = true;
      }
  }

  fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, 1);
  args->clear();

  KStart start;

  return app.exec();
}
