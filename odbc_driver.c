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

#ifdef HAVE_LIBODBC

/* Needed for MS quasi-types */
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "common.h"
#include "sdb.h"

static int store_result(SQLHSTMT hstmt, int colanz,
	int (*callback)(int, char **, void *), void *closure)
{
	int row = 0;
	long erg;
	char **p;

	p = (char **)sdb_malloc(colanz*sizeof *p);
        while ((erg = SQLFetch(hstmt)) == SQL_SUCCESS) {
                int i;
                char b[1024];
                SQLINTEGER n;

                for (i = 1; i <= colanz; i++) {
                        erg = SQLGetData(hstmt, i, SQL_C_CHAR,
                                (SQLPOINTER)b, sizeof b, &n);
			if (erg != SQL_SUCCESS) p[i-1] = NULL;
			else p[i-1] = sdb_strdup(b);
                }
		(*callback)(colanz, p, closure);
		for (i = 1; i <= colanz; i++) sdb_free(p[i-1]);
		row++;
        }
	sdb_free(p);

	return row;
}

static int odbc_driver(void *pdb, char *d, char *q,
	int (*callback)(int, char **, void *), void *closure)
{
	char *uid = sdb_url_value(d, "uid");
	char *pwd = sdb_url_value(d, "pwd");
	char *dsn = sdb_url_value(d, "dsn");
	SQLHENV Env;
	long erg;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	char stat[10];
	SQLINTEGER err;
	SQLSMALLINT mlen, colanz;
	char msg[200];
	int res;

	if (dsn == NULL) return -1;

	erg = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Env);
	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		return -1;
	}
	erg = SQLSetEnvAttr(Env, SQL_ATTR_ODBC_VERSION,
                          (void *) SQL_OV_ODBC3, 0);
        if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
                SQLFreeHandle(SQL_HANDLE_ENV, Env);
		return -1;
	}
	erg = SQLAllocHandle(SQL_HANDLE_DBC, Env, &hdbc);
        if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
                SQLFreeHandle(SQL_HANDLE_ENV, Env);
		return -1;
	}
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *) 5, 0);
	erg = SQLConnect(hdbc, (SQLCHAR *) dsn, SQL_NTS,
                        (SQLCHAR *)uid, SQL_NTS, (SQLCHAR *)pwd, SQL_NTS);
        if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
                SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1,
                              stat, &err, msg, 100,
                              &mlen);
		SQLFreeHandle(SQL_HANDLE_ENV, Env);
		return -1;
	}
        erg = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
                SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, stat,
                              &err, msg, 100, &mlen);
                SQLFreeHandle(SQL_HANDLE_ENV, Env);
		return -1;
	}
 
        erg = SQLExecDirect(hstmt, q, SQL_NTS);
        if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
                SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1,
                              stat, &err, msg,
                              100, &mlen);
                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
                SQLFreeHandle(SQL_HANDLE_ENV, Env);
		return -1;
	}
	erg = SQLNumResultCols(hstmt, &colanz);
        if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                SQLDisconnect(hdbc);
                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
                SQLFreeHandle(SQL_HANDLE_ENV, Env);
		return -1;
	}
	res = store_result(hstmt, colanz, callback, closure);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, Env);

	sdb_free(uid);
	sdb_free(dsn);
	sdb_free(pwd);
	return res;
}

void sdb_init_odbc(void)
{
	sdb_register_driver("odbc", odbc_driver, NULL, NULL);
}

#else

void sdb_init_odbc(void)
{
	;
}

#endif	/* ODBC */
