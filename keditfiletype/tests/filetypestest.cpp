/* This file is part of the KDE project
    Copyright 2007 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License or ( at
   your option ) version 3 or, at the discretion of KDE e.V. ( which shall
   act as a proxy as in section 14 of the GPLv3 ), any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
   Boston, MA 02110-1301, USA.
*/

#include <kservice.h>

#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <ksycoca.h>
#include <kcoreaddons_version.h>

// Qt
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QTest>
#include <QSignalSpy>
#include <QLoggingCategory>

#include <mimetypedata.h>
#include <mimetypewriter.h>


class FileTypesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        extern KSERVICE_EXPORT bool kservice_require_kded;
        kservice_require_kded = false;

#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5, 73, 0)
        QLoggingCategory::setFilterRules(QStringLiteral("kf.coreaddons.kdirwatch.debug=true"));
#else
        QLoggingCategory::setFilterRules(QStringLiteral("kf5.kcoreaddons.kdirwatch.debug=true"));
#endif

        QStandardPaths::setTestModeEnabled(true);

        m_mimeTypeCreatedSuccessfully = false;
        QStringList appsDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
        //qDebug() << appsDirs;
        m_localApps = appsDirs.first() + QLatin1Char('/');
        m_localConfig = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
        QVERIFY(QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/mime/packages")));

        QFile::remove(m_localConfig.filePath(QStringLiteral("mimeapps.list")));

        // Create fake applications for some tests below.
        bool mustUpdateKSycoca = false;
        fakeApplication = QStringLiteral("fakeapplication.desktop");
        if (createDesktopFile(m_localApps + fakeApplication))
            mustUpdateKSycoca = true;
        fakeApplication2 = QStringLiteral("fakeapplication2.desktop");
        if (createDesktopFile(m_localApps + fakeApplication2))
            mustUpdateKSycoca = true;

        // Cleanup after testMimeTypePatterns if it failed mid-way
        const QString packageFileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/") + QStringLiteral("packages/text-plain.xml") ;
        if (!packageFileName.isEmpty()) {
            QFile::remove(packageFileName);
            MimeTypeWriter::runUpdateMimeDatabase();
            mustUpdateKSycoca = true;
        }

        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + QStringLiteral("filetypesrc"));

        if ( mustUpdateKSycoca ) {
            // Update ksycoca in ~/.kde-unit-test after creating the above
            runKBuildSycoca();
        }
        KService::Ptr fakeApplicationService = KService::serviceByStorageId(fakeApplication);
        QVERIFY(fakeApplicationService);
    }

    void testMimeTypeGroupAutoEmbed()
    {
        MimeTypeData data(QStringLiteral("text"));
        QCOMPARE(data.majorType(), QStringLiteral("text"));
        QCOMPARE(data.name(), QStringLiteral("text"));
        QVERIFY(data.isMeta());
        QCOMPARE(data.autoEmbed(), MimeTypeData::No); // text doesn't autoembed by default
        QVERIFY(!data.isDirty());
        data.setAutoEmbed(MimeTypeData::Yes);
        QCOMPARE(data.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // save to disk. Should succeed, but return false (no need to run update-mime-database)
        QVERIFY(!data.isDirty());
        // Check what's on disk by creating another MimeTypeData instance
        MimeTypeData data2(QStringLiteral("text"));
        QCOMPARE(data2.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(!data2.isDirty());
        data2.setAutoEmbed(MimeTypeData::No); // revert to default, for next time
        QVERIFY(data2.isDirty());
        QVERIFY(!data2.sync());
        QVERIFY(!data2.isDirty());

        // TODO test askSave after cleaning up the code
    }

    void testMimeTypeAutoEmbed()
    {
        QMimeDatabase db;
        MimeTypeData data(db.mimeTypeForName(QStringLiteral("text/plain")));
        QCOMPARE(data.majorType(), QStringLiteral("text"));
        QCOMPARE(data.minorType(), QStringLiteral("plain"));
        QCOMPARE(data.name(), QStringLiteral("text/plain"));
        QVERIFY(!data.isMeta());
        QCOMPARE(data.autoEmbed(), MimeTypeData::UseGroupSetting);
        QVERIFY(!data.isDirty());
        data.setAutoEmbed(MimeTypeData::Yes);
        QCOMPARE(data.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // save to disk. Should succeed, but return false (no need to run update-mime-database)
        QVERIFY(!data.isDirty());
        // Check what's on disk by creating another MimeTypeData instance
        MimeTypeData data2(db.mimeTypeForName(QStringLiteral("text/plain")));
        QCOMPARE(data2.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(!data2.isDirty());
        data2.setAutoEmbed(MimeTypeData::UseGroupSetting); // revert to default, for next time
        QVERIFY(data2.isDirty());
        QVERIFY(!data2.sync());
        QVERIFY(!data2.isDirty());
    }

    void testMimeTypePatterns()
    {
        // Given the text/plain mimetype
        QMimeDatabase db;
        MimeTypeData data(db.mimeTypeForName(QStringLiteral("text/plain")));
        QCOMPARE(data.name(), QStringLiteral("text/plain"));
        QCOMPARE(data.majorType(), QStringLiteral("text"));
        QCOMPARE(data.minorType(), QStringLiteral("plain"));
        QVERIFY(!data.isMeta());
        QStringList patterns = data.patterns();
        QVERIFY(patterns.contains(QStringLiteral("*.txt")));
        QVERIFY(!patterns.contains(QStringLiteral("*.toto")));

        // When the user changes the patterns
        const QStringList origPatterns = patterns;
        patterns.removeAll(QStringLiteral("*.txt"));
        patterns.append(QStringLiteral("*.toto")); // yes, a french guy wrote this, as you can see
        patterns.sort(); // for future comparisons
        QVERIFY(!data.isDirty());
        data.setPatterns(patterns);
        QVERIFY(data.isDirty());
        bool needUpdateMimeDb = data.sync();
        QVERIFY(needUpdateMimeDb);
        MimeTypeWriter::runUpdateMimeDatabase();

        // Then the GUI and the QMimeDatabase API should show the new patterns
        QCOMPARE(data.patterns(), patterns);
        data.refresh(); // reload from the xml
        QCOMPARE(data.patterns(), patterns);
        // Check what's in QMimeDatabase
        QStringList newPatterns = db.mimeTypeForName(QStringLiteral("text/plain")).globPatterns();
        newPatterns.sort();
        QCOMPARE(newPatterns, patterns);
        QVERIFY(!data.isDirty());

        // And then removing the custom file by hand should revert to the initial state
        const QString packageFileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/") + QStringLiteral("packages/text-plain.xml") ;
        QVERIFY(!packageFileName.isEmpty());
        QFile::remove(packageFileName);
        MimeTypeWriter::runUpdateMimeDatabase();
        // Check what's in QMimeDatabase
        newPatterns = db.mimeTypeForName(QStringLiteral("text/plain")).globPatterns();
        newPatterns.sort();
        QCOMPARE(newPatterns, origPatterns);
    }

    void testAddService()
    {
        QMimeDatabase db;
        QString mimeTypeName = QStringLiteral("application/rtf"); // use inherited mimetype to test #321706
        MimeTypeData data(db.mimeTypeForName(mimeTypeName));
        QStringList appServices = data.appServices();
        //qDebug() << appServices;
        QVERIFY(!appServices.isEmpty());
        const QString oldPreferredApp = appServices.first();
        QVERIFY(!appServices.contains(fakeApplication)); // already there? hmm can't really test then
        QVERIFY(!data.isDirty());
        appServices.prepend(fakeApplication);
        data.setAppServices(appServices);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        QVERIFY(!data.isDirty());
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkAddedAssociationsContains(mimeTypeName, fakeApplication);

        // Test reordering apps, i.e. move fakeApplication under oldPreferredApp
        appServices.removeFirst();
        appServices.insert(1, fakeApplication);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        QVERIFY(!data.isDirty());
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkAddedAssociationsContains(mimeTypeName, fakeApplication);

        // Now test removing (in the same test, since it's inter-dependent)
        QVERIFY(appServices.removeAll(fakeApplication) > 0);
        data.setAppServices(appServices);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);
    }

    void testRemoveTwice()
    {
        QMimeDatabase db;
        // Remove fakeApplication from image/png
        QString mimeTypeName = QStringLiteral("image/png");
        MimeTypeData data(db.mimeTypeForName(mimeTypeName));
        QStringList appServices = data.appServices();
        qDebug() << "initial list for" << mimeTypeName << appServices;
        QVERIFY(appServices.removeAll(fakeApplication) > 0);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);

        // Remove fakeApplication2 from image/png; must keep the previous entry in "Removed Associations"
        qDebug() << "Removing fakeApplication2";
        QVERIFY(appServices.removeAll(fakeApplication2) > 0);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication2);

        // And now re-add fakeApplication2...
        qDebug() << "Re-adding fakeApplication2";
        appServices.prepend(fakeApplication2);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);
        checkRemovedAssociationsDoesNotContain(mimeTypeName, fakeApplication2);
    }

    void testCreateMimeType()
    {
        QMimeDatabase db;
        const QString mimeTypeName = QStringLiteral("fake/unit-test-fake-mimetype");
        // Clean up after previous runs if necessary
        if (MimeTypeWriter::hasDefinitionFile(mimeTypeName))
            MimeTypeWriter::removeOwnMimeType(mimeTypeName);

        MimeTypeData data(mimeTypeName, true);
        data.setComment(QStringLiteral("Fake MimeType"));
        QStringList patterns = QStringList() << QStringLiteral("*.pkg.tar.gz");
        data.setPatterns(patterns);
        QVERIFY(data.isDirty());
        QVERIFY(data.sync());
        MimeTypeWriter::runUpdateMimeDatabase();
        //runKBuildSycoca();
        // QMimeDatabase doesn't even try to update the cache if less than
        // 5000 ms have passed
        QTest::qSleep(5000);
        QMimeType mime = db.mimeTypeForName(mimeTypeName);
        QVERIFY(mime.isValid());
        QCOMPARE(mime.comment(), QStringLiteral("Fake MimeType"));
        QCOMPARE(mime.globPatterns(), patterns); // must sort them if more than one

        // Testcase for the shaman.xml bug
        QCOMPARE(db.mimeTypeForFile(QStringLiteral("/whatever/foo.pkg.tar.gz")).name(), QStringLiteral("fake/unit-test-fake-mimetype"));

        m_mimeTypeCreatedSuccessfully = true;
    }

    void testDeleteMimeType()
    {
        QMimeDatabase db;
        if (!m_mimeTypeCreatedSuccessfully)
            QSKIP("This test relies on testCreateMimeType");
        const QString mimeTypeName = QStringLiteral("fake/unit-test-fake-mimetype");
        QVERIFY(MimeTypeWriter::hasDefinitionFile(mimeTypeName));
        MimeTypeWriter::removeOwnMimeType(mimeTypeName);
        MimeTypeWriter::runUpdateMimeDatabase();
        //runKBuildSycoca();
        QMimeType mime = db.mimeTypeForName(mimeTypeName);
        QVERIFY(mime.isValid());
    }

    void testModifyMimeTypeComment() // of a system mimetype. And check that it's re-read correctly.
    {
        QMimeDatabase db;
        QString mimeTypeName = QStringLiteral("image/png");
        MimeTypeData data(db.mimeTypeForName(mimeTypeName));
        QCOMPARE(data.comment(), QString::fromLatin1("PNG image"));
        QString fakeComment = QStringLiteral("PNG image [testing]");
        data.setComment(fakeComment);
        QVERIFY(data.isDirty());
        QVERIFY(data.sync());
        MimeTypeWriter::runUpdateMimeDatabase();
        //runKBuildSycoca();
        QMimeType mime = db.mimeTypeForName(mimeTypeName);
        QVERIFY(mime.isValid());
        QCOMPARE(mime.comment(), fakeComment);

        // Cleanup
        QVERIFY(MimeTypeWriter::hasDefinitionFile(mimeTypeName));
        MimeTypeWriter::removeOwnMimeType(mimeTypeName);
    }

    void cleanupTestCase()
    {
        // If we remove it, then every run of the unit test has to run kbuildsycoca... slow.
        //QFile::remove(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1Char('/') + "fakeapplication.desktop");
    }

private: // helper methods

    void checkAddedAssociationsContains(const QString& mimeTypeName, const QString& application)
    {
        const KConfig config(m_localConfig.filePath(QStringLiteral("mimeapps.list")), KConfig::NoGlobals);
        const KConfigGroup group(&config, "Added Associations");
        const QStringList addedEntries = group.readXdgListEntry(mimeTypeName);
        if (!addedEntries.contains(application)) {
            qWarning() << addedEntries << "does not contain" << application;
            QVERIFY(addedEntries.contains(application));
        }
    }

    void checkRemovedAssociationsContains(const QString& mimeTypeName, const QString& application)
    {
        const KConfig config(m_localConfig.filePath(QStringLiteral("mimeapps.list")), KConfig::NoGlobals);
        const KConfigGroup group(&config, "Removed Associations");
        const QStringList removedEntries = group.readXdgListEntry(mimeTypeName);
        if (!removedEntries.contains(application)) {
            qWarning() << removedEntries << "does not contain" << application;
            QVERIFY(removedEntries.contains(application));
        }
    }

    void checkRemovedAssociationsDoesNotContain(const QString& mimeTypeName, const QString& application)
    {
        const KConfig config(m_localConfig.filePath(QStringLiteral("mimeapps.list")), KConfig::NoGlobals);
        const KConfigGroup group(&config, "Removed Associations");
        const QStringList removedEntries = group.readXdgListEntry(mimeTypeName);
        if (removedEntries.contains(application)) {
            qWarning() << removedEntries << "contains" << application;
            QVERIFY(!removedEntries.contains(application));
        }
    }

    void runKBuildSycoca()
    {
        // Wait for notifyDatabaseChanged DBus signal
        // (The real KCM code simply does the refresh in a slot, asynchronously)

        QProcess proc;
        //proc.setProcessChannelMode(QProcess::ForwardedChannels);
        const QString kbuildsycoca = QStandardPaths::findExecutable(QLatin1String(KBUILDSYCOCA_EXENAME));
        QVERIFY(!kbuildsycoca.isEmpty());
        QStringList args;
        args << QStringLiteral("--testmode");
        proc.start(kbuildsycoca, args);
        QSignalSpy spy(KSycoca::self(), SIGNAL(databaseChanged(QStringList)));
        proc.waitForFinished();
        qDebug() << "waiting for signal";
        QVERIFY(spy.wait(10000));
        qDebug() << "got signal";
    }

    bool createDesktopFile(const QString& path)
    {
        if (!QFile::exists(path)) {
            KDesktopFile file(path);
            KConfigGroup group = file.desktopGroup();
            group.writeEntry("Name", "FakeApplication");
            group.writeEntry("Type", "Application");
            group.writeEntry("Exec", "ls");
            group.writeEntry("MimeType", "image/png");
            return true;
        }
        return false;
    }

    void checkMimeTypeServices(const QString& mimeTypeName, const QStringList& expectedServices)
    {
        QMimeDatabase db;
        MimeTypeData data2(db.mimeTypeForName(mimeTypeName));
        if (data2.appServices() != expectedServices)
            qDebug() << "got" << data2.appServices() << "expected" << expectedServices;
        QCOMPARE(data2.appServices(), expectedServices);
    }

    QString fakeApplication; // storage id of the fake application
    QString fakeApplication2; // storage id of the fake application2
    QString m_localApps;
    QDir m_localConfig;
    bool m_mimeTypeCreatedSuccessfully;
};

QTEST_MAIN(FileTypesTest)

#include "filetypestest.moc"
