/*
   Copyright (C) 2008  Ulric Eriksson <ulric@siag.nu>

   Based on tsql.c from freetds:
   FreeTDS - Library of routines accessing Sybase and Microsoft databases
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Brian Bruns

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

#ifdef HAVE_LIBTDS

# include <sys/time.h>
# include <time.h>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif /* HAVE_LANGINFO_H */

#include <tds.h>
#include <tdsconvert.h>
#include "common.h"
#include "sdb.h"

enum
{
	OPT_VERSION =  0x01,
	OPT_TIMER =    0x02,
	OPT_NOFOOTER = 0x04,
	OPT_NOHEADER = 0x08,
	OPT_QUIET =    0x10
};

struct conn_info {
	TDSSOCKET *tds;
	TDSLOGIN *login;
	TDSCONTEXT *context;
	TDSCONNECTION *connection;
};

#define QUIET (sdb_debuglevel == 0)

static int sdb_handle_message(const TDSCONTEXT * context, TDSSOCKET * tds, TDSMESSAGE * msg)
{
	if (msg->msgno == 0) {
		sdb_debug("%s", msg->message);
		return 0;
	}

	if (msg->msgno != 5701 && msg->msgno != 5703
	    && msg->msgno != 20018) {
		sdb_debug("Msg %d, Level %d, State %d, Server %s, Line %d\n%s",
			msg->msgno, msg->severity, msg->state, msg->server, msg->line_number, msg->message);
	}

	return 0;
}

static int do_query(TDSSOCKET* tds, char *buf, int opt_flags,
		int (*callback)(int, char **, void *), void *closure)
{
	int rows = -1;
	int rc, i;
	TDSCOLUMN *col;
	int ctype;
	CONV_RESULT dres;
	unsigned char *src;
	TDS_INT srclen;
	TDS_INT resulttype;
	struct timeval start, stop;
	char message[128];
	char **p;

	rc = tds_submit_query(tds, buf);
	if (rc != TDS_SUCCEED) {
		sdb_debug("tds_submit_query() failed");
		return -1;
	}

	while ((rc = tds_process_tokens(tds, &resulttype, NULL, TDS_TOKEN_RESULTS)) == TDS_SUCCEED) {
		if (opt_flags & OPT_TIMER) {
			gettimeofday(&start, NULL);
		}
		switch (resulttype) {
		case TDS_ROWFMT_RESULT:
			if (sdb_debuglevel && tds->current_results) {
				for (i = 0; i < tds->current_results->num_cols; i++) {
					sdb_debug("Column %d = '%s'", i, tds->current_results->columns[i]->column_name);
				}
			}
			break;
		case TDS_COMPUTE_RESULT:
		case TDS_ROW_RESULT:
			rows = 0;
			p = sdb_malloc(tds->current_results->num_cols * sizeof *p);

			while ((rc = tds_process_tokens(tds, &resulttype, NULL, TDS_STOPAT_ROWFMT|TDS_RETURN_DONE|TDS_RETURN_ROW|TDS_RETURN_COMPUTE)) == TDS_SUCCEED) {
				if (resulttype != TDS_ROW_RESULT && resulttype != TDS_COMPUTE_RESULT)
					break;

				rows++;

				if (!tds->current_results)
					continue;

				for (i = 0; i < tds->current_results->num_cols; i++) {
					col = tds->current_results->columns[i];
					if (col->column_cur_size < 0) {
						if (sdb_debuglevel)
							sdb_debug("[%d,%d] %s", rows, i, "NULL");
						p[i] = NULL;
						continue;
					}
					ctype = tds_get_conversion_type(col->column_type, col->column_size);

					src = &(tds->current_results->current_row[col->column_offset]);
					if (is_blob_type(col->column_type))
						src = (unsigned char *) ((TDSBLOB *) src)->textvalue;
					srclen = col->column_cur_size;


					if (tds_convert(tds->tds_ctx, ctype, (TDS_CHAR *) src, srclen, SYBVARCHAR, &dres) < 0)
						continue;
					if (sdb_debuglevel)
						sdb_debug("[%d,%d] %s", rows, i, dres.c);
					p[i] = sdb_strdup(dres.c);
					free(dres.c);
				}
				if (sdb_debuglevel)
					sdb_debug("Call the callback here");
				(*callback)(tds->current_results->num_cols, p, closure);
				for (i = 0; i < tds->current_results->num_cols; i++) {
					sdb_free(p[i]);
				}
			}
			sdb_free(p);
			break;
		case TDS_STATUS_RESULT:
			sdb_debug("(return status = %d)", tds->ret_status);
			break;
		default:
			break;
		}

		if (opt_flags & OPT_VERSION) {
			char version[64];
			int line = 0;

			line = tds_version(tds, version);
			if (line) {
				sprintf(message, "using TDS version %s", version);
				tds_client_msg(tds->tds_ctx, tds, line, line, line, line, message);
			}
		}
		if (opt_flags & OPT_TIMER) {
			gettimeofday(&stop, NULL);
			sprintf(message, "Total time for processing %d rows: %ld msecs\n",
				rows, (long) ((stop.tv_sec - start.tv_sec) * 1000) + ((stop.tv_usec - start.tv_usec) / 1000));
			tds_client_msg(tds->tds_ctx, tds, 1, 1, 1, 1, message);
		}
	}
	return rows;
}

static void populate_login(TDSLOGIN *login, char *url)
{
	char *hostname = sdb_url_value(url, "host");
	char *servername = sdb_url_value(url, "server");
	char *username = sdb_url_value(url, "uid");
	char *password = sdb_url_value(url, "pwd");
	char *confile = sdb_url_value(url, "cfg");
	char *portname = sdb_url_value(url, "port");
	int port = 1433;
	const char *locale = NULL;
	char *charset = NULL;

	if (portname) port = atoi(portname);

	setlocale(LC_ALL, "");
	locale = setlocale(LC_ALL, NULL);

#if HAVE_LOCALE_CHARSET
	charset = locale_charset();
#endif
#if HAVE_NL_LANGINFO && defined(CODESET)
	if (!charset)
		charset = nl_langinfo(CODESET);
#endif

	if (locale)
		if (!QUIET) sdb_debug("locale is \"%s\"", locale);
	if (charset) {
		if (!QUIET) sdb_debug("locale charset is \"%s\"", charset);
	} else {
		charset = "ISO-8859-1";
		if (!QUIET) sdb_debug("using default charset \"%s\"", charset);
	}

	/* validate parameters */
	if (!servername && !hostname) {
		sdb_error("Missing server or host");
	}
	if (hostname && !port) {
		sdb_error("Missing port");
	}
	if (!username) {
		sdb_error("Missing user");
	}
	if (!password) {
		sdb_error("Missing password");
	}

	/* all validated, let's do it */

	/* if it's a servername */

	if (servername) {
		tds_set_user(login, username);
		tds_set_app(login, "TSQL");
		tds_set_library(login, "TDS-Library");
		tds_set_server(login, servername);
		tds_set_client_charset(login, charset);
		tds_set_language(login, "us_english");
		tds_set_passwd(login, password);
		if (confile) {
			tds_set_interfaces_file_loc(confile);
		}
		/* else we specified hostname/port */
	} else {
		tds_set_user(login, username);
		tds_set_app(login, "TSQL");
		tds_set_library(login, "TDS-Library");
		tds_set_server(login, hostname);
		tds_set_port(login, port);
		tds_set_client_charset(login, charset);
		tds_set_language(login, "us_english");
		tds_set_passwd(login, password);
	}

	sdb_free(hostname);
	sdb_free(servername);
	sdb_free(username);
	sdb_free(password);
	sdb_free(confile);
	sdb_free(portname);
}

static void *sdb_tds_open(char *url)
{
	struct conn_info *ci = sdb_malloc(sizeof *ci);

	if (sdb_debuglevel) sdb_debug("sdb_tds_open(%s)", url);
	/* grab a login structure */
	ci->login = tds_alloc_login();

	ci->context = tds_alloc_context(NULL);
	if (ci->context->locale && !ci->context->locale->date_fmt) {
		/* set default in case there's no locale file */
		ci->context->locale->date_fmt = strdup("%b %e %Y %I:%M%p");
	}

	ci->context->msg_handler = sdb_handle_message;
	ci->context->err_handler = sdb_handle_message;

	/* process all the command line args into the login structure */
	populate_login(ci->login, url);

	/* Try to open a connection */
	ci->tds = tds_alloc_socket(ci->context, 512);
	tds_set_parent(ci->tds, NULL);
	ci->connection = tds_read_config_info(NULL, ci->login, ci->context->locale);
	if (!ci->connection || tds_connect(ci->tds, ci->connection) == TDS_FAIL) {
		tds_free_socket(ci->tds);
		tds_free_connection(ci->connection);
		sdb_debug("There was a problem connecting to the server");
		sdb_free(ci);
		return NULL;
	}
	tds_free_connection(ci->connection);
	return ci;
}

static int sdb_tds_close(void *db)
{
	struct conn_info *ci = db;

	if (sdb_debuglevel) sdb_debug("sdb_tds_close(%p)", db);

	/* close up shop */
	tds_free_socket(ci->tds);
	tds_free_login(ci->login);
	tds_free_context(ci->context);

	sdb_free(ci);
	return 0;
}

/* Parameters:
   pdb = pointer to previously opened database connection, or NULL
   url = url string
   query = query string
   callback = function to send the data to
   closure = pointer to data passed from application
*/
static int tds_driver(void *pdb, char *url, char *query,
	int (*callback)(int, char **, void *), void *closure)
{
	struct conn_info *ci;
	int opt_flags = 0;
	int rows;

	if (sdb_debuglevel) sdb_debug("tds_driver(%p, %s, %s, %p, %p)",
				pdb, url, query, callback, closure);
	if (pdb == NULL) ci = sdb_tds_open(url);
	else ci = pdb;
	if (ci == NULL) {
		sdb_debug("Can't open database");
		return -1;
	}

	rows = do_query(ci->tds, query, opt_flags, callback, closure);

	if (pdb == NULL) sdb_tds_close(ci);

	return rows;
}

void sdb_init_tds(void)
{
	sdb_register_driver("tds", tds_driver,
		sdb_tds_open, sdb_tds_close);
}

#else

void sdb_init_tds(void)
{
	;
}

#endif	/* HAVE_LIBTDS */
