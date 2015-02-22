/*
   Copyright (C) 2001-2008  Ulric Eriksson <ulric@siag.nu>
 
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
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sdb.h"
#include "common.h"

static int foreground = 0;

static void writedata(int fd, char *b, int n)
{
	int i, j;
	for (i = 0; i < n; i += j) {
		j = write(fd, b+i, n-i);
		if (j < 0) sdb_error("Error writing data");
	}
}

static void writestring(int fd, char *b)
{
	writedata(fd, b, strlen(b));
}

static void waitforchild(int i)
{
	if (sdb_debuglevel) sdb_debug("waitforchild()");
	waitpid(-1, NULL, WNOHANG);
	signal(SIGCHLD, waitforchild);
}

static void chomp(char *p)
{
	int i;
	for (i = 0; p[i]; i++) {
		if (p[i] == '\r' || p[i] == '\n') {
			p[i] = '\0';
			return;
		}
	}
}

static int db_callback(int n, char **p, void *closure)
{
	int i;
	int fd = *((int *)closure);
	char b[10];

	if (!n) return 0;
	sprintf(b, "%d ", n);
	writestring(fd, b);		/* number of columns */
	for (i = 0; i < n; i++) {
		char *q = p[i];
		if (q == NULL) q = "";
		if (sdb_debuglevel > 1) sdb_debug("%s ", q);
		sprintf(b, "%ld ", (long)strlen(q));
		writestring(fd, b);	/* column width */
		writestring(fd, q);	/* column */
	}
	if (sdb_debuglevel > 1) sdb_debug("\n");
	return 0;
}

static int doquery(int sock, char *u)
{
	int n;
	char q[4096];

	if (sdb_debuglevel) sdb_debug("doquery(%d) begin", sock);
	n = read(sock, q, sizeof q);
	if (n < 0) sdb_error("Can't read query");
	q[n] = '\0';
	if (sdb_debuglevel) sdb_debug("'%s'\n", q);

	if (sdb_debuglevel) sdb_debug("calling sdb_query");

	n = sdb_query(u, q, db_callback, &sock);
	writedata(sock, "0", 2);		/* The End */
	if (sdb_debuglevel) sdb_debug("doquery() end");

	return n;
}

static void dourl(int sock)
{
	int n;
	char u[1024], *db;

	if (sdb_debuglevel) sdb_debug("dourl(%d) begin", sock);
	n = read(sock, u, sizeof u);
	if (n < 0) sdb_error("Can't read URL");
	u[n] = '\0';
	chomp(u);
	if (sdb_debuglevel) sdb_debug("'%s'\n", u);
	writedata(sock, " ", 1);

	if (!strcmp(u, "sdb_open")) {
		n = read(sock, u, sizeof u);
		if (n < 0) sdb_error("Can't read URL to open");
		u[n] = '\0';
		if (sdb_debuglevel) sdb_debug("'%s'\n", u);
		chomp(u);
		if (sdb_debuglevel) sdb_debug("sdb_open(%s)", u);
		db = sdb_open(u);
		if (db == NULL) {
			writedata(sock, "0", 2);
		} else {
			writedata(sock, "+0", 2);
		}
		while (doquery(sock, u) != -1);
		sdb_close(db);
	} else {
		doquery(sock, u);
	}
	if (sdb_debuglevel) sdb_debug("dourl() end");
}

static void usage(void)
{
	printf("usage: sdbd [-df] port\n");
	exit(0);
}

static void background(void)
{
	int childpid;
	if ((childpid = fork()) < 0) {
		sdb_error("Can't fork");
	} else {
		if (childpid > 0) exit(0);
	}
	setsid();
}

static int options(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "df")) != EOF) {
		switch (c) {
		case 'd':
			sdb_debuglevel++;
			break;
		case 'f':
			foreground = 1;
			break;
		default:
			usage();
		}
	}
	return optind;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, port, pid;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int one = 1;
	int n = options(argc, argv);
	argc -= n;
	argv += n;

	if (argc < 1) {
		usage();
	}

	if (!foreground) background();

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) sdb_error("Error opening socket");
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	port = atoi(argv[0]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof one);
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		sdb_error("Error on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	signal(SIGCHLD, waitforchild);
	sdb_init();
	for (;;) {
		if (sdb_debuglevel) sdb_debug("accept(%d, ...)", sockfd);
		newsockfd = accept(sockfd,
				   (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd == -1) {
			if (sdb_debuglevel && errno != EINTR) {
				sdb_debug("Error on accept");
				perror("accept");
			}
			continue;
		}
		pid = fork();
		if (pid < 0) sdb_error("Error on fork");
		if (pid == 0) {
#if 1
			close(sockfd);
#endif
			dourl(newsockfd);
			exit(0);
		} else {
			close(newsockfd);
		}
	}
	return 0;
}
