/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"
#include "misc.h"

char	*Spice_Path;
char	*News_File;
char	*Default_MFB_Cap;
char	*Help_Path;
char	*Lib_Path;


static void
env_overr(v, e)
    char    **v;
    char    *e;
{
#ifdef HAS_ENVIRON
    char    *p;
    if (v && e && (p = getenv(e)))
	*v = p;
#endif
}

static void
mkvar(p, b, v, e)
    char    **p;
    char    *b;
    char    *v;
    char    *e;
{
    char buffer[256];

    env_overr(p, e);
    if (!*p) {
	sprintf(buffer, "%s%s%s", b, DIR_PATHSEP, v);
	*p = tmalloc(strlen(buffer) + 1);
	strcpy(*p, buffer);
    }
}

void
ivars( )
{

    env_overr(&Spice_Exec_Dir, "SPICE_EXEC_DIR");
    env_overr(&Spice_Lib_Dir, "SPICE_LIB_DIR");

    mkvar(&News_File, Spice_Lib_Dir, "news", "SPICE_NEWS");
    mkvar(&Default_MFB_Cap, Spice_Lib_Dir, "mfbcap", "SPICE_MFBCAP");
    mkvar(&Help_Path, Spice_Lib_Dir, "helpdir", "SPICE_HELP_DIR");
    mkvar(&Lib_Path, Spice_Lib_Dir, "scripts", "SPICE_SCRIPTS");
    mkvar(&Spice_Path, Spice_Exec_Dir, "spice3", "SPICE_PATH");

    env_overr(&Spice_Host, "SPICE_HOST");
    env_overr(&Bug_Addr, "SPICE_BUGADDR");
    env_overr(&Def_Editor, "SPICE_EDITOR");
    env_overr(&AsciiRawFile, "SPICE_ASCIIRAWFILE");

}
