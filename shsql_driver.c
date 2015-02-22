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

#ifdef HAVE_LIBSHSQL

/* Highly experimental! */

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "sdb.h"

extern int SHSQL_allconfig(void);
extern int SHSQL_sql( int dbc, char *sql );
extern int SHSQL_getrow( int dbc, char *fields[], int *n );

static void shsql_error(int code)
{
	switch (code) {
	case 0:
		fprintf(stderr, "Success [0]\n");
		break;
	case 1:
		fprintf(stderr, "Requested record not found [1]\n");
		break;
	case 7:
		fprintf(stderr, "Record locked [7]\n");
		break;
	case 9:
		fprintf(stderr, "The table is write-locked [9]\n");
		break;
	case 20:
		fprintf(stderr, "Attempt to update read-only database [20]\n");
		break;
	default:
		fprintf(stderr, "Other error [%d]\n", code);
		break;
	}
}

/* Parameters:
   pdb = pointer to previously opened database connection, or NULL
   d = url string
   q = query string
   callback = function to send the data to
   closure = pointer to data passed from application
*/
static int shsql_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	char *channel;
	char *fields[80];
	int i, n, nfields, dbc;

        channel = sdb_url_value(d, "dbc");
	if (channel) {
		dbc = atoi(channel);
	} else {
		dbc = 0;
	}

	n = SHSQL_sql(dbc, q);
	if (n != 0) {
		shsql_error(n);
		return -1;
	}

	i = 0;
	while (SHSQL_getrow(dbc, fields, &nfields) == 0) {
		(*callback)(nfields, fields, closure);
		i++;
	}

	return i;
}

void sdb_init_shsql(void)
{
	int n = SHSQL_allconfig();
	if (n) shsql_error(n);
	sdb_register_driver("shsql", shsql_driver,
		NULL, NULL);
}

#else

void sdb_init_shsql(void)
{
	;
}

#endif	/* SHSQL */
