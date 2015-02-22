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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "sdb.h"

int sdb_debuglevel = 0;

void sdb_debug(char *fmt, ...)
{
	char b[4096];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(b, sizeof b, fmt, ap);
	fprintf(stderr, "%s\n", b);
	va_end(ap);
}

void sdb_error(char *fmt, ...)
{
	char b[4096];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(b, sizeof b, fmt, ap);
	fprintf(stderr, "%s\n", b);
	va_end(ap);
	exit(EXIT_FAILURE);
}

/* A callback for queries where we don't expect anything interesting */
int sdb_null(int n, char **p, void *closure)
{
        return 0;
}      

/* A callback for queries which return a single integer */
int sdb_integer(int n, char **p, void *closure)
{
        int *i = (int *)closure;

        *i = atoi(p[0]);
        return 0;
}

int sdb_strcasecmp(const char *p, const char *q)
{
	int c;

	while (!(c = toupper(*p)-toupper(*q)) && *p) {
		p++;
		q++;
	}
	return c;
}

int sdb_strncasecmp(const char *p, const char *q, size_t n)
{
	size_t i = 0;
	int c = 0;

	while ((i < n) && !(c = toupper(*p)-toupper(*q)) && *p) {
		p++;
		q++;
		i++;
	}
	return c;
}

static void alloc_fail(void)
{
	sdb_error("Memory allocation failure");
}

char *sdb_strdup(const char *s)
{
	char *p;

	if (s) p = malloc(strlen(s)+1);
	else p = NULL;
	if (p == NULL) alloc_fail();
	else strcpy(p, s);
	return p;
}

void *sdb_malloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (p == NULL) alloc_fail();
	return p;
}

void *sdb_calloc(size_t count, size_t size)
{
	void *p;

	p = calloc(count, size);
	if (p == NULL) alloc_fail();
	return p;
}

void *sdb_realloc(void *r, size_t size)
{
	void *p;

	p = realloc(r, size);
	if (p == NULL) alloc_fail();
	return p;
}

void sdb_free(void *p)
{
	free(p);
}

void sdb_chomp(char *p)
{
	if ((p = strchr(p, '\n'))) *p = '\0';
}
