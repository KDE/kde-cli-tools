#ifndef _FILETYPESVIEW_H
#define _FILETYPESVIEW_H

#include <kcmodule.h>

class QListView;
class QListViewItem;
class QListBox;
class KIconButton;
class QLineEdit;
class QComboBox;

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
  void addExtension();
  void removeExtension();
  void updateDisplay(QListViewItem *);
  void updateIcon(QString icon);
  void updateDescription(const QString &desc);
  void promoteService();
  void demoteService();
  void enableMoveButtons(int index);
  void enableExtButtons(int index);
  void addService();
  void slotFilter(const QString &patternFilter);

protected:
  void updatePreferredServices();
  void readFileTypes(const QString &patternFilter = QString::null);

private:
  QListView *typesLV;
  KIconButton *iconButton;
  QListBox *extensionLB;
  QLineEdit *description;
  QListBox *servicesLB;
  QPushButton *servUpButton, *servDownButton;
  QPushButton *servNewButton;
  QPushButton *addExtButton, *removeExtButton;
  QLineEdit *patternFilterLE;

  QStringList removedList;
};

#endif
