#ifndef _KSERVICELISTWIDGET_H
#define _KSERVICELISTWIDGET_H

#include <qgroupbox.h>
class TypesListItem;
class QLineEdit;
class QListBox;
class QPushButton;

/**
 * This widget holds a list of services, with 4 buttons to manage it.
 * It's a separate class so that it can be used by both tabs of the
 * module, once for applications and once for services.
 * The "kind" is determined by the argument given to the constructor.
 */
class KServiceListWidget : public QGroupBox
{
  Q_OBJECT
public:
  enum { SERVICELIST_APPLICATIONS, SERVICELIST_SERVICES };
  KServiceListWidget(int kind, QWidget *parent = 0, const char *name = 0);

  void setTypeItem( TypesListItem * item );

signals:
  void changed(bool);

protected slots:
  void promoteService();
  void demoteService();
  void addService();
  void removeService();
  void enableMoveButtons(int index);

protected:
  void updatePreferredServices();

private:
  int m_kind;
  QListBox *servicesLB;
  QPushButton *servUpButton, *servDownButton;
  QPushButton *servNewButton, *servRemoveButton;
  TypesListItem *m_item;
};

#endif
