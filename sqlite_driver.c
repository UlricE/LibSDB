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

#ifdef HAVE_LIBSQLITE

/* Highly experimental! */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite.h>
#include "common.h"
#include "sdb.h"

#define FAIL(db, msg) {\
	fprintf(stderr, "error '%s'\n", msg);\
	sqlite_close(db);\
	return -1;\
}

struct cb_data {
	int r;			/* row within query result */
	int (*callback)(int, char **, void *);
	void *closure;
};

static int store_result(void *data, int n, char **p, char **q)
{
	struct cb_data *cbd = (struct cb_data *)data;
	int (*callback)(int, char **, void *) = cbd->callback;
	void *closure = cbd->closure;

	(*callback)(n, p, closure);
	cbd->r++;
	return 0;
}

static void *sdb_sqlite_open(char *url)
{
	sqlite *db;
	char *errmsg;
	char *database = sdb_url_value(url, "db");
	if (!database) {
		fprintf(stderr, "No db in '%s'\n", url);
		return NULL;
	}
	if (sdb_debuglevel) sdb_debug("sqlite_open(%s)", database);
	db = sqlite_open(database, 0666, &errmsg);
	sdb_free(database);
	return db;
}

static int sdb_sqlite_close(void *db)
{
	sqlite_close(db);
	return 0;
}

/* Parameters:
   pdb = pointer to previously opened database connection, or NULL
   d = url string
   q = query string
   callback = function to send the data to
   closure = pointer to data passed from application
*/
static int sqlite_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	int n;
	sqlite *db;
	char *errmsg;
	struct cb_data cbd;

	cbd.r = 0;
	cbd.callback = callback;
	cbd.closure = closure;

	if (pdb == NULL) db = sdb_sqlite_open(d);
	else db = pdb;

	if (!db) {
		sdb_debug("Can't open database");
		return -1;
	}
	n = sqlite_exec(db, q, store_result, &cbd, &errmsg);
	if (n != 0) {
		sdb_debug("Query failed: %s", errmsg);
	}

	if (pdb == NULL) sdb_sqlite_close(db);

	return cbd.r;
}

void sdb_init_sqlite(void)
{
	sdb_register_driver("sqlite", sqlite_driver,
		sdb_sqlite_open, sdb_sqlite_close);
}

#else

void sdb_init_sqlite(void)
{
	;
}

#endif	/* SQLITE */
