/*
   Copyright (C) 2000-2005  Ulric Eriksson <ulric@siag.nu>
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.
 */

#ifdef HAVE_LIBGDBM

/*
skey=value stores the key key with the value value
fkey fetches the value of the key key
dkey deletes the key key
l lists all keys
r reorganizes the database
ekey tests if key key exists

The url looks like gdbm:db=database
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdbm.h>
#include "common.h"
#include "sdb.h"


static datum s2d(char *s)
{
	datum d;
	d.dptr = s;
	d.dsize = strlen(s)+1;
	return d;
}

#if 0	/* unused */
static char *d2s(datum d)
{
	return d.dptr;
}
#endif

static void problem(char *p)
{
	if (sdb_debuglevel) sdb_debug("problem(%s)", p);
	fprintf(stderr, "Problem: '%s'\n", p);
}

static GDBM_FILE openordie(char *name, int rw)
{
	GDBM_FILE dbf = gdbm_open(name, 512, rw, 0644, problem);
	if (sdb_debuglevel) sdb_debug("openordie(%s, %d) => %p", name, rw, dbf);
	if (dbf == NULL) problem("Can't open database");
	return dbf;
}

static int gdbm_driver(void *pdb, char *dd, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	char *database = sdb_url_value(dd, "db");
	GDBM_FILE dbf = NULL;

	int res = -1;
	int r;
	char *k, *v;
	datum d;

	if (database == NULL) {
		if (sdb_debuglevel) sdb_debug("no database");
		return -1;
	}

	switch (q[0]) {
	case 's':
		k = q+1;
		v = strchr(k, '=');
		if (v == NULL) {
			problem(q);
			break;
		}
		*v++ = '\0';
		dbf = openordie(database, GDBM_WRCREAT);
		if (dbf == NULL) {
			break;
		}
		r = gdbm_store(dbf, s2d(k), s2d(v), GDBM_REPLACE);
		if (r == -1) {
			problem(q);
		} else {
			res = 0;
		}
		break;
	case 'f':
		k = q+1;
		dbf = openordie(database, GDBM_READER);
		if (dbf == NULL) {
			break;
		}
		d = gdbm_fetch(dbf, s2d(k));
		v = d.dptr;
		if (v) {
			(*callback)(1, &v, closure);
			free(v);
			res = 0;
		}
		break;
	case 'd':
		k = q+1;
		dbf = openordie(database, GDBM_WRCREAT);
		if (dbf == NULL) {
			break;
		}
		r = gdbm_delete(dbf, s2d(k));
		if (r) {
			if (sdb_debuglevel) sdb_debug("Can't delete '%s'", k);
			res = -1;
		} else {
			if (sdb_debuglevel) sdb_debug("Key '%s' deleted", k);
			res = 0;
		}
		break;
	case 'l':
		if (sdb_debuglevel) sdb_debug("------");
		dbf = openordie(database, GDBM_READER);
		if (dbf == NULL) {
			break;
		}
		d = gdbm_firstkey(dbf);
		while (d.dptr) {
			k = d.dptr;
			(*callback)(1, &k, closure);
			if (sdb_debuglevel) sdb_debug("%s", k);
			d = gdbm_nextkey(dbf, d);
			free(k);
		}
		res = 0;
		if (sdb_debuglevel) sdb_debug("------");
		break;
	case 'r':
		dbf = openordie(database, GDBM_WRCREAT);
		if (dbf == NULL) {
			break;
		}
		gdbm_reorganize(dbf);
		if (sdb_debuglevel) sdb_debug("Database reorganized");
		res = 0;
		break;
	case 'e':
		k = q+1;
		dbf = openordie(database, GDBM_READER);
		if (dbf == NULL) {
			break;
		}
		r = gdbm_exists(dbf, s2d(k));
		if (r) {
			if (sdb_debuglevel) sdb_debug("Exists");
			res = 0;
		} else {
			if (sdb_debuglevel) sdb_debug("Doesn't exist");
			res = -1;
		}
		break;
	default:
		problem(q);
		break;
	}
	if (dbf) gdbm_close(dbf);
	/* end of code from slow.c */

	sdb_free(database);
	return res;
}

void sdb_init_gdbm(void)
{
	sdb_register_driver("gdbm", gdbm_driver, NULL, NULL);
}

#else

void sdb_init_gdbm(void)
{
	;
}

#endif	/* GDBM */
