/*
 * kstart.C. Part of the KDE project.
 *
 * Copyright (C) 1997-2000 Matthias Ettrich <ettrich@kde.org>
 *
 * First port to NETWM by David Faure <faure@kde.org>
 */

#include "kstart.moc"
#include "version.h"

#include <fcntl.h>
#include <stdlib.h>

#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kprocess.h>
#include <klocale.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>

#include <netwm.h>

pid_t execute(const QString & cmd){
  KShellProcess proc;
  proc << cmd;
  if( proc.start(KShellProcess::DontCare))
      return proc.pid();
  return -1;
}


// some globals

static QCString command = 0;
static QCString window = 0;
static int desktop = 0;
static bool activate = 0;
static bool iconify = 0;
static unsigned long state = 0;
static unsigned long mask = 0;
static NET::WindowType windowtype = NET::Unknown;
static KWinModule* kwinmodule;

KStart::KStart()
    :QObject()
{
    // connect to window add to get the NEW windows
    connect(kwinmodule, SIGNAL(windowAdded(WId)), SLOT(windowAdded(WId)));

    if (window)
	kwinmodule->doNotManage( window );

    // propagate the app startup notification info to the started app
    KStartupInfoId id;
    id.initId( kapp->startupId());
    id.setupStartupEnv();
    
    //finally execute the comand
    pid_t pid = execute(command);
    
    if( pid >= 0 ) {
        KStartupInfoData data;
        data.addPid( pid );
        data.setName( command );
        QCString bin = command; 
        int space = bin.find( ' ' ); // try to get the name of the binary
        if( space != -1 )
            bin = bin.left( space );
        data.setBin( bin.mid( bin.findRev( '/' ) + 1 ));
        KStartupInfo::sendChange( id, data );
    }
    else
        KStartupInfo::sendFinish( id ); // failed to start
}

void KStart::windowAdded(WId w){

    KWin::Info info = KWin::info( w );

    if ( window) {
	QString title = info.name;
	QRegExp r( window );
	if (r.match(title) == -1)
	    return; // no match
    }
    applyStyle( w );
    QApplication::exit();
}


extern Atom qt_wm_state; // defined in qapplication_x11.cpp
static bool wstate_withdrawn( WId winid )
{
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;
    int r = XGetWindowProperty( qt_xdisplay(), winid, qt_wm_state, 0, 2,
				FALSE, AnyPropertyType, &type, &format,
				&length, &after, &data );
    bool withdrawn = TRUE;
    if ( r == Success && data && format == 32 ) {
	Q_UINT32 *wstate = (Q_UINT32*)data;
	withdrawn  = (*wstate == WithdrawnState );
	XFree( (char *)data );
    }
    return withdrawn;
}


void KStart::applyStyle(WId w ) {


    if ( iconify || windowtype != NET::Unknown || desktop >= 1 ) {
	
	XWithdrawWindow(qt_xdisplay(), w, qt_xscreen());
	QApplication::flushX();

	while ( !wstate_withdrawn(w) )
	    ;
    }

    NETWinInfo info( qt_xdisplay(), w, qt_xrootwin(), NET::WMState );

    if (desktop != 0)
	info.setDesktop( desktop );

    if (iconify) {
	XWMHints * hints = XGetWMHints(qt_xdisplay(), w );
	if (hints ) {
	    hints->flags |= StateHint;
	    hints->initial_state = IconicState;
	    XSetWMHints( qt_xdisplay(), w, hints );
	    XFree(hints);
	}
    }

    if ( windowtype != NET::Unknown ) {
	info.setWindowType( windowtype );
    }

    info.setState( state, mask );

    XSync(qt_xdisplay(), False);

    XMapWindow(qt_xdisplay(), w);
    XSync(qt_xdisplay(), False);
    if (activate)
      KWin::setActiveWindow( w );

    QApplication::flushX();
}

// David, 05/03/2000
static KCmdLineOptions options[] =
{
  { "!+command", I18N_NOOP("Command to execute."), 0 },
  // "!" means: all options after command are treated as arguments to the command
  { "window <regexp>", I18N_NOOP("A regular expression matching the window title.\n"
                  "If you do not specify one, then the very first window\n"
                  "to appear will be taken. Not recommended!"), 0 },
  { "desktop <number>", I18N_NOOP("Desktop where to make the window appear. "), 0 },
  { "currentdesktop", I18N_NOOP("Make the window appear on the desktop that was active\nwhen starting the application. "), 0 },
  { "alldesktops", I18N_NOOP("Make the window appear on all desktops"), 0 },
  { "iconify", I18N_NOOP("Iconify the window"), 0 },
  { "maximize", I18N_NOOP("Maximize the window"), 0 },
  { "type <type>", I18N_NOOP("The window type: Normal, Desktop, Dock, Tool, \nMenu, Dialog or Override"), 0 },
  { "activate", I18N_NOOP("Jump to the window even if it is started on a \n"
                          "different virtual desktop"), 0 },
  { "ontop", I18N_NOOP("Make the window always stay on top of any other window"), 0 },
  { "skiptaskbar", I18N_NOOP("The window does not get an entry in the taskbar"), 0 },
  { "skippager", I18N_NOOP("The window does not get an entry on the pager"), 0 },
  { 0, 0, 0}
};

int main( int argc, char *argv[] )
{
  // David, 05/03/2000
  KAboutData aboutData( "kstart", I18N_NOOP("KStart"), KSTART_VERSION,
      I18N_NOOP(""
       "Utility to launch applications with special window properties \n"
       "such as iconified, maximized, a certain virtual desktop, a special decoration\n"
       "and so on." ),
      KAboutData::License_GPL,
       "(C) 1997-2000 Matthias Ettrich (ettrich@kde.org)" );
  aboutData.addAuthor( "Matthias Ettrich", 0, "ettrich@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() == 0 )
      KCmdLineArgs::usage(i18n("No command specified"));

  // Perhaps we should use a konsole-like solution here (shell, list of args...)
  for(int i=0; i < args->count(); i++)
      command += QCString(args->arg(i)) + " ";

  kwinmodule = new KWinModule;

  desktop = args->getOption( "desktop" ).toInt();
  if ( args->isSet ( "alldesktops")  )
      desktop = NETWinInfo::OnAllDesktops;
  if ( args->isSet ( "currentdesktop")  )
      desktop = kwinmodule->currentDesktop();

  window = args->getOption( "window" );

  QCString s = args->getOption( "type" );
  if ( !s.isEmpty() ) {
      s = s.lower();
      if ( s == "desktop" )
	  windowtype = NET::Desktop;
      else if ( s == "dock" )
	  windowtype = NET::Dock;
      else if ( s == "tool" )
	  windowtype = NET::Tool;
      else if ( s == "menu" )
	  windowtype = NET::Menu;
      else if ( s == "dialog" )
	  windowtype = NET::Dialog;
      else if ( s == "override" )
	  windowtype = NET::Override;
      else
	  windowtype = NET::Normal;
  }

  if ( args->isSet( "ontop" ) ) {
      state |= NET::StaysOnTop;
      mask |= NET::StaysOnTop;
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

  iconify = args->isSet("iconify");

  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  args->clear();

  KStart start;
  
  QTimer::singleShot( 120 * 1000, &app, SLOT( quit())); // quit if nothing happens in 2 minutes
  return app.exec();
}
