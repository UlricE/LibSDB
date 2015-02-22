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

/* These functions are private to libsdb and not part of the published api */

#define _(p) p
#define sdb_snprintf snprintf

extern char *sdb_url;

extern int sdb_debuglevel;
extern void sdb_debug(char *, ...);
extern void sdb_error(char *, ...);
extern int sdb_null(int, char **, void *);
extern int sdb_integer(int, char **, void *);
extern int sdb_strcasecmp(const char *p, const char *q);
extern int sdb_strncasecmp(const char *p, const char *q, size_t n);
extern char *sdb_strdup(const char *s);
extern void *sdb_malloc(size_t);
extern void *sdb_calloc(size_t, size_t);
extern void *sdb_realloc(void *, size_t);
extern void sdb_free(void *);
extern void sdb_chomp(char *);
