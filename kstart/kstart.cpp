/*
 * kstart.C. Part of the KDE project.
 *
 * Copyright (C) 1997-2000 Matthias Ettrich <ettrich@kde.org>
 *
 * First port to NETWM by David Faure <faure@kde.org>
 * Send to system tray by Richard Moore <rich@kde.org>
 */

#include "kstart.moc"
#include "version.h"

#include <fcntl.h>
#include <stdlib.h>

#include <QRegExp>
#include <QTimer>
#include <QDesktopWidget>
#include <qapplication.h>

#include <kdebug.h>
#include <k3process.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kwm.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>
#include <kxmessages.h>

#include <netwm.h>
#include <QX11Info>


// some globals

static K3Process* proc = 0;
static QString windowtitle = 0;
static QString windowclass = 0;
static int desktop = 0;
static bool activate = false;
static bool iconify = false;
static bool toSysTray = false;
static bool fullscreen = false;
static unsigned long state = 0;
static unsigned long mask = 0;
static NET::WindowType windowtype = NET::Unknown;

KStart::KStart()
    :QObject()
{
    NETRootInfo i( QX11Info::display(), NET::Supported );
    bool useRule = !toSysTray && i.isSupported( NET::WM2KDETemporaryRules );

    if( useRule )
        sendRule();
    else {
        // connect to window add to get the NEW windows
        connect(KWM::self(), SIGNAL(windowAdded(WId)), SLOT(windowAdded(WId)));
        if (!windowtitle.isEmpty())
    	    KWM::doNotManage( windowtitle );
    }
    // propagate the app startup notification info to the started app
    // We are not using KApplication, so the env remained set
    KStartupInfoId id = KStartupInfo::currentStartupIdEnv();

    //finally execute the comand
    if( proc->start(K3Process::DontCare) ) {
        KStartupInfoData data;
        data.addPid( proc->pid() );
        QString bin = proc->args().first();
        data.setName( bin );
        data.setBin( bin.mid( bin.lastIndexOf( '/' ) + 1 ));
        KStartupInfo::sendChange( id, data );
    }
    else
        KStartupInfo::sendFinish( id ); // failed to start

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
    if ( ( desktop > 0 && desktop <= KWM::numberOfDesktops() )
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

    KWindowInfo info = KWM::windowInfo( w, NET::WMWindowType | NET::WMName );

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

    if ( toSysTray || state || iconify || windowtype != NET::Unknown || desktop >= 1 ) {

	QX11Info info;
	XWithdrawWindow(QX11Info::display(), w, info.screen());
	QApplication::flush();

	while ( !wstate_withdrawn(w) )
	    ;
    }

    NETWinInfo info( QX11Info::display(), w, QX11Info::appRootWindow(), NET::WMState );

    if ( ( desktop > 0 && desktop <= KWM::numberOfDesktops() )
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

    if ( toSysTray ) {
	QApplication::beep();
//	KWM::setSystemTrayWindowFor( w, QX11Info::appRootWindow() );
    }

    if ( fullscreen ) {
	QRect r = QApplication::desktop()->screenGeometry();
	XMoveResizeWindow( QX11Info::display(), w, r.x(), r.y(), r.width(), r.height() );
    }


    XSync(QX11Info::display(), False);

    XMapWindow(QX11Info::display(), w );
    XSync(QX11Info::display(), False);

    if (activate)
      KWM::forceActiveWindow( w );

    QApplication::flush();
}

static KCmdLineOptions options[] =
{
  { "!+command", I18N_NOOP("Command to execute"), 0 },
  // "!" means: all options after command are treated as arguments to the command
  { "window <regexp>", I18N_NOOP("A regular expression matching the window title"), 0 },
  { "windowclass <class>", I18N_NOOP("A string matching the window class (WM_CLASS property)\n"
                  "The window class can be found out by running\n"
                  "'xprop | grep WM_CLASS' and clicking on a window\n"
                  "(use either both parts separated by a space or only the right part).\n"
                  "NOTE: If you specify neither window title nor window class,\n"
                  "then the very first window to appear will be taken;\n"
                  "omitting both options is NOT recommended."), 0 },
  { "desktop <number>", I18N_NOOP("Desktop on which to make the window appear"), 0 },
  { "currentdesktop", I18N_NOOP("Make the window appear on the desktop that was active\nwhen starting the application"), 0 },
  { "alldesktops", I18N_NOOP("Make the window appear on all desktops"), 0 },
  { "iconify", I18N_NOOP("Iconify the window"), 0 },
  { "maximize", I18N_NOOP("Maximize the window"), 0 },
  { "maximize-vertically", I18N_NOOP("Maximize the window vertically"), 0 },
  { "maximize-horizontally", I18N_NOOP("Maximize the window horizontally"), 0 },
  { "fullscreen", I18N_NOOP("Show window fullscreen"), 0 },
  { "type <type>", I18N_NOOP("The window type: Normal, Desktop, Dock, Toolbar, \nMenu, Dialog, TopMenu or Override"), 0 },
  { "activate", I18N_NOOP("Jump to the window even if it is started on a \n"
                          "different virtual desktop"), 0 },
  { "ontop", 0, 0 },
  { "keepabove", I18N_NOOP("Try to keep the window above other windows"), 0 },
  { "onbottom", 0, 0 },
  { "keepbelow", I18N_NOOP("Try to keep the window below other windows"), 0 },
  { "skiptaskbar", I18N_NOOP("The window does not get an entry in the taskbar"), 0 },
  { "skippager", I18N_NOOP("The window does not get an entry on the pager"), 0 },
  { "tosystray", I18N_NOOP("The window is sent to the system tray in Kicker"), 0 },
  KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "kstart", I18N_NOOP("KStart"), KSTART_VERSION,
      I18N_NOOP(""
       "Utility to launch applications with special window properties \n"
       "such as iconified, maximized, a certain virtual desktop, a special decoration\n"
       "and so on." ),
      KAboutData::License_GPL,
       "(C) 1997-2000 Matthias Ettrich (ettrich@kde.org)" );

  aboutData.addAuthor( "Matthias Ettrich", 0, "ettrich@kde.org" );
  aboutData.addAuthor( "David Faure", 0, "faure@kde.org" );
  aboutData.addAuthor( "Richard J. Moore", 0, "rich@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KComponentData componentData( &aboutData );
  QApplication app( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() == 0 )
      KCmdLineArgs::usage(i18n("No command specified"));

  proc = new K3Process;
  for(int i=0; i < args->count(); i++)
    (*proc) << args->arg(i);

  desktop = args->getOption( "desktop" ).toInt();
  if ( args->isSet ( "alldesktops")  )
      desktop = NETWinInfo::OnAllDesktops;
  if ( args->isSet ( "currentdesktop")  )
      desktop = KWM::currentDesktop();

  windowtitle = args->getOption( "window" );
  windowclass = args->getOption( "windowclass" );
  if( !windowclass.isEmpty() )
      windowclass = windowclass.toLower();

  if( windowtitle.isEmpty() && windowclass.isEmpty())
      kWarning() << "Omitting both --window and --windowclass arguments is not recommended" << endl;

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
  toSysTray = args->isSet("tosystray");
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
