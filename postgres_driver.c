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

#ifdef HAVE_LIBPQ

/* Highly experimental! */

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "common.h"
#include "sdb.h"

struct conn_info {
	PGconn *conn;
};

static int store_result(PGresult *pgres,
	int (*callback)(int, char **, void *), void *closure)
{
	int i, j, ncols;
	char **p;
	ExecStatusType est;

	if (!pgres ||
	    ((est = PQresultStatus(pgres)) != PGRES_COMMAND_OK &&
	     est != PGRES_TUPLES_OK)) {
		fprintf(stderr, "%s\n", PQresultErrorMessage(pgres));
		PQclear(pgres);
		return -1;
	}

	ncols = PQnfields(pgres);
	p = sdb_malloc(ncols*sizeof *p);
	for (i = 0; i < PQntuples(pgres); i++) {
		for (j = 0; j < ncols; j++) {
			p[j] = PQgetvalue(pgres, i, j);
		}
		(*callback)(ncols, p, closure);
	}
	sdb_free(p);
	PQclear(pgres);
	return i;
}

static void *sdb_postgres_open(char *url)
{
    struct conn_info *ci = sdb_malloc(sizeof *ci);

    char *host = sdb_url_value(url, "host");
    char *database = sdb_url_value(url, "db");
    char *port = sdb_url_value(url, "port");
    char *user = sdb_url_value(url, "user");
    char *passwd = sdb_url_value(url, "passwd");
    char *pgoptions = NULL, *pgtty = NULL;
    
//  conn = PQsetdb(host, port, pgoptions, pgtty, database);
    ci->conn = PQsetdbLogin(host, port, pgoptions, pgtty, 
                            database, user, passwd);

    if (PQstatus(ci->conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database '%s' failed.\n",
                database);
        fprintf(stderr, "%s\n", PQerrorMessage(ci->conn));
        PQfinish(ci->conn);
        sdb_free(ci);
        ci = NULL;
    }

    sdb_free(host);
    sdb_free(database);
    sdb_free(port);
    sdb_free(user);
    sdb_free(passwd);
    return ci;
}

static int sdb_postgres_close(void *db)
{
    struct conn_info *ci = db;
    if (ci) {
        PQfinish(ci->conn);
        sdb_free(ci);
    }
    return 0;
}

static int postgres_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	PGresult *pgres;
	int res;
	struct conn_info *ci;

	if (pdb == NULL)
            ci = sdb_postgres_open(d);
	else
            ci = pdb;

	if (ci == NULL) {
            sdb_debug("Can't open database");
            return -1;
	}

	pgres = PQexec(ci->conn, q);
	res = store_result(pgres, callback, closure);

	if (pdb == NULL)
            sdb_postgres_close(ci);

	return res;
}

void sdb_init_postgres(void)
{
	sdb_register_driver("postgres", postgres_driver,
                            sdb_postgres_open, sdb_postgres_close);
}

#else

void sdb_init_postgres(void)
{
	;
}

#endif	/* POSTGRES */
