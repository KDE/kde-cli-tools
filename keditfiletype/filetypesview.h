#ifndef _FILETYPESVIEW_H
#define _FILETYPESVIEW_H

#include <kcmodule.h>

class QListView;
class QListViewItem;
class QListBox;
class KIconButton;
class QLineEdit;
class QComboBox;
class FileTypeDetails;

class FileTypesView : public KCModule
{
  Q_OBJECT
public:
  FileTypesView(QWidget *p = 0, const char *name = 0);
  ~FileTypesView();

  /** fill in the various graphical elements, set up other stuff. */
  void init();
  bool sync();

  void load();
  void save();
  void defaults();
  QString quickHelp();

protected slots:
  void addType();
  void removeType();
  void updateDisplay(QListViewItem *);
  void slotFilter(const QString &patternFilter);

protected:
  void readFileTypes(const QString &patternFilter = QString::null);

private:
  void setDirty(bool state);

  QListView *typesLV;

  FileTypeDetails * m_details;

  QLineEdit *patternFilterLE;
  QStringList removedList;
  bool m_dirty;
};

#endif
