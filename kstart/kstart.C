/*
 * kstart.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 */

#include <qdir.h>

#include <kstart.moc>
#include "version.h"

#include <fcntl.h>
#include <kprocess.h>
#include <qregexp.h>
#include <klocale.h>
#include <kwin.h>
#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

void execute(const char* cmd){
  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}

KStart::KStart(const char* command_arg,
	       const char* window_arg,
	       int desktop_arg,
	       bool activate_arg,
	       bool maximize_arg,
	       bool iconify_arg,
	       bool sticky_arg,
	       int decoration_arg
	       )
  :QObject(){
    kwinmodule = new KWinModule;
    command = command_arg;
    window = window_arg;
    desktop = desktop_arg;
    activate = activate_arg;
    maximize = maximize_arg;
    iconify = iconify_arg;
    sticky = sticky_arg;
    decoration = decoration_arg;

    // just connect to the initialized() signal, we want to be
    // informed if we recieved all existing windows
    //connect(kwinmodule, SIGNAL(initialized()), SLOT(initialized()));
    //kwinmodule->connectToKWM();
    initialized();
}

void KStart::initialized(){
    // ok, we are initialized. Now connect to window add to get the NEW windows
    connect(kwinmodule, SIGNAL(windowAdd(WId)), SLOT(windowAdd(WId)));
    if (window) {
	KWM::doNotManage(window);
	XSync(qt_xdisplay(), False);
    }
    //finally execute the comand
    execute(command);
}

void KStart::windowAdd(WId w){
    if (window) {
	QString t = KWM::title(w);
	QRegExp r = window;
	if (r.match(t) != -1){
	    applyStyle( w );
	    ::exit(0);
	}
    }
    else {
	// not window specified, just take the first one
	applyStyle( w );
	::exit(0);
    }
}

void KStart::applyStyle(Window w) {
    if (window)
	KWM::prepareForSwallowing(w);
    if (desktop > 0) {
	if (KWM::desktop(w) != desktop)
        {
            debug("moving window to desktop %d",desktop);
	    KWM::moveToDesktop(w, desktop);
        }
    }
    if (maximize) {
	debug("do maximize");
	KWM::doMaximize(w, true);
    }
    if (iconify)
	KWM::setIconify(w, true);
    if (sticky) {
	KWM::setSticky(w, true);
    }
    if (decoration != KWM::normalDecoration) {
	KWM::setDecoration(w, decoration);
    }

    XSync(qt_xdisplay(), False);
    if (window) {
	XMapWindow(qt_xdisplay(), w);
	XSync(qt_xdisplay(), False);
    }
    if (activate)
      KWM::activate(w);
    XSync(qt_xdisplay(), False);
}

// David, 05/03/2000
static KCmdLineOptions options[] =
{
  { "+command", I18N_NOOP("Command to execute."), 0 },
  { "window <regexp>", I18N_NOOP("A regular expression matching the window title.\n"
                  "If you do not specify one, then the very first window\n"
                  "to appear will be taken. Not recommended!"), 0 },
  { "desktop <number>", I18N_NOOP("Desktop where to make the window appear"), 0 },
  { "sticky", I18N_NOOP("Make the window sticky (appears on all desktops)"), 0 },
  { "iconify", I18N_NOOP("Iconify the window"), 0 },
  { "maximize", I18N_NOOP("Maximize the window"), 0 },
  { "decoration <d>", I18N_NOOP("Sets the decoration, 'tiny' or 'none', defaults to normal."), 0 },
  { "activate", I18N_NOOP("Jump to the window even if it is started on a \n"
                          "different virtual desktop"), 0 },
  { "nofocus", I18N_NOOP("The window does not get the focus\n"
                         "(and therefore has no entry in the taskbar)."), 0 },
  { "staysontop", I18N_NOOP("Make the window stay on top of any other window"), 0 },
  { 0, 0, 0}
};

int main( int argc, char *argv[] )
{
  // David, 05/03/2000
  KAboutData aboutData( "kstart", I18N_NOOP("KStart"), KSTART_VERSION,
      I18N_NOOP(""
       "Utility to launch applications with special KDE window properties \n"
       "such as iconified, maximized, a certain virtual desktop, a special decoration\n"
       "or sticky. Furthermore you can exclude the window from getting the focus or\n"
       "force the window manager to keep the window always on top."),
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
  QCString command;
  for(int i=0; i < args->count(); i++)
      command += QCString(args->arg(i)) + " "; 

  int desktop = args->getOption( "desktop" ).toInt();
  QCString window = args->getOption( "window" );
  QCString s = args->getOption( "decoration" );
  int decoration = KWM::normalDecoration;
  if (s == "tiny")
    decoration = KWM::tinyDecoration;
  if (s == "none")
    decoration = KWM::noDecoration;
  int noFocus = 0;
  if ( !args->isSet( "focus" ) ) // opposite to nofocus
     noFocus = KWM::noFocus;
  int staysOnTop = 0;
  if ( args->isSet( "staysontop" ) )
     staysOnTop = KWM::staysOnTop;
  bool activate = args->isSet("activate");
  bool maximize = args->isSet("maximize");
  bool iconify = args->isSet("iconify");
  bool sticky = args->isSet("sticky");

  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);

  args->clear();

  new KStart(command, window, desktop, activate, maximize, iconify, sticky, decoration | noFocus | staysOnTop);

  return app.exec();
}
