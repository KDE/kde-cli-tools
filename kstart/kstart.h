/*
 * kstart.h Part of the KDE project.
 *
 * Copyright (C) 1997-2000 Matthias Ettrich <ettrich@kde.org>
 *
 * Port to NETWM by David Faure <faure@kde.org>
 *
 */

#include <qobject.h>
#include <netwm.h>

class KWinModule;

class KStart: public QObject {
  Q_OBJECT

public:
  KStart(const char* command_arg,
	 const char* window_arg,
	 int desktop_arg,
	 bool activate_arg,
	 bool maximize_arg,
	 bool iconify_arg,
	 bool sticky_arg,
	 bool staysontop_arg);
  ~KStart(){};

public slots:

  void windowAdded(WId);

private:

  void applyStyle(Window, NETWinInfo &);

  KWinModule* kwinmodule;
  QString command;
  QString window;
  int desktop;
  bool activate;
  bool maximize;
  bool iconify;
  bool sticky;
  bool staysontop;
};



