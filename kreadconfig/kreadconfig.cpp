/*  Read KConfig() entries - for use in shell scripts.

    Copyright (c) 2001 Red Hat, Inc.

    Programmed by Bernhard Rosenkraenzer <bero@redhat.com>

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

/*
 * If --type is specified as bool, the return value is 0 if the value
 * is set, 1 if it isn't set. There is no output.
 *
 * If --type is specified as num, the return value matches the value
 * of the key. There is no output.
 *
 * If --type is not set, the value of the key is simply printed to stdout.
 *
 * Usage examples:
 *	if kreadconfig --group KDE --key macStyle --type bool; then
 *		echo "We're using Mac-Style menus."
 *	else
 *		echo "We're using normal menus."
 *	fi
 *
 *	TRASH=`kreadconfig --group Paths --key Trash`
 *	if test -n "$TRASH"; then
 *		mv someFile "$TRASH"
 *	else
 *		rm someFile
 *	fi
 */

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <stdio.h>

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	KAboutData aboutData("kreadconfig", 0, i18n("KReadConfig"),
		"1.0.1",
		i18n("Read KConfig entries - for use in shell scripts"),
		KAboutData::License_GPL,
		i18n("(c) 2001 Red Hat, Inc."));
	aboutData.addAuthor(i18n("Bernhard Rosenkraenzer"), QString(), "bero@redhat.com");

	KAboutData::setApplicationData(aboutData);

	QCommandLineParser parser;
	parser.addOption(QCommandLineOption("file", i18n("Use <file> instead of global config"), "file"));
	parser.addOption(QCommandLineOption("group", i18n("Group to look in. Use repeatedly for nested groups."), "group", "KDE"));
	parser.addOption(QCommandLineOption("key", i18n("Key to look for"), "key"));
	parser.addOption(QCommandLineOption("default", i18n("Default value"), "value"));
	parser.addOption(QCommandLineOption("type", i18n("Type of variable"), "type"));

	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	QStringList groups=parser.values("group");
	QString key=parser.value("key");
	QString file=parser.value("file");
	QString dflt=parser.value("default");
	QString type=parser.value("type").toLower();

	if (parser.positionalArguments().isEmpty()) {
		parser.showHelp(1);
	}

	KSharedConfig::openConfig();

	KConfig *konfig;
	bool configMustDeleted = false;
	if (file.isEmpty())
	   konfig = KSharedConfig::openConfig().data();
	else
	{
		konfig = new KConfig( file, KConfig::NoGlobals );
		configMustDeleted=true;
	}
	KConfigGroup cfgGroup = konfig->group("");
	foreach (const QString &grp, groups)
		cfgGroup = cfgGroup.group(grp);
	if(type=="bool") {
		dflt=dflt.toLower();
		bool def=(dflt=="true" || dflt=="on" || dflt=="yes" || dflt=="1");
		bool retValue = !cfgGroup.readEntry(key, def);
		if ( configMustDeleted )
			delete konfig;
		return retValue;
	} else if((type=="num") || (type=="int")) {
		int retValue = cfgGroup.readEntry(key, dflt.toInt());
		if ( configMustDeleted )
			delete konfig;
		return retValue;
	} else if (type=="path"){
		fprintf(stdout, "%s\n", cfgGroup.readPathEntry(key, dflt).toLocal8Bit().data());
		if ( configMustDeleted )
			delete konfig;
		return 0;
	} else {
		/* Assume it's a string... */
		fprintf(stdout, "%s\n", cfgGroup.readEntry(key, dflt).toLocal8Bit().data());
		if ( configMustDeleted )
			delete konfig;
		return 0;
	}
}

