#ifndef _FILETYPEDETAILS_H
#define _FILETYPEDETAILS_H

#include <qwidget.h>
#include <qgroupbox.h>
class TypesListItem;
class KIconButton;
class QLineEdit;
class QListBox;
class QPushButton;
class KServiceListWidget;

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
  void changed(bool);

protected slots:
  void updateIcon(QString icon);
  void updateDescription(const QString &desc);
  void addExtension();
  void removeExtension();
  void enableExtButtons(int index);

private:
  TypesListItem * m_item;

  KIconButton *iconButton;
  QListBox *extensionLB;
  QPushButton *addExtButton, *removeExtButton;
  QLineEdit *description;
  KServiceListWidget *serviceListWidget;
};

#endif
