#include <qstring.h>
#include <qtooltip.h>

#include <kdebug.h>
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
    if (acceptService(*it))
      m_services.append((*it).service()->desktopEntryPath());
  }
}

bool TypesListItem::isDirty() const
{
  if ((m_mimetype->name() != name()) &&
      (name() != "application/octet-stream"))
  {
    kdDebug() << "Mimetype Name Dirty :" << m_mimetype->name() << " name()=" << name() << endl;
    return true;
  }
  if (m_mimetype->comment(QString(), false) != m_comment)
  {
    kdDebug() << "Mimetype Comment Dirty :" << m_mimetype->comment(QString(),false) << " m_comment=" << m_comment << endl;
    return true;
  }
  if (m_mimetype->icon(QString(), false) != m_icon)
  {
    kdDebug() << "Mimetype Icon Dirty :" << m_mimetype->icon(QString(),false) << " m_icon=" << m_icon << endl;
    return true;
  }

  if (m_mimetype->patterns() != m_patterns)
  {
    kdDebug() << "Mimetype Patterns Dirty :" << m_mimetype->patterns().join(";")
              << " m_patterns=" << m_patterns.join(";") << endl;
    return true;
  }

  KServiceTypeProfile::OfferList offerList =
    KServiceTypeProfile::offers(m_mimetype->name());
  QValueListIterator<KServiceOffer> it(offerList.begin());
  QStringList oldservices;
  for (; it != offerList.end(); ++it) {
    if (acceptService(*it))
      oldservices.append((*it).service()->desktopEntryPath());
  }

  if (oldservices != m_services)
  {
    kdDebug() << "Mimetype Services Dirty :" << oldservices.join(";")
              << " m_services=" << m_services.join(";") << endl;
    return true;
  }

  // nothing seems to have changed, it's not dirty.
  return false;
}

void TypesListItem::sync()
{
  QString loc = name() + ".desktop";
  loc = locateLocal("mime", loc);

  KDesktopFile config( loc );

  config.writeEntry("Type", "MimeType");
  config.writeEntry("MimeType", name());
  config.writeEntry("Icon", m_icon);
  config.writeEntry("Patterns", m_patterns, ';');
  config.writeEntry("Comment", m_comment);
  config.writeEntry("Hidden", false);

  KSimpleConfig profile("profilerc");

  // Deleting current contents in profilerc relating to
  // this service type
  //
  QStringList groups = profile.groupList();

  for (QStringList::Iterator it = groups.begin();
       it != groups.end(); it++ )
  {
    profile.setGroup(*it);

    // Entries with Preference <= 0 or AllowAsDefault == false
    // or which are not apps (TODO ! (DF))
    // are not in m_services
    if ( profile.readEntry( "ServiceType" ) == name()
         && profile.readNumEntry( "Preference" ) > 0
         && profile.readBoolEntry( "AllowAsDefault" ) )
    {
      profile.deleteGroup( *it );
    }
  }

  // Save preferred services
  //
  int groupCount = 1;

  QStringList::Iterator it(m_services.begin());
  for (int i = m_services.count(); it != m_services.end(); ++it, i--) {

    KService::Ptr pService = KService::serviceByDesktopPath(*it);
    ASSERT(pService);

    // Find a group header. The headers are just dummy names as far as
    // KUserProfile is concerned, but using the mimetype makes it a
    // bit more structured for "manual" reading
    while ( profile.hasGroup( name() + " - " + QString::number(groupCount) ) )
        groupCount++;

    profile.setGroup( name() + " - " + QString::number(groupCount) );

    profile.writeEntry("ServiceType", name());
    profile.writeEntry("Application", pService->desktopEntryPath());
    profile.writeEntry("AllowAsDefault", true);
    profile.writeEntry("Preference", i);

    QString serviceLoc;

    if ( pService->type() == QString("Service") )
      serviceLoc = locateLocal("services", pService->desktopEntryPath());
    else
      serviceLoc = locateLocal("apps", pService->desktopEntryPath());

    KDesktopFile desktop( serviceLoc );

    desktop.writeEntry("Type", pService->type());
    desktop.writeEntry("Icon", pService->icon());
    desktop.writeEntry("Name", pService->name());
    desktop.writeEntry("Comment", pService->comment());
    desktop.writeEntry("Exec", pService->exec());

    // merge new mimetype
    QStringList serviceList = pService->serviceTypes();

    if (!serviceList.contains(name()))
      serviceList.append(name());

    desktop.writeEntry("MimeType", serviceList, ';');
  }

  // Handle removed services
  //
  KServiceTypeProfile::OfferList offerList =
    KServiceTypeProfile::offers(m_mimetype->name());

  QValueListIterator<KServiceOffer> it_srv(offerList.begin());

  for (; it_srv != offerList.end(); ++it_srv) {

    // Only those were added in init()
    if ( acceptService(*it_srv) ) {

      KService::Ptr pService = (*it_srv).service();

      if ( ! m_services.contains( pService->desktopEntryPath() ) ) {
        // The service was in m_services but has been removed
        // create a new .desktop file without this mimetype

        QStringList serviceTypeList = pService->serviceTypes();

        if ( serviceTypeList.contains( name() ) ) {
          // The mimetype is listed explicitly in the .desktop files, so
          // just remove it and we're done
          QString serviceLoc;

          if ( pService->type() == QString("Service") )
            serviceLoc = locateLocal("services", pService->desktopEntryPath());
          else
            serviceLoc = locateLocal("apps", pService->desktopEntryPath());

          KDesktopFile desktop(serviceLoc);

          serviceTypeList.remove(name());
          desktop.writeEntry("MimeType", serviceTypeList, ';');

          desktop.writeEntry("Type", pService->type());
          desktop.writeEntry("Icon", pService->icon());
          desktop.writeEntry("Name", pService->name());
          desktop.writeEntry("Comment", pService->comment());
          desktop.writeEntry("Exec", pService->exec());

        }
        else {
          // The mimetype is not listed explicitly so it can't
          // be removed. Preference = 0 handles this.

          // Find a group header. The headers are just dummy names as far as
          // KUserProfile is concerned, but using the mimetype makes it a
          // bit more structured for "manual" reading
          while ( profile.hasGroup(
                  name() + " - " + QString::number(groupCount) ) )
              groupCount++;

          profile.setGroup( name() + " - " + QString::number(groupCount) );

          profile.writeEntry("Application", pService->desktopEntryPath());
          profile.writeEntry("ServiceType", name());
          profile.writeEntry("AllowAsDefault", true);
          profile.writeEntry("Preference", 0);
        }
      }
    }
  }

}

bool TypesListItem::acceptService( const KServiceOffer & offer ) const
{
  return (offer.allowAsDefault()
          && offer.service()->type() == QString::fromLatin1("Application"));
}
