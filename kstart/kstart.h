/*
 * kstart.h Part of the KDE project.
 *
 * Copyright (C) 1997-2000 Matthias Ettrich <ettrich@kde.org>
 *
 * Port to NETWM by David Faure <faure@kde.org>
 *
 */

#include <qobject.h>

class KWinModule;

class KStart: public QObject {
  Q_OBJECT

public:
  KStart();
  ~KStart(){};

public slots:
  void windowAdded(WId);

private:

  void applyStyle(WId );
};



