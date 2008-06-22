/* Read KConfig() entries - for use in shell scripts.
 * (c) 2001 Red Hat, Inc.
 * Programmed by Bernhard Rosenkraenzer <bero@redhat.com>
 *
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
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdio.h>
int main(int argc, char **argv)
{
	KAboutData aboutData("kreadconfig", 0, ki18n("KReadConfig"),
		"1.0.1",
		ki18n("Read KConfig entries - for use in shell scripts"),
		KAboutData::License_GPL,
		ki18n("(c) 2001 Red Hat, Inc."));
	aboutData.addAuthor(ki18n("Bernhard Rosenkraenzer"), KLocalizedString(), "bero@redhat.com");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KCmdLineOptions options;
	options.add("file <file>", ki18n("Use <file> instead of global config"));
	options.add("group <group>", ki18n("Group to look in. Use repeatedly for nested groups."), "KDE");
	options.add("key <key>", ki18n("Key to look for"));
	options.add("default <default>", ki18n("Default value"));
	options.add("type <type>", ki18n("Type of variable"));
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args=KCmdLineArgs::parsedArgs();

	QStringList groups=args->getOptionList("group");
	QString key=args->getOption("key");
	QString file=args->getOption("file");
	QString dflt=args->getOption("default");
	QString type=args->getOption("type").toLower();

	if (key.isNull()) {
		KCmdLineArgs::usage();
		return 1;
	}

	KComponentData inst(&aboutData);
	KGlobal::config();

	KConfig *konfig;
        bool configMustDeleted = false;
	if (file.isEmpty())
	   konfig = KGlobal::config().data();
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

