/*
   Copyright (C) 2000-2002  Ulric Eriksson <ulric@siag.nu>
 
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "sdb.h"

#define SQL_VARCHAR 12

#define SQL_SUCCESS                            "00000"
#define SQL_INSUFFICIENT_ITEM_DESCRIPTOR_AREAS "01005"
#define SQL_NO_DATA                            "02000"
#define SQL_CONNECTION_REJECTED                "28000"

exec sql begin declare section;
static varchar statement  [129];       /* Extended statement name */
static varchar cursor     [129];       /* Extended cursor name    */
static varchar descriptor [129];       /* SQL descriptor name     */
static char    sqlstate   [6];         /* SQLSTATE                */
static char    buf     [8192];      /* Data buffer             */
exec sql end declare section;

static int  seqno = 0;     /* Sequence number for generating unique names */

static int DsqlConnect(char *url)
{
	exec sql whenever sqlerror goto exception;
	exec sql begin declare section;
	varchar  database [129] = "";
	varchar  username [129] = "";
	varchar  password [19]  = "";
	exec sql end declare section;
	char* p, *db, *user, *pass;

	if (sdb_debuglevel) sdb_debug("DsqlConnect(%s)", url);

	db = sdb_url_value(url, "db");
	if (db == NULL) {
		sdb_debug("No db in %s", url);
		return -1;
	}
	strcpy(database, db);
	sdb_free(db);
	user = sdb_url_value(url, "uid");
	if (user == NULL) {
		sdb_debug("No uid in %s", url);
		return -1;
	}
	strcpy(username, user);
	sdb_free(user);
	pass = sdb_url_value(url, "pwd");
	if (pass == NULL) {
		sdb_debug("No pwd in %s", url);
		return -1;
	}
	strcpy(password, pass);
	sdb_free(pass);

	for (p = username; *p; p++) *p = (char)toupper((int)*p); 

	exec sql connect to :database user :username using :password;

	return 1;

exception:

	if (strcmp(sqlstate,SQL_CONNECTION_REJECTED) == 0) {
		return 0;
	}
	return -1;
}

static int DsqlDisconnect(void)
{
	exec sql whenever sqlerror goto exception;

	if (sdb_debuglevel) sdb_debug("DsqlDisconnect()");

	exec sql commit;
	exec sql disconnect all;
	return 0;

exception:

	return -1;
}

static int DsqlExecute(char* sqlstmt)
{
	exec sql whenever sqlerror goto exception;
	exec sql begin declare section;
	int  count;
	int  n;
	int  type;
	int  length;
	exec sql end declare section;

	if (sdb_debuglevel) sdb_debug("DsqlExecute(%s)", sqlstmt);

	if (sqlstmt == NULL) return -1;
	/*
	 *  Make unique statement and descriptor names.
	 */
	seqno++;
	sprintf(statement, "%d",seqno);
	sprintf(descriptor,"%d",seqno);

	strncpy(buf,sqlstmt,sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	exec sql prepare :statement from :buf;

	exec sql allocate descriptor :descriptor;

	exec sql describe output :statement using sql descriptor :descriptor;
	if (strcmp(sqlstate,SQL_INSUFFICIENT_ITEM_DESCRIPTOR_AREAS) == 0) {
		/*
		 *  The descriptor area was insufficient.
		 *  Allocate a new one with appropriate size.
		 */
		exec sql get descriptor :descriptor :count = count;
		exec sql deallocate descriptor :descriptor;
		exec sql allocate descriptor :descriptor with max :count;
		exec sql describe output :statement using sql descriptor :descriptor;
	}
	/*
	 *  Get descriptor count to see if this was a select statement or not.
	 */
	exec sql get descriptor :descriptor :count = count;
	if (count == 0) {
		/*
		 *  Non-select statement. Execute.
		 */
		exec sql execute :statement;
		/*
		 *  Deallocate statement and descriptor.
		 */
		exec sql deallocate prepare :statement;
		exec sql deallocate descriptor :descriptor;
		/*
		 *  Return successful completion of non-select statement.
		 */
		return 0;
	} else {
		/*
		 *  Select statement. Set all columns to VARCHAR(512).
		 *                    MIMER/SQL automatic type conversion will
		 *                    handle numeric data.
		 */
		type   = SQL_VARCHAR;
		length = 512;
		for (n = 1; n <= count; n++) {
			exec sql set descriptor :descriptor value :n type   = :type,
					                      length = :length;
		}
		/*
		 *  Make a unique cursor name.
		 */
		sprintf(cursor,"%d",seqno);

		exec sql allocate :cursor cursor for :statement;
		exec sql open :cursor;

		return seqno;
	}

exception:

	return -1;
}

/* This function processes one row */
static int DsqlFetch(int c,char* record,int len,
	int (*callback)(int, char **, void *), void *closure)
{
	exec sql whenever sqlerror goto exception;
	exec sql begin declare section;
	int  count;
	int  length;
	int  isnull;
	int  n;
	exec sql end declare section;

	char **row;

	if (sdb_debuglevel) sdb_debug("DsqlFetch(%d, %s, %d)", c, record, len);

	/*
	 *  Check arguments.
	 */
	if (record == NULL || len < 32) return -1;
	/*
	 *  Build cursor and descriptor names.
	 */
	sprintf(cursor,    "%d",c);
	sprintf(descriptor,"%d",c);
	/*
	 *  Fetch next row from result table.
	 */
	exec sql fetch next from :cursor into sql descriptor :descriptor;
	if (strcmp(sqlstate,SQL_SUCCESS) == 0) {
		/*
		 *  Get number of columns.
		 */
		exec sql get descriptor :descriptor :count = count;
	if (sdb_debuglevel) sdb_debug("%d columns: ", count);
		row = sdb_malloc((count+1) * sizeof *row);
		for (n = 1; n <= count; n++) {
			/*
			 *  Get one column value.
			 */
			exec sql get descriptor :descriptor value :n :buf = data,
					              :length = returned_length,
					              :isnull = indicator;
			/*
			 *  Check NULL indicator, trim spaces and append a tab.
			 */
			if (isnull == -1) {
#if 0
				;	/* ignore */
#else
				row[n] = NULL;
				if (sdb_debuglevel) sdb_debug("null ");
#endif
			} else {
				buf[length] = '\0';
#if 0
				if (cbd->res == NIL) {
					cbd->res = strcons(length, buf);
				} else {
					ins_data(cbd->buf, siod_interpreter,
					buf, val, MTEXT, cbd->sht,
					cbd->row+cbd->r, cbd->col+n-1);
				}
#else
				row[n] = sdb_strdup(buf);
				if (sdb_debuglevel) sdb_debug("%s ", buf);
#endif
			}
		}
		(*callback)(count, row+1, closure);
		for (n = 1; n <= count; n++) sdb_free(row[n]);
		sdb_free(row);
		return count;
	} else if (strcmp(sqlstate,SQL_NO_DATA) == 0) {
		/*
		 *  No more data.
		 */
		return 0;
	}

exception:

	return -1;
}

static int DsqlClose(int c)
{
	exec sql whenever sqlerror goto exception;

	if (sdb_debuglevel) sdb_debug("DsqlClose(%d)", c);

	sprintf(cursor,    "%d",c);
	sprintf(statement, "%d",c);
	sprintf(descriptor,"%d",c);

	exec sql close :cursor;

	exec sql deallocate prepare :statement;
	exec sql deallocate descriptor :descriptor;
	return 0;

exception:

	return -1;
}

static int DsqlError(char* message,int len)
{
	exec sql whenever sqlerror goto exception;
	exec sql begin declare section;
	int   count;
	int   length;
	char  state [6]; /* returned_sqlstate may not be stored sqlstate */
	int   n;
	exec sql end declare section;
	char* p;

	if (sdb_debuglevel) sdb_debug("DsqlError(%s, %d)", message, len);

	if (message == NULL || len < 32) return -1;
	/*
	 *  Get number of errors.
	 */
	exec sql get diagnostics :count = number;
	for (n = 1; n <= count; n++) {
		/*
		 *  Get diagnostics of an error.
		 */
		exec sql get diagnostics exception :n :buf = message_text,
					           :length = message_length,
					           :state  = returned_sqlstate;
		/*
		 *  Prepare message area.
		 */
		if (length > len - 18) length = len - 18;
		memcpy(message,buf,length);
		p = strchr(message,':');
		if (p) *p = '\n';
		if (length > 64) {
			p = strchr(&message[64],' ');
			if (p) *p = '\n';
		}
		message += length;
		len     -= length;
		*message++ = '\n';
		/*
		 *  Add SQLSTATE code.
		 */
		strcpy(message,"SQLSTATE:");
		message += 9;
		strcpy(message,state);
		message += 5;
		*message++ = '\n';
		*message++ = '\n';
		len -= 17;
		if (len < 32) break;
	}
	*message = '\0';
	return 0;

exception:

	strcpy(message,"cannot get diagnostics\n\n");
	return -1;
}

/* MAIN */

static char sqlbuf[4096];

static void error(void)
{
	DsqlError(sqlbuf,sizeof(sqlbuf));
	sdb_debug("%s",sqlbuf);
}

static int mimer_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	int   rc;
	int   cursor;
	int   rowcount;

	rc = DsqlConnect(d);
	if (rc <= 0) {
		error();
		return -1;
	}

	/*
	 *  Exec argv[4] as an SQL statement
	 */
	strcpy(sqlbuf, q);

	cursor = DsqlExecute(sqlbuf);
	if (cursor > 0) {
		rowcount = 0;
		while ((rc = DsqlFetch(cursor, sqlbuf, sizeof sqlbuf,
					callback, closure)) > 0) {
			rowcount++;
		}
		if (rc < 0 || DsqlClose(cursor) < 0) {
			error();
		}
	} else if (cursor == 0) {
		if (sdb_debuglevel)
			sdb_debug("SQL statement executed successfully!");
		rowcount = 0;
	} else {
		error();
		rowcount = -1;
	}

	if (DsqlDisconnect() < 0) error();
	return rowcount;
}

void sdb_init_mimer(void)
{
	sdb_register_driver("mimer", mimer_driver, NULL, NULL);
}

