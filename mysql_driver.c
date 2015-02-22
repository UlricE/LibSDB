/*
   Copyright (C) 2000-2005  Ulric Eriksson <ulric@siag.nu>
 
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

#ifdef HAVE_LIBMYSQLCLIENT

/* Highly experimental! */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#include "common.h"
#include "sdb.h"

struct conn_info {
	MYSQL mysql;
};

#define FAIL(m, p) {\
	fprintf(stderr, "error '%s': %s\n", p, mysql_error(m));\
	return -1;\
}

static int store_result(MYSQL *mysql,
		int (*callback)(int, char **, void *), void *closure)
{
	int ncols, row = 0;
	char **p;
	MYSQL_RES *mysql_res;
	MYSQL_ROW mysql_row;

	if (mysql == NULL) FAIL(mysql, "mysql is null");
	mysql_res = mysql_store_result(mysql);
	ncols = mysql_field_count(mysql);

	if (ncols) {
		while ((mysql_row = mysql_fetch_row(mysql_res))) {
			p = (char **)mysql_row;
			(*callback)(ncols, p, closure);
			row++;
		}
	}
	mysql_free_result(mysql_res);
	return row;
}

static void *sdb_mysql_open(char *url)
{
	struct conn_info *ci = sdb_malloc(sizeof *ci);
	char *host = sdb_url_value(url, "host");
	char *database = sdb_url_value(url, "db");
	char *user = sdb_url_value(url, "uid");
	char *password = sdb_url_value(url, "pwd");

	mysql_init(&(ci->mysql));
	if (!mysql_real_connect(&(ci->mysql), host, user, password,
				database, 0, NULL, 0)) {
		fprintf(stderr, "error '%s': %s\n",
			"connect", mysql_error(&(ci->mysql)));
		sdb_free(ci);
		ci = NULL;
	}

	sdb_free(host);
	sdb_free(password);
	sdb_free(user);
	sdb_free(database);
	return ci;
}

static int sdb_mysql_close(void *db)
{
	struct conn_info *ci = db;
        if (ci) {
            mysql_close(&(ci->mysql));
            sdb_free(ci);
        }
	return 0;
}

static int mysql_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	int res;
	struct conn_info *ci;

	if (pdb == NULL) ci = sdb_mysql_open(d);
	else ci = pdb;

	if (ci == NULL) {
		sdb_debug("Can't open database");
		return -1;
	}

	if (mysql_real_query(&(ci->mysql), q, strlen(q)))
		FAIL(&(ci->mysql), "query");
	res = store_result(&(ci->mysql), callback, closure);
	if (pdb == NULL) sdb_mysql_close(ci);
	return res;
}

void sdb_init_mysql(void)
{
	sdb_register_driver("mysql", mysql_driver,
			sdb_mysql_open, sdb_mysql_close);
}

#else

void sdb_init_mysql(void)
{
	;
}

#endif	/* MYSQL */
