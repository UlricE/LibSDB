        /*******************************************************/
        /* Mimer (R) Embedded SQL Preprocessor  Version 8.2.4G */
        /*******************************************************/

static       int  sql001[4] =
{
    0,0,0,0
};
static const int  sql002[13] =
{
    3,
    40,-3,128,0,
    38,-3,128,0,
    39,-3,18,0
};
static       int  sql003[4] =
{
    0,0,0,0
};
static const int  sql004[1] =
{
    0
};
static       int  sql005[4] =
{
    0,0,0,0
};
static const int  sql006[1] =
{
    0
};
static       int  sql007[4] =
{
    0,0,0,0
};
static const int  sql008[9] =
{
    2,
    42,-3,128,0,
    6,-1,8191,0
};
static       int  sql009[4] =
{
    0,0,0,0
};
static const int  sql010[5] =
{
    1,
    1,-3,128,0
};
static       int  sql011[4] =
{
    0,0,0,0
};
static const int  sql012[9] =
{
    2,
    42,-3,128,0,
    1,-3,128,0
};
static       int  sql013[4] =
{
    0,0,0,0
};
static const int  sql014[9] =
{
    2,
    1,-3,128,0,
    3,4,sizeof(int),0
};
static       int  sql015[4] =
{
    0,0,0,0
};
static const int  sql016[5] =
{
    1,
    1,-3,128,0
};
static       int  sql017[4] =
{
    0,0,0,0
};
static const int  sql018[9] =
{
    2,
    1,-3,128,0,
    3,4,sizeof(int),0
};
static       int  sql019[4] =
{
    0,0,0,0
};
static const int  sql020[9] =
{
    2,
    42,-3,128,0,
    1,-3,128,0
};
static       int  sql021[4] =
{
    0,0,0,0
};
static const int  sql022[9] =
{
    2,
    1,-3,128,0,
    3,4,sizeof(int),0
};
static       int  sql023[4] =
{
    0,0,0,0
};
static const int  sql024[5] =
{
    1,
    42,-3,128,0
};
static       int  sql025[4] =
{
    0,0,0,0
};
static const int  sql026[5] =
{
    1,
    42,-3,128,0
};
static       int  sql027[4] =
{
    0,0,0,0
};
static const int  sql028[5] =
{
    1,
    1,-3,128,0
};
static       int  sql029[4] =
{
    0,0,0,0
};
static const int  sql030[17] =
{
    4,
    1,-3,128,0,
    2,4,sizeof(int),0,
    10,4,sizeof(int),0,
    11,4,sizeof(int),0
};
static       int  sql031[4] =
{
    0,0,0,0
};
static const int  sql032[9] =
{
    2,
    43,-3,128,0,
    42,-3,128,0
};
static       int  sql033[4] =
{
    0,0,0,0
};
static const int  sql034[5] =
{
    1,
    43,-3,128,0
};
static       int  sql035[4] =
{
    0,0,0,0
};
static const int  sql036[9] =
{
    2,
    43,-3,128,0,
    1,-3,128,0
};
static       int  sql037[4] =
{
    0,0,0,0
};
static const int  sql038[9] =
{
    2,
    1,-3,128,0,
    3,4,sizeof(int),0
};
static       int  sql039[4] =
{
    0,0,0,0
};
static const int  sql040[21] =
{
    5,
    1,-3,128,0,
    2,4,sizeof(int),0,
    9,-1,8191,0,
    18,4,sizeof(int),0,
    15,4,sizeof(int),0
};
static       int  sql041[4] =
{
    0,0,0,0
};
static const int  sql042[5] =
{
    1,
    43,-3,128,0
};
static       int  sql043[4] =
{
    0,0,0,0
};
static const int  sql044[5] =
{
    1,
    42,-3,128,0
};
static       int  sql045[4] =
{
    0,0,0,0
};
static const int  sql046[5] =
{
    1,
    1,-3,128,0
};
static       int  sql047[4] =
{
    0,0,0,0
};
static const int  sql048[5] =
{
    1,
    20,4,sizeof(int),0
};
static       int  sql049[4] =
{
    0,0,0,0
};
static const int  sql050[17] =
{
    4,
    30,4,sizeof(int),0,
    26,-1,8191,0,
    27,4,sizeof(int),0,
    23,-1,5,0
};
static void* sqlpaa[5];
static int   sqlrcv[6];

#define __MIMESQLH
#ifdef  _WIN32
#define MIMAPI __stdcall
#else
#define MIMAPI
#endif

#ifdef  __cplusplus
extern "C" {
#endif
void MIMAPI dbapi4(int*,char*,int*,const int*,void**);
void MIMAPI dberm4(int*,char*,int*);
#ifdef  __cplusplus
}
#endif

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

/****
exec sql begin declare section;
 ****/
static char    statement  [129];       /* Extended statement name */
static char    cursor     [129];       /* Extended cursor name    */
static char    descriptor [129];       /* SQL descriptor name     */
static char    sqlstate   [6];         /* SQLSTATE                */
static char    buf     [8192];      /* Data buffer             */
/****
exec sql end declare section;
 ****/

static int  seqno = 0;     /* Sequence number for generating unique names */

static int DsqlConnect(char *url)
{
/****
	exec sql whenever sqlerror goto exception;
 ****/
     /* Mimer Extended SQL */
/****
	exec sql begin declare section;
 ****/
	char     database [129] = "";
	char     username [129] = "";
	char     password [19]  = "";
/****
	exec sql end declare section;
 ****/
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

/****
	exec sql connect to :database user :username using :password;
 ****/
     /* Mimer Extended SQL */
 {
     sqlpaa[0] = (void*)database;
     sqlpaa[1] = (void*)username;
     sqlpaa[2] = (void*)password;
     sqlrcv[0] = 119;
     dbapi4(sqlrcv,sqlstate,sql001,sql002,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }

	return 1;

exception:

	if (strcmp(sqlstate,SQL_CONNECTION_REJECTED) == 0) {
		return 0;
	}
	return -1;
}

static int DsqlDisconnect(void)
{
/****
	exec sql whenever sqlerror goto exception;
 ****/
     /* Mimer Extended SQL */

	if (sdb_debuglevel) sdb_debug("DsqlDisconnect()");

/****
	exec sql commit;
 ****/
     /* Transitional SQL-92 */
 {
     sqlrcv[0] = 62;
     dbapi4(sqlrcv,sqlstate,sql003,sql004,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
/****
	exec sql disconnect all;
 ****/
     /* Full SQL-92 */
 {
     sqlrcv[0] = 59;
     dbapi4(sqlrcv,sqlstate,sql005,sql006,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
	return 0;

exception:

	return -1;
}

static int DsqlExecute(char* sqlstmt)
{
/****
	exec sql whenever sqlerror goto exception;
 ****/
     /* Mimer Extended SQL */
/****
	exec sql begin declare section;
 ****/
	int  count;
	int  n;
	int  type;
	int  length;
/****
	exec sql end declare section;
 ****/

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

/****
	exec sql prepare :statement from :buf;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)statement;
     sqlpaa[1] = (void*)buf;
     sqlrcv[0] = 1;
     dbapi4(sqlrcv,sqlstate,sql007,sql008,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }

/****
	exec sql allocate descriptor :descriptor;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)descriptor;
     sqlrcv[0] = 83;
     dbapi4(sqlrcv,sqlstate,sql009,sql010,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }

/****
	exec sql describe output :statement using sql descriptor :descriptor;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)statement;
     sqlpaa[1] = (void*)descriptor;
     sqlrcv[0] = 81;
     dbapi4(sqlrcv,sqlstate,sql011,sql012,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
	if (strcmp(sqlstate,SQL_INSUFFICIENT_ITEM_DESCRIPTOR_AREAS) == 0) {
		/*
		 *  The descriptor area was insufficient.
		 *  Allocate a new one with appropriate size.
		 */
/****
		exec sql get descriptor :descriptor :count = count;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)descriptor;
      sqlpaa[1] = (void*)&count;
      sqlrcv[0] = 85;
      dbapi4(sqlrcv,sqlstate,sql013,sql014,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
/****
		exec sql deallocate descriptor :descriptor;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)descriptor;
      sqlrcv[0] = 84;
      dbapi4(sqlrcv,sqlstate,sql015,sql016,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
/****
		exec sql allocate descriptor :descriptor with max :count;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)descriptor;
      sqlpaa[1] = (void*)&count;
      sqlrcv[0] = 83;
      dbapi4(sqlrcv,sqlstate,sql017,sql018,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
/****
		exec sql describe output :statement using sql descriptor :descriptor;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)statement;
      sqlpaa[1] = (void*)descriptor;
      sqlrcv[0] = 81;
      dbapi4(sqlrcv,sqlstate,sql019,sql020,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
	}
	/*
	 *  Get descriptor count to see if this was a select statement or not.
	 */
/****
	exec sql get descriptor :descriptor :count = count;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)descriptor;
     sqlpaa[1] = (void*)&count;
     sqlrcv[0] = 85;
     dbapi4(sqlrcv,sqlstate,sql021,sql022,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
	if (count == 0) {
		/*
		 *  Non-select statement. Execute.
		 */
/****
		exec sql execute :statement;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)statement;
      sqlrcv[0] = 5;
      dbapi4(sqlrcv,sqlstate,sql023,sql024,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
		/*
		 *  Deallocate statement and descriptor.
		 */
/****
		exec sql deallocate prepare :statement;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)statement;
      sqlrcv[0] = 77;
      dbapi4(sqlrcv,sqlstate,sql025,sql026,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
/****
		exec sql deallocate descriptor :descriptor;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)descriptor;
      sqlrcv[0] = 84;
      dbapi4(sqlrcv,sqlstate,sql027,sql028,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
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
/****
			exec sql set descriptor :descriptor value :n type   = :type,
					                      length = :length;
 ****/
       /* Full SQL-92 */
   {
       sqlpaa[0] = (void*)descriptor;
       sqlpaa[1] = (void*)&n;
       sqlpaa[2] = (void*)&type;
       sqlpaa[3] = (void*)&length;
       sqlrcv[0] = 86;
       dbapi4(sqlrcv,sqlstate,sql029,sql030,sqlpaa);
       sqlstate[5] = '\0';
       if (sqlrcv[4] == 3) goto exception;
   }
		}
		/*
		 *  Make a unique cursor name.
		 */
		sprintf(cursor,"%d",seqno);

/****
		exec sql allocate :cursor cursor for :statement;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)cursor;
      sqlpaa[1] = (void*)statement;
      sqlrcv[0] = 76;
      dbapi4(sqlrcv,sqlstate,sql031,sql032,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
/****
		exec sql open :cursor;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)cursor;
      sqlrcv[0] = 13;
      dbapi4(sqlrcv,sqlstate,sql033,sql034,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }

		return seqno;
	}

exception:

	return -1;
}

/* This function processes one row */
static int DsqlFetch(int c,char* record,int len,
	int (*callback)(int, char **, void *), void *closure)
{
/****
	exec sql whenever sqlerror goto exception;
 ****/
     /* Mimer Extended SQL */
/****
	exec sql begin declare section;
 ****/
	int  count;
	int  length;
	int  isnull;
	int  n;
/****
	exec sql end declare section;
 ****/

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
/****
	exec sql fetch next from :cursor into sql descriptor :descriptor;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)cursor;
     sqlpaa[1] = (void*)descriptor;
     sqlrcv[0] = 88;
     dbapi4(sqlrcv,sqlstate,sql035,sql036,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
	if (strcmp(sqlstate,SQL_SUCCESS) == 0) {
		/*
		 *  Get number of columns.
		 */
/****
		exec sql get descriptor :descriptor :count = count;
 ****/
      /* Full SQL-92 */
  {
      sqlpaa[0] = (void*)descriptor;
      sqlpaa[1] = (void*)&count;
      sqlrcv[0] = 85;
      dbapi4(sqlrcv,sqlstate,sql037,sql038,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
	if (sdb_debuglevel) sdb_debug("%d columns: ", count);
		row = sdb_malloc((count+1) * sizeof *row);
		for (n = 1; n <= count; n++) {
			/*
			 *  Get one column value.
			 */
/****
			exec sql get descriptor :descriptor value :n :buf = data,
					              :length = returned_length,
					              :isnull = indicator;
 ****/
       /* Full SQL-92 */
   {
       sqlpaa[0] = (void*)descriptor;
       sqlpaa[1] = (void*)&n;
       sqlpaa[2] = (void*)buf;
       sqlpaa[3] = (void*)&length;
       sqlpaa[4] = (void*)&isnull;
       sqlrcv[0] = 85;
       dbapi4(sqlrcv,sqlstate,sql039,sql040,sqlpaa);
       sqlstate[5] = '\0';
       if (sqlrcv[4] == 3) goto exception;
   }
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
/****
	exec sql whenever sqlerror goto exception;
 ****/
     /* Mimer Extended SQL */

	if (sdb_debuglevel) sdb_debug("DsqlClose(%d)", c);

	sprintf(cursor,    "%d",c);
	sprintf(statement, "%d",c);
	sprintf(descriptor,"%d",c);

/****
	exec sql close :cursor;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)cursor;
     sqlrcv[0] = 101;
     dbapi4(sqlrcv,sqlstate,sql041,sql042,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }

/****
	exec sql deallocate prepare :statement;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)statement;
     sqlrcv[0] = 77;
     dbapi4(sqlrcv,sqlstate,sql043,sql044,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
/****
	exec sql deallocate descriptor :descriptor;
 ****/
     /* Full SQL-92 */
 {
     sqlpaa[0] = (void*)descriptor;
     sqlrcv[0] = 84;
     dbapi4(sqlrcv,sqlstate,sql045,sql046,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
	return 0;

exception:

	return -1;
}

static int DsqlError(char* message,int len)
{
/****
	exec sql whenever sqlerror goto exception;
 ****/
     /* Mimer Extended SQL */
/****
	exec sql begin declare section;
 ****/
	int   count;
	int   length;
	char  state [6]; /* returned_sqlstate may not be stored sqlstate */
	int   n;
/****
	exec sql end declare section;
 ****/
	char* p;

	if (sdb_debuglevel) sdb_debug("DsqlError(%s, %d)", message, len);

	if (message == NULL || len < 32) return -1;
	/*
	 *  Get number of errors.
	 */
/****
	exec sql get diagnostics :count = number;
 ****/
     /* Transitional SQL-92 */
 {
     sqlpaa[0] = (void*)&count;
     sqlrcv[0] = 75;
     dbapi4(sqlrcv,sqlstate,sql047,sql048,sqlpaa);
     sqlstate[5] = '\0';
     if (sqlrcv[4] == 3) goto exception;
 }
	for (n = 1; n <= count; n++) {
		/*
		 *  Get diagnostics of an error.
		 */
/****
		exec sql get diagnostics exception :n :buf = message_text,
					           :length = message_length,
					           :state  = returned_sqlstate;
 ****/
      /* Transitional SQL-92 */
  {
      sqlpaa[0] = (void*)&n;
      sqlpaa[1] = (void*)buf;
      sqlpaa[2] = (void*)&length;
      sqlpaa[3] = (void*)state;
      sqlrcv[0] = 75;
      dbapi4(sqlrcv,sqlstate,sql049,sql050,sqlpaa);
      sqlstate[5] = '\0';
      if (sqlrcv[4] == 3) goto exception;
  }
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

