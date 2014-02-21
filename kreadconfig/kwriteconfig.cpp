/*  Write KConfig() entries - for use in shell scripts.

    Copyright (c) 2001 Red Hat, Inc.
    Copyright (c) 2001 Luís Pedro Coelho <luis_pedro@netcabo.pt>

    Programmed by Luís Pedro Coelho <luis_pedro@netcabo.pt>
    based on kreadconfig by Bernhard Rosenkraenzer <bero@redhat.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KConfig>
#include <KConfigGroup>
#include <stdio.h>
#include <KAboutData>
#include <KLocalizedString>
#include <QCoreApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
	KAboutData aboutData("kwriteconfig", 0, i18n("KWriteConfig"),
		"1.0.0",
		i18n("Write KConfig entries - for use in shell scripts"),
		KAboutData::License_GPL,
		i18n("(c) 2001 Red Hat, Inc. & Luís Pedro Coelho"));
	aboutData.addAuthor("Luís Pedro Coelho", QString(), "luis_pedro@netcabo.pt");
	aboutData.addAuthor("Bernhard Rosenkraenzer", i18n("Wrote kreadconfig on which this is based"), "bero@redhat.com");

	QCoreApplication app(argc, argv);
	KAboutData::setApplicationData(aboutData);

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addOption(QCommandLineOption("file", i18n("Use <file> instead of global config"), "file"));
	parser.addOption(QCommandLineOption("group", i18n("Group to look in. Use repeatedly for nested groups."), "group", "KDE"));
	parser.addOption(QCommandLineOption("key <key>", i18n("Key to look for")));
	parser.addOption(QCommandLineOption("type <type>", i18n("Type of variable. Use \"bool\" for a boolean, otherwise it is treated as a string")));
	parser.addPositionalArgument("value", i18n( "The value to write. Mandatory, on a shell use '' for empty" ));

	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	QStringList groups=parser.values("group");
	QString key=parser.value("key");
	QString file=parser.value("file");
	QString type=parser.value("type").toLower();


	if (parser.positionalArguments().isEmpty()) {
		parser.showHelp(1);
	}
	QByteArray value = parser.positionalArguments().first().toLocal8Bit();

	KConfig *konfig;
	if (file.isEmpty())
	konfig = new KConfig(QString::fromLatin1( "kdeglobals"), KConfig::NoGlobals );
	else
	konfig = new KConfig( file, KConfig::NoGlobals );

		KConfigGroup cfgGroup = konfig->group("");
		foreach (const QString &grp, groups)
			cfgGroup = cfgGroup.group(grp);
	if ( konfig->accessMode() != KConfig::ReadWrite || cfgGroup.isEntryImmutable( key ) ) return 2;

	if(type=="bool") {
		// For symmetry with kreadconfig we accept a wider range of values as true than Qt
		bool boolvalue=(value=="true" || value=="on" || value=="yes" || value=="1");
		cfgGroup.writeEntry( key, boolvalue );
	} else if (type=="path") {
		cfgGroup.writePathEntry( key, QString::fromLocal8Bit( value ) );
	} else {
		cfgGroup.writeEntry( key, QString::fromLocal8Bit( value ) );
	}
	konfig->sync();
		delete konfig;
	return 0;
}

