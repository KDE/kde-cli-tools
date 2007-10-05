/* Write KConfig() entries - for use in shell scripts.
 * (c) 2001 Red Hat, Inc. & Luís Pedro Coelho
 * Programmed by Luís Pedro Coelho <luis_pedro@netcabo.pt>
 *  based on kreadconfig by Bernhard Rosenkraenzer <bero@redhat.com>
 *
 * License: GPL
 *
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

	KCmdLineOptions options;
	options.add("file <file>", ki18n("Use <file> instead of global config"));
	options.add("group <group>", ki18n("Group to look in"), "KDE");
	options.add("key <key>", ki18n("Key to look for"));
	options.add("type <type>", ki18n("Type of variable. Use \"bool\" for a boolean, otherwise it is treated as a string"));
	options.add("+value", ki18n( "The value to write. Mandatory, on a shell use '' for empty" ));
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args=KCmdLineArgs::parsedArgs();

	QString group=args->getOption("group");
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
	   konfig = new KConfig(QString::fromLatin1( "kdeglobals"), KConfig::CascadeConfig );
	else
	   konfig = new KConfig( file, KConfig::CascadeConfig );

        KConfigGroup cfgGroup = konfig->group(group);
	if ( konfig->getConfigState() != KConfig::ReadWrite || cfgGroup.entryIsImmutable( key ) ) return 2;

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

