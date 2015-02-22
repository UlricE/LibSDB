/*
A simple demo program for libsdb. Reads SQL from stdin and prints
results on stdout.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "sdb.h"

static int persistent = 0;
static char *separator = " ";
static char *command = NULL;

static int db_callback(int n, char **p, void *closure)
{
	int i;

	if (!n) return 0;
	printf("%s", p[0]);
	for (i = 1; i < n; i++) {
		printf("%s%s", separator, p[i]);
	}
	printf("\n");
	return 0;
}

static void usage(void)
{
	printf("usage: sdb_client [-d] [-p] [-s separator] [-c command] url\n");
	exit(0);
}

static int options(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "c:dps:")) != EOF) {
		switch (c) {
		case 'c':
			command = optarg;
			break;
		case 'd':
			sdb_debuglevel++;
			break;
		case 'p':
			persistent = 1;
			break;
		case 's':
			separator = optarg;
			break;
		default:
			usage();
		}
	}
	return optind;
}

int main(int argc, char **argv)
{
	char b[4096];
	char *url, *db;
	int n = options(argc, argv);

	argc -= n;
	argv += n;

	if (argc < 1) usage();

	url = argv[0];
	if (!url) {
		printf("Usage: sdb_client [options] url\n");
		exit(0);
	}

	if (persistent) db = sdb_open(url);
	else db = url;
	if (db == NULL) {
		sdb_debug("Can't open %s", url);
		return EXIT_FAILURE;
	}
	if (command) {
		sdb_query(db, command, db_callback, NULL);
		return 0;
	}

	for (;;) {
		if (ttyname(0)) {
			printf("sdb> ");
			fflush(stdout);
		}
		if (!fgets(b, sizeof b, stdin)) break;
		sdb_chomp(b);
		if (b[0] == '!') {
			system(b+1);
		} else {
			n = sdb_query(db, b, db_callback, NULL);
			if (ttyname(0)) {
				printf("Return code: %d\n", n);
			}
		}
	}
	if (persistent) sdb_close(db);
	return 0;
}
