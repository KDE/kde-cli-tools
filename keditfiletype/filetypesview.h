#ifndef _FILETYPESVIEW_H
#define _FILETYPESVIEW_H

#include <qlist.h>
#include <qmap.h>

#include <kcmodule.h>

#include "typeslistitem.h"

class QLabel;
class QListView;
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

  bool sync();

  void load();
  void save();
  void defaults();
  QString quickHelp() const;

protected slots:
  /** fill in the various graphical elements, set up other stuff. */
  void init();

  void addType();
  void removeType();
  void updateDisplay(QListViewItem *);
  void slotDoubleClicked(QListViewItem *);
  void slotFilter(const QString &patternFilter);
  void setDirty(bool state);

protected:
    void readFileTypes();

private:
  QListView *typesLV;
  QPushButton *m_removeTypeB;

  QWidgetStack * m_widgetStack;
  FileTypeDetails * m_details;
  FileGroupDetails * m_groupDetails;
  QLabel * m_emptyWidget;

  QLineEdit *patternFilterLE;
  QStringList removedList;
  bool m_dirty;
  QMap<QString,TypesListItem*> m_majorMap;
  QList<TypesListItem> m_itemList;
};

#endif
