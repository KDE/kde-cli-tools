#include <qstring.h>
#include <kservice.h>
#include <kuserprofile.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdesktopfile.h>

#include "typeslistitem.h"

TypesListItem::TypesListItem(QListView *parent, KMimeType::Ptr mimetype)
  : QListViewItem(parent), metaType(true)
{
  init(mimetype);
  setText(0, majorType());
}

TypesListItem::TypesListItem(TypesListItem *parent, KMimeType::Ptr mimetype)
  : QListViewItem(parent), metaType(false)
{
  init(mimetype);
  setText(0, minorType());
}

TypesListItem::~TypesListItem()
{
}

void TypesListItem::init(KMimeType::Ptr mimetype)
{
  m_mimetype = mimetype;

  int index = mimetype->name().find("/");
  if (index != -1) {
    m_major = mimetype->name().left(index);
    m_minor = mimetype->name().right(mimetype->name().length() -
				     (index+1));
  } else {
    m_major = mimetype->name();
    m_minor = "";
  }
  m_comment = mimetype->comment(QString(), false);
  m_icon = mimetype->icon(QString(), false);
  m_patterns = mimetype->patterns();

  KServiceTypeProfile::OfferList offerList = 
    KServiceTypeProfile::offers(mimetype->name());
  QValueListIterator<KServiceOffer> it(offerList.begin());

  for (; it != offerList.end(); ++it) {
    if ((*it).allowAsDefault())
      m_services.append((*it).service()->name());
  }
}

bool TypesListItem::isDirty() const
{
  if ((m_mimetype->name() != name()) && 
      (name() != "application/octet-stream"))
    return true;
  if (m_mimetype->comment(QString(), false) != m_comment)
    return true;
  if (m_mimetype->icon(QString(), false) != m_icon)
    return true;

  if (m_mimetype->patterns() != m_patterns)
    return true;

  KServiceTypeProfile::OfferList offerList = 
    KServiceTypeProfile::offers(m_mimetype->name());
  QValueListIterator<KServiceOffer> it(offerList.begin());
  QStringList oldservices;
  for (; it != offerList.end(); ++it) {
    if ((*it).allowAsDefault())
      oldservices.append((*it).service()->name());
  }

  if (oldservices != m_services)
    return true;

  // nothing seems to have changed, it's not dirty.
  return false;
}

void TypesListItem::sync()
{
  QString loc = name() + ".desktop";
  loc = locateLocal("mime", loc);

  KDesktopFile config(loc, false, "mime");
  config.writeEntry("Type", "MimeType");
  config.writeEntry("MimeType", name());
  config.writeEntry("Icon", m_icon);
  config.writeEntry("Patterns", m_patterns);
  config.writeEntry("Comment", m_comment);

  KSimpleConfig profile("profilerc");
  QStringList::Iterator it(m_services.begin());
  for (int i = m_services.count(); it != m_services.end(); ++it, i--) {
    profile.setGroup(*it);
    profile.writeEntry("ServiceType", name());
    profile.writeEntry("AllowAsDefault", true);
    profile.writeEntry("Preference", i);
    
    KService::Ptr pService = KService::service(*it);
    ASSERT(pService);

    QString serviceLoc = locateLocal("apps", pService->desktopEntryPath());
    KDesktopFile desktop(serviceLoc);
    desktop.writeEntry("Type", pService->type());
    desktop.writeEntry("Icon", pService->icon());
    desktop.writeEntry("Name", *it);
    desktop.writeEntry("Comment", pService->comment());
    desktop.writeEntry("Exec", pService->exec());

    // merge new mimetype
    QStringList serviceList = pService->serviceTypes();
    if (!serviceList.contains(name()))
      serviceList.append(name());
    desktop.writeEntry("MimeType", serviceList, ';');
   }
}
