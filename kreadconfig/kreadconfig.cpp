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
#include <kglobal.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdio.h>

static KCmdLineOptions options[] =
{
	{ "file <file>", I18N_NOOP("Use <file> instead of global config"), 0 },
	{ "group <group>", I18N_NOOP("Group to look in"), "KDE" },
        { "key <key>", I18N_NOOP("Key to look for"), 0 },
        { "default <default>", I18N_NOOP("Default value"), 0 },
	{ "type <type>", I18N_NOOP("Type of variable"), 0 },
        KCmdLineLastOption
};
int main(int argc, char **argv)
{
	KAboutData aboutData("kreadconfig", I18N_NOOP("KReadConfig"),
		"1.0.1",
		I18N_NOOP("Read KConfig entries - for use in shell scripts"),
		KAboutData::License_GPL,
		"(c) 2001 Red Hat, Inc.");
	aboutData.addAuthor("Bernhard Rosenkraenzer", 0, "bero@redhat.com");
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args=KCmdLineArgs::parsedArgs();

	QString group=QString::fromLocal8Bit(args->getOption("group"));
	QString key=QString::fromLocal8Bit(args->getOption("key"));
	QString file=QString::fromLocal8Bit(args->getOption("file"));
	QCString dflt=args->getOption("default");
	QCString type=args->getOption("type").lower();

	if (key.isNull()) {
		KCmdLineArgs::usage();
		return 1;
	}

	KInstance inst(&aboutData);
	KGlobal::config();

	KConfig *konfig;
        bool configMustDeleted = false;
	if (file.isEmpty())
	   konfig = KGlobal::config();
	else
        {
	   konfig = new KConfig(file, true, false);
           configMustDeleted=true;
        }
	konfig->setGroup(group);
	if(type=="bool") {
		dflt=dflt.lower();
		bool def=(dflt=="true" || dflt=="on" || dflt=="yes" || dflt=="1");
                bool retValue = !konfig->readBoolEntry(key, def);
                if ( configMustDeleted )
                    delete konfig;
		return retValue;
	} else if((type=="num") || (type=="int")) {
            long retValue = konfig->readLongNumEntry(key, dflt.toLong());
            if ( configMustDeleted )
                delete konfig;
            return retValue;
	} else if (type=="path"){
                fprintf(stdout, "%s\n", konfig->readPathEntry(key, dflt).local8Bit().data());
                if ( configMustDeleted )
                    delete konfig;
		return 0;
	} else {
            /* Assume it's a string... */
                fprintf(stdout, "%s\n", konfig->readEntry(key, dflt).local8Bit().data());
                if ( configMustDeleted )
                    delete konfig;
		return 0;
	}
}

