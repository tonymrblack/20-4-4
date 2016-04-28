/*
 * Security checks of enviroment
 * Added from shadow-utils package
 * by Arkadiusz Mi�kiewicz <misiek@misiek.eu.org>
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"

extern char **environ;

static char * const forbid[] = {
        "_RLD_=",
        "BASH_ENV=",    /* GNU creeping featurism strikes again... */
        "ENV=",
        "HOME=",
        "IFS=",
        "KRB_CONF=",
        "LD_",          /* anything with the LD_ prefix */
        "LIBPATH=",
        "MAIL=",
        "NLSPATH=",
        "PATH=",
        "SHELL=",
        "SHLIB_PATH=",
        (char *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
static char * const noslash[] = {
        "LANG=",
        "LANGUAGE=",
        "LC_",          /* anything with the LC_ prefix */
        (char *) 0
};

void
sanitize_env(void)
{
        char **envp = environ;
        char * const *bad;
        char **cur;
        char **move;

        for (cur = envp; *cur; cur++) {
                for (bad = forbid; *bad; bad++) {
                        if (strncmp(*cur, *bad, strlen(*bad)) == 0) {
                                for (move = cur; *move; move++)
                                        *move = *(move + 1);
                                cur--;
                                break;
                        }
                }
        }

        for (cur = envp; *cur; cur++) {
                for (bad = noslash; *bad; bad++) {
                        if (strncmp(*cur, *bad, strlen(*bad)) != 0)
                                continue;
                        if (!strchr(*cur, '/'))
                                continue;  /* OK */
                        for (move = cur; *move; move++)
                                *move = *(move + 1);
                        cur--;
                        break;
                }
        }
}
