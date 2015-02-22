/*
   Copyright (C) 2002-2005  Ulric Eriksson <ulric@siag.nu>
 
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

#ifdef HAVE_LIBMSQL

#include <stdio.h>
#include <stdlib.h>
#include <msql.h>
#include "common.h"
#include "sdb.h"

struct conn_info {
	int sock;
};

static void *sdb_msql_open(char *url)
{
	char *host = sdb_url_value(url, "host");
	char *db = sdb_url_value(url, "db");
	int sock;
	struct conn_info *ci;

	if (db == NULL) {
		sdb_debug("No db in %s", url);
		return NULL;
	}
	if ((sock = msqlConnect(host)) < 0) {
		sdb_debug("Can't connect to %s: %s", host, msqlErrMsg);
		return NULL;
	}
	if (msqlSelectDB(sock, db) < 0) {
		sdb_debug("Can't select %s: %s", db, msqlErrMsg);
		msqlClose(sock);
		return NULL;
	}
	ci = sdb_malloc(sizeof *ci);
	ci->sock = sock;
	return ci;
}

static int sdb_msql_close(void *db)
{
	struct conn_info *ci = db;
	if (ci && ci->sock >= 0) {
		msqlClose(ci->sock);
	}
	return 0;
}

/* Parameters:
   pdb = pointer to previously opened database connection, or NULL
   d = url string
   q = query string
   callback = function to send the data to
   closure = pointer to data passed from application
*/
static int msql_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	int n, off;
	struct conn_info *ci;
	m_result *result;
	m_row cur;
	char **row;

	if (pdb == NULL) ci = sdb_msql_open(d);
	else ci = pdb;

	if (!ci) {
		sdb_debug("Can't open database");
		return -1;
	}
	n = msqlQuery(ci->sock, q);
	if (n < 0) {
		sdb_debug("Query failed: %s", msqlErrMsg);
		n = -1;
		goto Done;
	}
	result = msqlStoreResult();
	if (result == NULL) {
		sdb_debug("null result");
		n = -1;
		goto Done;
	}

	n = 0;
	row = sdb_malloc(msqlNumFields(result)*sizeof *row);
	while ((cur = msqlFetchRow(result))) {
		for (off = 0; off < msqlNumFields(result); off++)
			row[off] = cur[off];
		(*callback)(msqlNumFields(result), row, closure);
		n++;
	}
	sdb_free(row);

Done:
	if (pdb == NULL) sdb_msql_close(ci);

	return n;
}

void sdb_init_msql(void)
{
	sdb_register_driver("msql", msql_driver,
		sdb_msql_open, sdb_msql_close);
}

#else

void sdb_init_msql(void)
{
	;
}

#endif	/* MSQL */
