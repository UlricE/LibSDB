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

#ifdef HAVE_LIBCLNTSH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <oratypes.h>
#include <ociapr.h>
#include <ocidem.h>

#include "common.h"
#include "sdb.h"

#define MAX_ITEM_BUFFER_SIZE    1024
#define MAX_SELECT_LIST_SIZE    64
#define PARSE_DEFERRED		0
#define V7_BEHAVIOR		2
#define NUMBER_WIDTH		8

/*
 * structures
 */

typedef struct
{
	sb4 dbsize;
	sb2 dbtype;
	sb1 buf[MAX_ITEM_BUFFER_SIZE];
	sb4 buflen;
	sb4 dsize;
	sb2 prec;
	sb2 scale;
	sb2 nullok;
} DescribeStruct;

typedef struct
{
	ub1 buf[MAX_ITEM_BUFFER_SIZE];
	float flt_buf;
	sword int_buf;
	sb2 indp;
	ub2 col_retlen;
	ub2 col_retcode;
} DefineStruct;

/* The size of the hda is 256 bytes on 32-bit systems and 512 bytes
   on 64-bit systems. We use 512 bytes everywhere for portability. */
struct conn_info
{
	DescribeStruct desc[MAX_SELECT_LIST_SIZE];
	DefineStruct def[MAX_SELECT_LIST_SIZE];
	Lda_Def lda;
	Cda_Def cda;
	ub1 hda[512];
};

static void handle_error (Lda_Def *lda, Cda_Def *cda)
{
    text msg[512];
    sword n;

    n = oerhms(lda, cda->rc, msg, (sword) sizeof (msg));
    sdb_debug ("%.*s", n, msg);
}

static void *sdb_oracle_open(char *url)
{
	char msg[100];
	char *username = sdb_url_value(url, "UID");
	char *password = sdb_url_value(url, "PWD");
	char *database = sdb_url_value(url, "DB");
	struct conn_info *ci = sdb_malloc(sizeof *ci);
	memset(&ci->lda, 0, sizeof ci->lda);
	memset(&ci->cda, 0, sizeof ci->cda);
	memset(ci->hda, 0, sizeof ci->hda);
	int n;

	n = olog (&ci->lda, /*(ub1 *)*/ ci->hda, /*(text *)*/username, -1,
		/*(text *)*/password, -1,
		/*(text *)*/ database, -1, /*(ub4)*/ OCI_LM_DEF);
	if (n) {
		sdb_debug ("Unable to connect to ORACLE");
		sdb_debug("olog returned %d", n);
		oermsg(n, msg);
		sdb_debug("Error message: '%s'", msg);
		handle_error(&ci->lda, &ci->cda);
		sdb_free (username);
		sdb_free (password);
		sdb_free(database);
		sdb_free (ci);
		return NULL;
	}
 
	n = oopen (&ci->cda, &ci->lda, (text *) 0, -1, -1, (text *) 0, -1);
	if (n) {
		sdb_debug ("Cannot open cursor");
		sdb_debug("oopen returned %d", n);
		handle_error(&ci->lda, &ci->cda);
		sdb_free (username);
		sdb_free (password);
		sdb_free(database);
		sdb_free (ci);
		ologof (&ci->lda);
		return NULL;
	}
	sdb_free (username);
	sdb_free (password);
	sdb_free(database);
	return ci;
}

static int sdb_oracle_close(void *db)
{
	struct conn_info *ci = db;
	if (oclose (&ci->cda)) {
		sdb_debug("Error closing cursor");
	}
	if (ologof (&ci->lda)) {
		sdb_debug("Error logging off");
	}
	sdb_free (ci);
	return 0;
}

static int store_result(Lda_Def *lda, Cda_Def *cda, DescribeStruct *desc,
		DefineStruct *def, sword ncols,
		int (*callback)(int, char **, void *), void *closure)
{
	sword col;
	long nrows = 0;
	char **p = sdb_malloc(ncols*sizeof *p);

	while (1) {
		if (ofetch (cda)) {
			if (cda->rc == NO_DATA_FOUND) {
				break;
			}
			if (cda->rc != NULL_VALUE_RETURNED) {
				handle_error (lda, cda);
				break;
			}	
	        }

		for (col = 0; col < ncols; col++) {
			char b[100];
			if (def[col].indp < 0) {
				p[col] = NULL;
				continue;
			}
			switch (desc[col].dbtype) {
			case FLOAT_TYPE:
				sprintf(b, "%0.*f", desc[col].scale,
					def[col].flt_buf);
				p[col] = sdb_strdup(b);
				break;
			case INT_TYPE:
				sprintf(b, "%ld", (long)def[col].int_buf);
				p[col] = sdb_strdup(b);
				break;
			default:
				p[col] = sdb_strdup((char *)def[col].buf);
				break;
			}
		}
		(*callback)(ncols, p, closure);
		for (col = 0; col < ncols; col++) {
			sdb_free(p[col]);
		}
		++nrows;
	}
	sdb_free(p);

	return nrows;
}

static sword describe_define (Lda_Def *lda, Cda_Def *cda, DescribeStruct *desc,
			DefineStruct *def)
{
	sword col, deflen, deftyp;
	static ub1 *defptr;

	for (col = 0; col < MAX_SELECT_LIST_SIZE; col++) {
		desc[col].buflen = MAX_ITEM_BUFFER_SIZE;
		if (odescr (cda, col + 1, &desc[col].dbsize,
			&desc[col].dbtype, &desc[col].buf[0],
			&desc[col].buflen, &desc[col].dsize,
			&desc[col].prec, &desc[col].scale,
			&desc[col].nullok)) {
			if (cda->rc == VAR_NOT_IN_LIST) {
				break;
			} else {
				handle_error (lda, cda);
				return 1;
			}
		}

		switch (desc[col].dbtype) {
		case NUMBER_TYPE:
			desc[col].dbsize = NUMBER_WIDTH;
			if (desc[col].scale != 0) {
				defptr = (ub1 *) &def[col].flt_buf;
				deflen = (sword) sizeof(float);
				deftyp = FLOAT_TYPE;
				desc[col].dbtype = FLOAT_TYPE;
			} else {
				defptr = (ub1 *) &def[col].int_buf;
				deflen = (sword) sizeof(sword);
			 	deftyp = INT_TYPE;
				desc[col].dbtype = INT_TYPE;
			}
			break;
		default:
			if (desc[col].dbtype == DATE_TYPE) {
				desc[col].dbsize = 32;
			}
			if (desc[col].dbtype == ROWID_TYPE) {
				desc[col].dbsize = 18;
			}
			defptr = def[col].buf;
			deflen = desc[col].dbsize > MAX_ITEM_BUFFER_SIZE ?
				MAX_ITEM_BUFFER_SIZE : desc[col].dbsize + 1;
			deftyp = STRING_TYPE;
			break;
		}

		if (odefin(cda, col + 1, defptr, deflen, deftyp, -1,
			&def[col].indp, (text *) 0, -1, -1,
			&def[col].col_retlen, &def[col].col_retcode)) {
			handle_error(lda, cda);
			return 1;
		}
	}

	return col;
}

static int exec_sql (char *query, Lda_Def *lda, Cda_Def *cda,
		DescribeStruct *desc, DefineStruct *def,
		int (*callback)(int, char **, void *), void *closure)
{
	sword ncols = 0, sql_function;
	int res = 0;

	if (oparse (cda, (text *) query, (sb4) -1,
		(sword) PARSE_DEFERRED, (ub4) V7_BEHAVIOR)) {
		handle_error(lda, cda);
		return -1;
	}

	sql_function = cda->ft;

	if (sql_function == FT_SELECT) {
		if ((ncols = describe_define (lda, cda, desc, def)) == -1) {
			return -1;
		}
	}

	if (oexec (cda)) {
		handle_error(lda, cda);
		return -1;
	}

	if (sql_function == FT_SELECT) {
		res = store_result(lda, cda, desc, def, ncols,
				callback, closure);
	}

	return res;
}

static int oracle_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	int res;
	struct conn_info *ci = pdb;

	if (pdb == NULL) ci = sdb_oracle_open (d);
	else ci = pdb;
	if (ci == NULL)
	{
		sdb_debug("Can't open database");
		return -1;
	}
	res = exec_sql(q, &ci->lda, &ci->cda, ci->desc, ci->def, callback, closure);
	if (pdb == NULL) sdb_oracle_close(ci);
	return res;
}

void sdb_init_oracle(void)
{
	sdb_register_driver("oracle", oracle_driver, sdb_oracle_open, sdb_oracle_close);
}

#else

void sdb_init_oracle(void)
{
	;
}

#endif	/* ORACLE */
