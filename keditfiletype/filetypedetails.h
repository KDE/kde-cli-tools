#ifndef _FILETYPEDETAILS_H
#define _FILETYPEDETAILS_H

#include <qtabwidget.h>
class TypesListItem;
class KIconButton;
class QLineEdit;
class QListBox;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class KServiceListWidget;

/**
 * This widget contains the right part of the file type configuration
 * dialog, that shows the details for a file type.
 * It is implemented as a separate class so that it can be used by
 * the keditfiletype program to show the details of a single mimetype.
 */
class FileTypeDetails : public QTabWidget
{
  Q_OBJECT
public:
  FileTypeDetails(QWidget *parent = 0, const char *name = 0);

  void setTypeItem( TypesListItem * item );

protected:
  void updateRemoveButton();

signals:
  void changed(bool);

protected slots:
  void updateIcon(QString icon);
  void updateDescription(const QString &desc);
  void addExtension();
  void removeExtension();
  void enableExtButtons(int index);
  void slotAutoEmbedClicked(int button);

private:
  TypesListItem * m_item;

  // First tab - General
  KIconButton *iconButton;
  QListBox *extensionLB;
  QPushButton *addExtButton, *removeExtButton;
  QLineEdit *description;
  KServiceListWidget *serviceListWidget;

  // Second tab - Embedding
  QButtonGroup *m_autoEmbed;
  KServiceListWidget *embedServiceListWidget;
  QRadioButton *m_rbGroupSettings;

};

#endif
