/*
 * sdb_auth.c
 *
 * AUTHOR: Ulric Eriksson <ulric@qbranch.se>
 *
 * Based on ncsa_auth by Arjan de Vet <Arjan.deVet@adv.iae.nl>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sdb.h>

static int cb_db(int n, char **p, void *closure)
{
	return 0;
}

int main(int argc, char **argv)
{
    char *url, query[1024];
    int n;
    char buf[256];
    char *user, *passwd, *p;
    setbuf(stdout, NULL);
    if (argc != 2) {
	fprintf(stderr, "Usage: sdb_auth url\n");
	exit(1);
    }
    url = argv[1];
    sdb_init();
    while (fgets(buf, 256, stdin) != NULL) {
	if ((p = strchr(buf, '\n')) != NULL)
	    *p = '\0';		/* strip \n */
	if ((user = strtok(buf, " ")) == NULL) {
		printf("ERR\n");
		continue;
	}
	if ((passwd = strtok(NULL, "")) == NULL) {
		printf("ERR\n");
		continue;
	}
	sprintf(query,
		"select * from htpasswd "
		"where user = '%s' "
		"and passwd = '%s'",
		user, passwd);
	n = sdb_query(url, query, cb_db, NULL);
	if (n < 1) {
	    printf("ERR\n");
	} else {
	    printf("OK\n");
	}
    }
    exit(0);
}
