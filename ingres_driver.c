/*
   Copyright (C) 2008  Ulric Eriksson <ulric@siag.nu>

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

#ifdef HAVE_INGRES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iiapi.h>
#include "common.h"
#include "sdb.h"

static void ingres_wait(IIAPI_GENPARM *genParm)
{
	IIAPI_WAITPARM waitParm = { -1 };
	while (genParm->gp_completed == FALSE)
		IIapi_wait(&waitParm);
}

static void ingres_check_status(IIAPI_STATUS status)
{
	sdb_debug("\tstatus = %s",
	       (status == IIAPI_ST_SUCCESS) ? "IIAPI_ST_SUCCESS" :
	       (status == IIAPI_ST_MESSAGE) ? "IIAPI_ST_MESSAGE" :
	       (status == IIAPI_ST_WARNING) ? "IIAPI_ST_WARNING" :
	       (status == IIAPI_ST_NO_DATA) ? "IIAPI_ST_NO_DATA" :
	       (status == IIAPI_ST_ERROR) ? "IIAPI_ST_ERROR" :
	       (status == IIAPI_ST_FAILURE) ? "IIAPI_ST_FAILURE" :
	       (status == IIAPI_ST_NOT_INITIALIZED) ? "IIAPI_ST_NOT_INITIALIZED" :
	       (status == IIAPI_ST_INVALID_HANDLE) ? "IIAPI_ST_INVALID_HANDLE" :
	       (status == IIAPI_ST_OUT_OF_MEMORY) ? "IIAPI_ST_OUT_OF_MEMORY" :
	       "(unknown status)");
}

static void ingres_check_error(IIAPI_GENPARM * genParm)
{
	IIAPI_GETEINFOPARM getErrParm;
	char type[33];

	ingres_check_status(genParm->gp_status);

	if (!genParm->gp_errorHandle)
		return;
	getErrParm.ge_errorHandle = genParm->gp_errorHandle;

	for (;;) {
		IIapi_getErrorInfo(&getErrParm);

		if (getErrParm.ge_status != IIAPI_ST_SUCCESS)
			break;

		switch (getErrParm.ge_type) {
		case IIAPI_GE_ERROR:
			strcpy(type, "ERROR");
			break;

		case IIAPI_GE_WARNING:
			strcpy(type, "WARNING");
			break;

		case IIAPI_GE_MESSAGE:
			strcpy(type, "USER MESSAGE");
			break;

		default:
			sprintf(type, "unknown error type: %d",
				getErrParm.ge_type);
			break;
		}

		sdb_debug("\tError Info: %s '%s' 0x%x: %s",
		       type, getErrParm.ge_SQLSTATE,
		       getErrParm.ge_errorCode,
		       getErrParm.ge_message ? getErrParm.
		       ge_message : "NULL");
	}

	return;
}

/* take a whatever and return a freshly allocated string, caller must free */
static char *ingres_convert(IIAPI_DESCRIPTOR *gd_descriptor, IIAPI_DATAVALUE *gc_columnData)
{
	IIAPI_CONVERTPARM convertParm;
	char b[1024];
	long long_value;
	double double_value;

	switch (gd_descriptor->ds_dataType) {
	case IIAPI_TXT_TYPE:
	case IIAPI_VBYTE_TYPE:
	case IIAPI_VCH_TYPE:
		gc_columnData->dv_length = *((II_INT2 *) gc_columnData->dv_value);
	case IIAPI_BYTE_TYPE:
	case IIAPI_CHA_TYPE:
	case IIAPI_CHR_TYPE:
	case IIAPI_LOGKEY_TYPE:
	case IIAPI_TABKEY_TYPE:
		if (sdb_debuglevel) sdb_debug("No need to convert\n");
		return sdb_strdup(gc_columnData->dv_value);
		break;
	case IIAPI_INT_TYPE:
		switch (gc_columnData->dv_length) {
		case 1:
			long_value = (long) *((II_INT1 *) gc_columnData->dv_value);
			break;
		case 2:
			long_value = (long) *((II_INT2 *) gc_columnData->dv_value);
			break;
		case 4:
			long_value = (long) *((II_INT4 *) gc_columnData->dv_value);
			break;
		default:
			if (sdb_debuglevel) sdb_debug("Bogus size %d",
			       gc_columnData->dv_length);
			long_value = 0;
		}
		sprintf(b, "%ld", long_value);
		return sdb_strdup(b);
		break;
	case IIAPI_FLT_TYPE:
		switch (gc_columnData->dv_length) {
		case 4:
			double_value = (double) *((II_FLOAT4 *) gc_columnData->dv_value);
			break;
		case 8:
			double_value = (double) *((II_FLOAT8 *) gc_columnData->dv_value);
			break;
		default:
			if (sdb_debuglevel) sdb_debug("Bogus size %d",
			       gc_columnData->dv_length);
			double_value = 0;
		}
		sprintf(b, "%f", double_value);
		return sdb_strdup(b);
		break;
	default:
		if (sdb_debuglevel) sdb_debug("Should convert type to char");
		convertParm.cv_srcDesc.ds_dataType = gd_descriptor->ds_dataType;
		convertParm.cv_srcDesc.ds_nullable = gd_descriptor->ds_nullable;
		convertParm.cv_srcDesc.ds_length = gd_descriptor->ds_length;
		convertParm.cv_srcDesc.ds_precision = gd_descriptor->ds_precision;
		convertParm.cv_srcDesc.ds_scale = gd_descriptor->ds_scale;
		convertParm.cv_srcDesc.ds_columnType = gd_descriptor->ds_columnType;
		convertParm.cv_srcDesc.ds_columnName = gd_descriptor->ds_columnName;

		convertParm.cv_srcValue.dv_null = gc_columnData->dv_null;
		convertParm.cv_srcValue.dv_length = gc_columnData->dv_length;
		convertParm.cv_srcValue.dv_value = gc_columnData->dv_value;

		convertParm.cv_dstDesc.ds_dataType = IIAPI_CHA_TYPE;
		convertParm.cv_dstDesc.ds_nullable = FALSE;
		convertParm.cv_dstDesc.ds_length = sizeof(b) - 1;
		convertParm.cv_dstDesc.ds_precision = 0;
		convertParm.cv_dstDesc.ds_scale = 0;
		convertParm.cv_dstDesc.ds_columnType = IIAPI_COL_TUPLE;
		convertParm.cv_dstDesc.ds_columnName = NULL;

		convertParm.cv_dstValue.dv_null = FALSE;
		convertParm.cv_dstValue.dv_length = convertParm.cv_dstDesc.ds_length;
		convertParm.cv_dstValue.dv_value = b;

		IIapi_convertData(&convertParm);
		if (convertParm.cv_status != IIAPI_ST_SUCCESS) {
			ingres_check_status(convertParm.cv_status);
		}
		b[convertParm.cv_dstValue.dv_length] = '\0';
		return sdb_strdup(b);
	}
}

#if 0
static void ingres_term(void)
{
	IIAPI_TERMPARM termParm;

	if (sdb_debuglevel) sdb_debug("ingres_term: shutting down API");
	IIapi_terminate(&termParm);

	return;
}
#endif

static void *sdb_ingres_open(char *url)
{
	IIAPI_CONNPARM connParm;

	if (sdb_debuglevel) sdb_debug("sdb_ingres_open(%s)", url);
	connParm.co_genParm.gp_callback = NULL;
	connParm.co_genParm.gp_closure = NULL;
	connParm.co_connHandle = NULL;
	connParm.co_tranHandle = NULL;
	connParm.co_timeout = -1;
	connParm.co_username = sdb_url_value(url, "uid");
	connParm.co_password = sdb_url_value(url, "pwd");
	connParm.co_target = sdb_url_value(url, "db");

	if (!connParm.co_target) {
		sdb_debug("No db in '%s'\n", url);
		return NULL;
	}
	IIapi_connect(&connParm);
	ingres_wait(&connParm.co_genParm);
	return connParm.co_connHandle;
}

static int sdb_ingres_close(void *db)
{
	IIAPI_DISCONNPARM disconnParm;

	if (sdb_debuglevel) sdb_debug("sdb_ingres_close(%p)", db);

	disconnParm.dc_genParm.gp_callback = NULL;
	disconnParm.dc_genParm.gp_closure = NULL;
	disconnParm.dc_connHandle = db;

	IIapi_disconnect(&disconnParm);
	ingres_wait(&disconnParm.dc_genParm);
	return 0;
}

/* Parameters:
   pdb = pointer to previously opened database connection, or NULL
   url = url string
   query = query string
   callback = function to send the data to
   closure = pointer to data passed from application
*/
static int ingres_driver(void *pdb, char *url, char *query,
        int (*callback)(int, char **, void *), void *closure)
{
	II_PTR connHandle = NULL;
	II_PTR tranHandle = NULL;
	IIAPI_COMMITPARM commitParm;

	IIAPI_QUERYPARM queryParm;
	IIAPI_CLOSEPARM closeParm;
	IIAPI_GETDESCRPARM getDescrParm;
	IIAPI_GETCOLPARM getColParm;
	IIAPI_DATAVALUE DataBuffer[1];	/* what's this? */
	char var1[1024];
	II_PTR stmtHandle = NULL;
	int row = 0, col = 0, ncols = 0;
	char **cols;

	if (sdb_debuglevel) sdb_debug("ingres_driver(%p, %s, %s, %p, %p)",
				pdb, url, query, callback, closure);
	if (pdb == NULL) connHandle = sdb_ingres_open(url);
	else connHandle = pdb;

	queryParm.qy_genParm.gp_callback = NULL;
	queryParm.qy_genParm.gp_closure = NULL;
	queryParm.qy_connHandle = connHandle;
	queryParm.qy_queryType = IIAPI_QT_QUERY;
	queryParm.qy_queryText = query;
	queryParm.qy_parameters = FALSE;
	queryParm.qy_tranHandle = tranHandle;
	queryParm.qy_stmtHandle = NULL;

	IIapi_query(&queryParm);
	ingres_wait(&queryParm.qy_genParm);

	if (queryParm.qy_genParm.gp_status != IIAPI_ST_SUCCESS)
		ingres_check_error(&queryParm.qy_genParm);

	tranHandle = queryParm.qy_tranHandle;
	stmtHandle = queryParm.qy_stmtHandle;

	if (sdb_debuglevel) sdb_debug("Get query result descriptors");
	getDescrParm.gd_genParm.gp_callback = NULL;
	getDescrParm.gd_genParm.gp_closure = NULL;
	getDescrParm.gd_stmtHandle = queryParm.qy_stmtHandle;
	getDescrParm.gd_descriptorCount = 0;
	getDescrParm.gd_descriptor = NULL;
	IIapi_getDescriptor(&getDescrParm);
	ingres_wait(&getDescrParm.gd_genParm);
	if (getDescrParm.gd_genParm.gp_status == IIAPI_ST_SUCCESS) {
		ncols = getDescrParm.gd_descriptorCount;
		if (sdb_debuglevel) {
			sdb_debug("Number of columns returned = %d", ncols);
			for (col = 0; col < getDescrParm.gd_descriptorCount; col++) {
				sdb_debug("Column %d is %s", col,
		       			getDescrParm.gd_descriptor[col].ds_columnName);
			}
		}

		if (sdb_debuglevel) sdb_debug("Get results");
		cols = sdb_malloc(ncols * sizeof *cols);
		getColParm.gc_genParm.gp_callback = NULL;
		getColParm.gc_genParm.gp_closure = NULL;
		/* only get one row, because we don't know what types there are */
		getColParm.gc_rowCount = 1;
		/* same with columns */
		getColParm.gc_columnCount = 1;
		getColParm.gc_rowsReturned = 0;
		getColParm.gc_columnData = DataBuffer;	/* what's this? */
		getColParm.gc_columnData[0].dv_value = var1;	/* and this? */
		getColParm.gc_stmtHandle = queryParm.qy_stmtHandle;
		getColParm.gc_moreSegments = 0;
		row = col = 0;
		for (;;) {
			IIapi_getColumns(&getColParm);
			ingres_wait(&getColParm.gc_genParm);
			if (getColParm.gc_genParm.gp_status >= IIAPI_ST_NO_DATA)
				break;
			var1[DataBuffer[0].dv_length] = '\0';
			if (sdb_debuglevel) sdb_debug("Datatype: %d",
		       	getDescrParm.gd_descriptor[col].ds_dataType);
			cols[col] = ingres_convert(&(getDescrParm.gd_descriptor[col]), &(getColParm.gc_columnData[0]));
			if (sdb_debuglevel) sdb_debug("\tConverted value: [%d,%d]: %s = %s",
				row, col, getDescrParm.gd_descriptor[col].ds_columnName, cols[col]);

			col++;
			if (col >= ncols) {
				(*callback)(ncols, cols, closure);
				while (col) {
					col--;
					sdb_free(cols[col]);
				}
				row++;
			}
		}
		sdb_free(cols);
		if (sdb_debuglevel) sdb_debug("We found %d rows", row);
	} else if (getDescrParm.gd_genParm.gp_status != IIAPI_ST_NO_DATA) {
		ingres_check_error(&getDescrParm.gd_genParm);
	}

	closeParm.cl_genParm.gp_callback = NULL;
	closeParm.cl_genParm.gp_closure = NULL;
	closeParm.cl_stmtHandle = queryParm.qy_stmtHandle;

	IIapi_close(&closeParm);
	ingres_wait(&closeParm.cl_genParm);

	if (closeParm.cl_genParm.gp_status != IIAPI_ST_SUCCESS)
		ingres_check_error(&closeParm.cl_genParm);

	/* Commit transaction */
	commitParm.cm_genParm.gp_callback = NULL;
	commitParm.cm_genParm.gp_closure = NULL;
	commitParm.cm_tranHandle = tranHandle;
	IIapi_commit(&commitParm);
	ingres_wait(&commitParm.cm_genParm);

	if (pdb == NULL) sdb_ingres_close(connHandle);

// There's no good place to put this
//	ingres_term();

	return row;
}

void sdb_init_ingres(void)
{
	IIAPI_INITPARM initParm;

	if (sdb_debuglevel) sdb_debug("sdb_init_ingres()");
	initParm.in_version = IIAPI_VERSION_1;
	initParm.in_timeout = -1;
	IIapi_initialize(&initParm);

	sdb_register_driver("ingres", ingres_driver,
		sdb_ingres_open, sdb_ingres_close);
}

#else

void sdb_init_ingres(void)
{
	;
}

#endif	/* INGRES */
