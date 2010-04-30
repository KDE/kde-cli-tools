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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdio.h>
//Added by qt3to4:

int main(int argc, char **argv)
{
	KAboutData aboutData("kwriteconfig", 0, ki18n("KWriteConfig"),
		"1.0.0",
		ki18n("Write KConfig entries - for use in shell scripts"),
		KAboutData::License_GPL,
		ki18n("(c) 2001 Red Hat, Inc. & Luís Pedro Coelho"));
	aboutData.addAuthor(ki18n("Luís Pedro Coelho"), KLocalizedString(), "luis_pedro@netcabo.pt");
	aboutData.addAuthor(ki18n("Bernhard Rosenkraenzer"), ki18n("Wrote kreadconfig on which this is based"), "bero@redhat.com");
	KCmdLineArgs::init(argc, argv, &aboutData);

	QCoreApplication app(argc, argv);

	KCmdLineOptions options;
	options.add("file <file>", ki18n("Use <file> instead of global config"));
	options.add("group <group>", ki18n("Group to look in. Use repeatedly for nested groups."), "KDE");
	options.add("key <key>", ki18n("Key to look for"));
	options.add("type <type>", ki18n("Type of variable. Use \"bool\" for a boolean, otherwise it is treated as a string"));
	options.add("+value", ki18n( "The value to write. Mandatory, on a shell use '' for empty" ));
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args=KCmdLineArgs::parsedArgs();

	QStringList groups=args->getOptionList("group");
	QString key=args->getOption("key");
	QString file=args->getOption("file");
	QString type=args->getOption("type").toLower();


	if (key.isNull() || !args->count()) {
		KCmdLineArgs::usage();
		return 1;
	}
	QByteArray value = args->arg( 0 ).toLocal8Bit();

	KComponentData inst(&aboutData);

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

