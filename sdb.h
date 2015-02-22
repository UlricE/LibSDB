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

extern char *sdb_url_value(char *, char *);
extern void sdb_register_driver(char *,
	int (*)(void *, char *, char *, int (*)(int, char **, void *), void *),
	void *(*)(char *), int (*)(void *));
extern int sdb_query(char *, char *, int (*)(int, char **, void *), void *);
extern void sdb_init(void);
extern void *sdbd_open(char *);
extern int sdbd_close(void *);
extern int sdbd_driver(void *, char *, char *,
		int (*)(int, char **, void *), void *);
extern char *sdb_open(char *);
extern int sdb_close(char *);
