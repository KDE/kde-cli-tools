#ifndef _TYPESLISTITEM_H
#define _TYPESLISTITEM_H

#include <qlistview.h>

#include <kmimetype.h>

class TypesListItem : public QListViewItem
{
public:
  TypesListItem(QListView *parent, KMimeType::Ptr mimetype);
  TypesListItem(TypesListItem *parent, KMimeType::Ptr mimetype);
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
  QStringList defaultServices() const { return m_services; }
  void setDefaultServices(const QStringList &dsl) { m_services = dsl; }
  bool isDirty() const;
  void sync();

private:
  KMimeType::Ptr m_mimetype;
  void init(KMimeType::Ptr mimetype);

  bool metaType;
  QString m_major, m_minor, m_comment, m_icon;
  QStringList m_patterns;
  QStringList m_services;
};

#endif
