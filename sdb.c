/*
   Copyright (C) 2000-2008  Ulric Eriksson <ulric@siag.nu>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the Licence, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include "common.h"
#include "sdb.h"

#define CONN_MAX 100

extern void sdb_init_gdbm(void);	/* gdbm_driver.c */
extern void sdb_init_ingres(void);	/* ingres_driver.c */
extern void sdb_init_lago(void);        /* lago_driver.c */
extern void sdb_init_mimer(void);       /* mimer_driver.c */
extern void sdb_init_msql(void);	/* msql_driver.c */
extern void sdb_init_mysql(void);       /* mysql_driver.c */
extern void sdb_init_odbc(void);	/* odbc_driver.c */
extern void sdb_init_oracle(void);      /* oracle_driver.c */
extern void sdb_init_postgres(void);    /* postgres_driver.c */
extern void sdb_init_shsql(void);	/* shsql_driver.c */
extern void sdb_init_sqlite(void);      /* sqlite_driver.c */
extern void sdb_init_sqlite3(void);     /* sqlite3_driver.c */
extern void sdb_init_tds(void);		/* tds_driver.c */
extern void sdb_init_text(void);	/* text_driver.c */

static struct sdb_driver {
	char *name;
	int (*driver)(void *, char *, char *,
			int (*)(int, char **, void *), void *);
	void *(*open)(char *);
	int (*close)(void *);
} drivers[20];

static int ndrivers = 0;

static struct sdb_conn {
	char *url;
	char *index;
	int driver;
	void *db;
} conn[CONN_MAX];

char *sdb_url_value(char *p, char *k)
{
        int len = strlen(k);
        char b[1024];
        int i;
 
	if (sdb_debuglevel) sdb_debug("sdb_url_value(%s, %s)", p, k);

        while (p) {
                if (!sdb_strncasecmp(p, k, len) && p[len] == '=') {
                        p = p+len+1;
                        for (i = 0; p[i] && p[i] != ':'; i++)
                                b[i] = p[i];
                        b[i] = '\0';
			if (sdb_debuglevel) {
				sdb_debug("Key %s found, value '%s'", k, b);
			}
                        return sdb_strdup(b);
                }
                p = strchr(p, ':');
                if (p) p++;
        }
	if (sdb_debuglevel) sdb_debug("Key %s not found, returning NULL", k);
        return NULL;
}

void sdb_register_driver(char *name,
	int (*driver)(void *, char *, char *,
			int (*)(int, char **, void *), void *),
	void *(*open)(char *), int (*close)(void *))
{
	if (sdb_debuglevel) sdb_debug("sdb_register_driver(%s, %p, %p, %p)\n",
				name, driver, open, close);
	drivers[ndrivers].name = name;
	drivers[ndrivers].driver = driver;
	drivers[ndrivers].open = open;
	drivers[ndrivers].close = close;
	ndrivers++;
}

/* Return index of driver or -1 for failure */
static int lookup_driver(char *url)
{
	char driver[1024], *p;
	int n, i;

	if (url == NULL) return -1;
	if (sdb_debuglevel) sdb_debug("lookup_driver(%s)", url);
	p = strchr(url, ':');
	if (p) n = p-url;
	else n = strlen(url);
	if (n > 1000) n = 1000;
	strncpy(driver, url, n);
	driver[n] = '\0';
	if (sdb_debuglevel) sdb_debug("driver name = '%s'", driver);
	for (i = 0; drivers[i].name; i++) {
	if (sdb_debuglevel) sdb_debug("compare with '%s'", drivers[i].name);
		if (!sdb_strcasecmp(drivers[i].name, driver)) {
			return i;
		}
	}
	return -1;
}

/*
;@db_query(url, query)
;@Makes database queries. <a href="db.html">More information</a>.
;@db_query("odbc:postgresql", "select * from foo")
;@
*/
int sdb_query(char *url, char *query,
	int (*callback)(int, char **, void *), void *closure)
{
        char *u, *p;
        int i, j;
	void *db;
 
        if (url == NULL || query == NULL) return -1;
	sdb_init();
 
        u = sdb_strdup(url);
        p = strchr(u, ':');
	if (p == NULL) {
		j = atoi(u);
		if (j < 0 || j >= CONN_MAX || conn[j].url == NULL) {
			sdb_debug("%d: no such connection", j);
			return -1;
		}
		if (strcmp(u, conn[j].index)) {
			sdb_debug("Index %s does not match %s",
				u, conn[j].index);
			return -1;
		}
		i = conn[j].driver;
		db = conn[j].db;
		p = conn[j].url;
	} else {
		i = lookup_driver(u);
		if (i == -1) {
			sdb_debug("No driver for %s", u);
			return -1;
		}
		db = NULL;
        	*p++ = '\0';
	}
 
	return (*drivers[i].driver)(db, p, query, callback, closure);
}

char *sdb_open(char *url)
{
	char b[100];
	void *db;
	int i, j;

	sdb_init();

	i = lookup_driver(url);
	if (i == -1) {
		sdb_debug("No driver for %s", url);
		return NULL;
	}
	if (drivers[i].open == NULL) db = NULL;
	else db = (*drivers[i].open)(url+strlen(drivers[i].name)+1);
	for (j = 0; j < CONN_MAX; j++) {
		if (conn[j].url == NULL) break;
	}
	if (j == CONN_MAX) {
		fprintf(stderr, "Too many open connections\n");
		return NULL;
	}
	conn[j].url = sdb_strdup(strchr(url, ':')+1);
	sprintf(b, "%d", j);
	conn[j].index = sdb_strdup(b);
	conn[j].driver = i;
	conn[j].db = db;
	return conn[j].index;
}

int sdb_close(char *url)
{
	int i, j, n;

	if (strchr(url, ':') || !isdigit(*url)) return -1;

	j = atoi(url);
	if (j < 0 || j >= CONN_MAX) return -1;

	i = conn[j].driver;
	if (drivers[i].close && conn[j].db) n = (*drivers[i].close)(conn[j].db);
	else n = 0;

	sdb_free(conn[j].url);
	conn[j].url = NULL;
	sdb_free(conn[j].index);
	conn[j].index = NULL;
	return n;
}

static void sdb_exit(void)
{
	int i;
	char b[1024];

	for (i = 0; i < CONN_MAX; i++) {
		if (conn[i].url) {
			sprintf(b, "%d", i);
			sdb_close(b);
		}
	}
}

void sdb_init_sdbd(void)
{
	sdb_register_driver("sdbd", sdbd_driver, sdbd_open, sdbd_close);
}

void sdb_init(void)
{
	static int init_done = 0;

	if (init_done) return;
	init_done = 1;
	ndrivers = 0;
	sdb_init_sdbd();
	sdb_init_ingres();
        sdb_init_lago();
        sdb_init_mysql();
        sdb_init_sqlite();
        sdb_init_sqlite3();
        sdb_init_mimer();
        sdb_init_postgres();
        sdb_init_oracle();
	sdb_init_odbc();
	sdb_init_gdbm();
	sdb_init_msql();
	sdb_init_shsql();
	sdb_init_tds();
	sdb_init_text();
	atexit(sdb_exit);
}
