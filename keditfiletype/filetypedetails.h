#ifndef _FILETYPEDETAILS_H
#define _FILETYPEDETAILS_H

#include <qwidget.h>
class TypesListItem;
class KIconButton;
class QLineEdit;
class QListBox;
class QPushButton;

/**
 * This widget contains all the right part of the main kcmfiletypes
 * dialog. It is implemented as a separate class so that it can be
 * used by the keditfiletype program to show the details of a single
 * mimetype.
 */
class FileTypeDetails : public QWidget
{
  Q_OBJECT
public:
  FileTypeDetails(QWidget *parent = 0, const char *name = 0);

  void setTypeItem( TypesListItem * item );

signals:
  void dirty();

protected slots:
  void updateIcon(QString icon);
  void updateDescription(const QString &desc);
  void addExtension();
  void removeExtension();
  void promoteService();
  void demoteService();
  void addService();
  void removeService();
  void enableMoveButtons(int index);
  void enableExtButtons(int index);

protected:
  void updatePreferredServices();
private:

  TypesListItem * m_item;

  KIconButton *iconButton;
  QListBox *extensionLB;
  QLineEdit *description;
  QListBox *servicesLB;
  QPushButton *servUpButton, *servDownButton;
  QPushButton *servNewButton, *servRemoveButton;
  QPushButton *addExtButton, *removeExtButton;
};

#endif
