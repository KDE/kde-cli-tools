#ifndef _FILETYPESVIEW_H
#define _FILETYPESVIEW_H

#include <qptrlist.h>
#include <qmap.h>

#include <kconfig.h>
#include <kcmodule.h>

#include "typeslistitem.h"

class QLabel;
class KListView;
class QListViewItem;
class QListBox;
class QPushButton;
class KIconButton;
class QLineEdit;
class QComboBox;
class FileTypeDetails;
class FileGroupDetails;
class QWidgetStack;

class FileTypesView : public KCModule
{
  Q_OBJECT
public:
  FileTypesView(QWidget *p = 0, const char *name = 0);
  ~FileTypesView();

  void load();
  void save();
  void defaults();

protected slots:
  /** fill in the various graphical elements, set up other stuff. */
  void init();

  void addType();
  void removeType();
  void updateDisplay(QListViewItem *);
  void slotDoubleClicked(QListViewItem *);
  void slotFilter(const QString &patternFilter);
  void setDirty(bool state);

  void slotDatabaseChanged();
  void slotEmbedMajor(const QString &major, bool &embed);

protected:
  void readFileTypes();
  bool sync( QValueList<TypesListItem *>& itemsModified );

private:
  KListView *typesLV;
  QPushButton *m_removeTypeB;

  QWidgetStack * m_widgetStack;
  FileTypeDetails * m_details;
  FileGroupDetails * m_groupDetails;
  QLabel * m_emptyWidget;

  QLineEdit *patternFilterLE;
  QStringList removedList;
  bool m_dirty;
  QMap<QString,TypesListItem*> m_majorMap;
  QPtrList<TypesListItem> m_itemList;

  QValueList<TypesListItem *> m_itemsModified;

  KSharedConfig::Ptr m_konqConfig;
};

#endif
