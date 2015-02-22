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

#ifdef HAVE_LIBLAGO

/* Highly experimental! */

#include <stdio.h>
#include <stdlib.h>
#include <lago.h>

#include "common.h"
#include "sdb.h"

static int store_result(LRST resultset,
	int (*callback)(int, char **, void *), void *closure)
{
	int i, ncols;
	char **p;
	int row = 0;

/*printf("store_result()\n");
*/
	if (resultset == 0) return -1;
	ncols = Lgetncols(resultset);
/*printf("ncols = %d\n", ncols);
*/
	if (ncols == 0) return 0;

	p = sdb_malloc(ncols*sizeof *p);
	while (Lfetch(resultset) == LFETCH_MORE) {
		for (i = 1; i <= ncols; i++) {
			p[i-1] = (char *)Lgetasstr(resultset, i);
		}
		(*callback)(ncols, p, closure);
		row++;
	}
	sdb_free(p);
	return row;
}

static int lago_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	char *username = sdb_url_value(d, "uid");
	char *password = sdb_url_value(d, "pwd");
	char *database = sdb_url_value(d, "db");
	char *port = sdb_url_value(d, "port");
	char *host = sdb_url_value(d, "host");

	int n;
	LCTX context = Lnewctx();
	LRST resultset;
	int res;

	n = Lconnect(context, host, port, database, username, password);
	if (n) {
		Ldelctx(context);
		return -1;
	}

	resultset = Lquery(context, q);
	if (resultset == 0) {
		Ldelctx(context);
		return -1;
	}
	if (Lgeterrcode(context)) {
		fprintf(stderr, "Error: Native %05d SQLSTATE %s\n",
			Lgeterrcode(context), Lgetsqlstate(context));
		fprintf(stderr, "       %s\n", Lgeterrmsg(context));
		return -1;
	}
	res = store_result(resultset, callback, closure);
	Ldelctx(context);
	sdb_free(username);
	sdb_free(database);
	sdb_free(host);
	sdb_free(password);
	sdb_free(port);
	return res;
}

void sdb_init_lago(void)
{
	sdb_register_driver("lago", lago_driver, NULL, NULL);
}

#else

void sdb_init_lago(void)
{
	;
}

#endif	/* LAGO */
