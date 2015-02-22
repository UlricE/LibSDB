/*
   Copyright (C) 2000-2005 Ulric Eriksson <ulric@siag.nu>
 
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
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*#define DEBUG*/

#define error(p) {perror(p);return;}

struct conn_info {
	int sock;
};

static void readdata(int fd, char *b, int n)
{
	int i, j;
#ifdef DEBUG
printf("readdata(%d, %p, %d)\n", fd, b, n);
#endif
	for (i = 0; i < n; i += j) {
		j = read(fd, b+i, n-i);
		if (j < 0) error("Error reading data");
	}
}

static void writedata(int fd, char *b, int n)
{
	int i, j;
#ifdef DEBUG
printf("writedata(%d, %p, %d)\n", fd, b, n);
#endif
	for (i = 0; i < n; i += j) {
		j = write(fd, b+i, n-i);
		if (j < 0) error("Error writing data");
	}
}

static void writestring(int fd, char *b)
{
#ifdef DEBUG
printf("writestring(%d, %s)\n", fd, b);
#endif
	writedata(fd, b, strlen(b));
}

/* read a single char */
static int readchar(int fd)
{
	char b[1];
#ifdef DEBUG
printf("readchar(%d)\n", fd);
#endif
	readdata(fd, b, 1);
#ifdef DEBUG
printf("readchar => '%c'\n", b[0]);
#endif
	return b[0];
}

/* read a number terminated by non-digit */
static int readno(int fd)
{
	int c, no = 0;

#ifdef DEBUG
printf("readno(%d)\n", fd);
#endif
	for (;;) {
		c = readchar(fd);
		if (!isdigit(c)) break;
		no = 10*no + c-'0';
	}
	return no;
}

/*
Open a connection to the server, but don't send anything. Return the
socket or -1 for failure.
*/
static int sdbd_connect(char *url)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char *p, host[1024];
	int port, sockfd;

#ifdef DEBUG
printf("sdbd_connect(%s)\n", url);
#endif

	p = strstr(url, ":url=");
	if (p == NULL) {
		fprintf(stderr, "No url\n");
		return -1;
	}
	strncpy(host, url, p-url);
	host[p-url] = '\0';
	url = p+5;
	p = strchr(host, ':');
	if (p == NULL) {
		fprintf(stderr, "No port\n");
		return -1;
	}
	*p = '\0';
	port = atoi(p+1);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Error opening socket\n");
		return -1;
	}
	server = gethostbyname(host);
	if (server == NULL) {
		fprintf(stderr, "Error, no such host\n");
		return -1;
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *) &serv_addr.sin_addr.s_addr,
	       (char *) server->h_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd, (struct sockaddr *)&serv_addr,
		    sizeof(serv_addr)) < 0) {
		fprintf(stderr, "Error connecting\n");
		close(sockfd);
		return -1;
	}
	return sockfd;
}

static int sdbd_try(char *url)
{
	char *p;
	int sockfd = sdbd_connect(url);

#ifdef DEBUG
printf("sdbd_try(%s)\n", url);
#endif
	if (sockfd == -1) return -1;

	p = strstr(url, ":url=");
	p += 5;
	writestring(sockfd, p);
	readchar(sockfd);
	return sockfd;
}

/*
Open a connection and return a pointer to a conn_info structure.
NULL for failure.
*/
void *sdbd_open(char *url)
{
	char *p;
	struct conn_info *ci;
	int c, sockfd = sdbd_connect(url);

#ifdef DEBUG
printf("sdbd_open(%s)\n", url);
#endif
	if (sockfd == -1) return NULL;

	p = strstr(url, ":url=");
	p += 5;
	writestring(sockfd, "sdb_open");
	readchar(sockfd);
	writestring(sockfd, p);
	c = readchar(sockfd);
#ifdef DEBUG
printf("c = '%c'\n", c);
#endif
	if (c == '+') {
		readchar(sockfd);
	} else {
		close(sockfd);
		sockfd = -1;
	}
	ci = malloc(sizeof *ci);
	if (ci == NULL) {
		fprintf(stderr, "Can't allocate\n");
		close(sockfd);
		return NULL;
	}
	ci->sock = sockfd;
#ifdef DEBUG
printf("sdbd_open => %d\n", ci->sock);
#endif
	return ci;
}

int sdbd_close(void *db)
{
	struct conn_info *ci = db;
#ifdef DEBUG
printf("sdbd_close(%p)\n", db);
#endif
	if (ci) {
#ifdef DEBUG
printf("ci = <%d>\n", ci->sock);
#endif
		close(ci->sock);
		free(ci);
	}
	return 0;
}

/*
If pdb is not NULL but sock is -1, ignore sock (old protocol).
Connect again but remember to close.
*/
int sdbd_driver(void *pdb, char *url, char *query,
	int (*callback)(int, char **, void *), void *closure)
{
	struct conn_info *ci = pdb;
	int i, sockfd, nrow, ncol, colsize;
	char b[4096];
	char **coldata;

#ifdef DEBUG
printf("sdbd_driver(%p, %s, %s, %p, %p)\n", pdb, url, query, callback, closure);
#endif
	ci = pdb;
	if (pdb == NULL || ci->sock == -1) {
		sockfd = sdbd_try(url);
	} else {
		sockfd = ci->sock;
	}
	if (sockfd == -1) {
		fprintf(stderr, "Invalid socket\n");
		return -1;
	}

	writestring(sockfd, query);
	nrow = 0;
	while ((ncol = readno(sockfd)) > 0) {
		coldata = malloc(ncol*sizeof *coldata);
		if (coldata == NULL) {
			fprintf(stderr, "Can't allocate %ld bytes\n",
				(long)ncol*sizeof *coldata);
			return -1;
		}
		for (i = 0; i < ncol; i++) {
			colsize = readno(sockfd);
			memset(b, 0, sizeof b);
			readdata(sockfd, b, colsize);
			coldata[i] = malloc(strlen(b)+1);
			if (coldata[i] == NULL) {
				fprintf(stderr, "Can't allocate %ld bytes\n",
					(long)strlen(b)+1);
				return -1;
			}
			strcpy(coldata[i], b);
		}
		(*callback)(ncol, coldata, closure);
		for (i = 0; i < ncol; i++) free(coldata[i]);
		free(coldata);
		nrow++;
	}
	if (pdb == NULL || ci->sock == -1) close(sockfd);

	return nrow;
}

