#ifndef _TYPESLISTITEM_H
#define _TYPESLISTITEM_H

#include <qlistview.h>

#include <kmimetype.h>
#include <kuserprofile.h>

class TypesListItem : public QListViewItem
{
public:
  /**
   * Create a filetype group
   */
  TypesListItem(QListView *parent, KMimeType::Ptr mimetype);
  /**
   * Create a filetype item
   */
  TypesListItem(TypesListItem *parent, KMimeType::Ptr mimetype, bool newItem=false);
  ~TypesListItem();

  QString name() const { return m_major + "/" + m_minor; }
  QString majorType() const { return m_major; }
  QString minorType() const { return m_minor; }
  void setMinor(QString m) { m_minor = m; }
  QString comment() const { return m_comment; }
  void setComment(QString c) { m_comment = c; }
  bool isMeta() const { return metaType; }
  QString icon() const { return m_icon; }
  void setIcon(QString i) { m_icon = i; }
  QStringList patterns() const { return m_patterns; }
  void setPatterns(const QStringList &p) { m_patterns = p; }
  QStringList appServices() const { return m_appServices; }
  void setAppServices(const QStringList &dsl) { m_appServices = dsl; }
  QStringList embedServices() const { return m_embedServices; }
  void setEmbedServices(const QStringList &dsl) { m_embedServices = dsl; }
  int autoEmbed() const { return m_autoEmbed; }
  void setAutoEmbed( int a ) { m_autoEmbed = a; }

  bool isDirty() const;
  void sync();

private:
  void saveServices( KSimpleConfig & profile, QStringList services );
  KMimeType::Ptr m_mimetype;
  void init(KMimeType::Ptr mimetype);

  bool metaType;
  bool m_bNewItem;
  QString m_major, m_minor, m_comment, m_icon;
  QStringList m_patterns;
  QStringList m_appServices;
  QStringList m_embedServices;
  int m_autoEmbed; // 0,1,2
  int groupCount; // shared between saveServices and sync
};

#endif
