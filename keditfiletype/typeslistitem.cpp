#include <qstring.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <kservice.h>
#include <kuserprofile.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kdesktopfile.h>

#include "typeslistitem.h"

TypesListItem::TypesListItem(QListView *parent, const QString & major)
  : QListViewItem(parent), metaType(true), m_bNewItem(false)
{
  initMeta(major);
  setText(0, majorType());
}

TypesListItem::TypesListItem(TypesListItem *parent, KMimeType::Ptr mimetype, bool newItem)
  : QListViewItem(parent), metaType(false), m_bNewItem(newItem)
{
  init(mimetype);
  setText(0, minorType());
}

TypesListItem::TypesListItem(QListView *parent, KMimeType::Ptr mimetype)
  : QListViewItem(parent), metaType(false), m_bNewItem(false)
{
  init(mimetype);
  setText(0, majorType());
}

TypesListItem::~TypesListItem()
{
}

void TypesListItem::initMeta( const QString & major )
{
  m_mimetype = 0L;
  m_major = major;
  KConfig config("konquerorrc", true);
  config.setGroup("FMSettings");
  m_autoEmbed = config.readBoolEntry( QString::fromLatin1("embed-")+m_major, true ) ? 0 : 1;
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
    if ((*it).service()->type() == "Application") {
      if ((*it).allowAsDefault())
        m_appServices.append((*it).service()->desktopEntryPath());
    }
    else {
      m_embedServices.append((*it).service()->desktopEntryPath());
    }
  }

  QVariant v = mimetype->property( "X-KDE-AutoEmbed" );
  m_autoEmbed = v.isValid() ? (v.toBool() ? 0 : 1) : 2;
}

bool TypesListItem::isDirty() const
{
  if ( m_bNewItem )
  {
    kdDebug() << "New item, need to save it" << endl;
    return true;
  }

  if ( !isMeta() )
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

    QStringList oldAppServices;
    QStringList oldEmbedServices;
    QValueListIterator<KServiceOffer> it(offerList.begin());
    for (; it != offerList.end(); ++it) {
      if ((*it).service()->type() == "Application") {
        if ((*it).allowAsDefault())
          oldAppServices.append((*it).service()->desktopEntryPath());
      }
      else {
        oldEmbedServices.append((*it).service()->desktopEntryPath());
      }
    }

    if (oldAppServices != m_appServices)
    {
      kdDebug() << "App Services Dirty :" << oldAppServices.join(";")
                << " m_appServices=" << m_appServices.join(";") << endl;
      return true;
    }
    if (oldEmbedServices != m_embedServices)
    {
      kdDebug() << "Embed Services Dirty :" << oldEmbedServices.join(";")
                << " m_embedServices=" << m_embedServices.join(";") << endl;
      return true;
    }

    QVariant v = m_mimetype->property( "X-KDE-AutoEmbed" );
    int oldAutoEmbed = v.isValid() ? (v.toBool() ? 0 : 1) : 2;
    if ( oldAutoEmbed != m_autoEmbed )
      return true;

  } else {

    KConfig config("konquerorrc", true);
    config.setGroup("FMSettings");
    int oldAutoEmbed = config.readBoolEntry( QString::fromLatin1("embed-")+m_major, true ) ? 0 : 1;
    if ( m_autoEmbed != oldAutoEmbed )
      return true;
  }

  // nothing seems to have changed, it's not dirty.
  return false;
}

void TypesListItem::sync()
{
  if ( isMeta() )
  {
    KConfig config("konquerorrc");
    config.setGroup("FMSettings");
    config.writeEntry( QString::fromLatin1("embed-")+m_major, m_autoEmbed == 0 );
    return;
  }
  QString loc = name() + ".desktop";
  loc = locateLocal("mime", loc);

  KSimpleConfig config( loc );
  config.setDesktopGroup();

  config.writeEntry("Type", "MimeType");
  config.writeEntry("MimeType", name());
  config.writeEntry("Icon", m_icon);
  config.writeEntry("Patterns", m_patterns, ';');
  config.writeEntry("Comment", m_comment);
  config.writeEntry("Hidden", false);

  if ( m_autoEmbed == 2 )
    config.deleteEntry( QString::fromLatin1("X-KDE-AutoEmbed"), false );
  else
    config.writeEntry( QString::fromLatin1("X-KDE-AutoEmbed"), m_autoEmbed == 0 );

  m_bNewItem = false;

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

  groupCount = 1;

  saveServices( profile, m_appServices );
  saveServices( profile, m_embedServices );

  // Handle removed services
  // Note: we currently do that for applications only. Embedding services can't be removed.

  KServiceTypeProfile::OfferList offerList =
    KServiceTypeProfile::offers(m_mimetype->name());

  QValueListIterator<KServiceOffer> it_srv(offerList.begin());

  for (; it_srv != offerList.end(); ++it_srv) {

    // Only those were added in init()
    if ((*it_srv).service()->type() == "Application" &&
        (*it_srv).allowAsDefault() ) {

      KService::Ptr pService = (*it_srv).service();

      if ( ! m_appServices.contains( pService->desktopEntryPath() ) ) {
        // The service was in m_appServices but has been removed
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

void TypesListItem::saveServices( KSimpleConfig & profile, QStringList services )
{
  QStringList::Iterator it(services.begin());
  for (int i = services.count(); it != services.end(); ++it, i--) {

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
    QStringList serviceTypeList = pService->serviceTypes();

    if (!serviceTypeList.contains(name()))
      serviceTypeList.append(name());

    desktop.writeEntry("MimeType", serviceTypeList, ';');
    desktop.writeEntry("ServiceTypes", "");
  }
}

