/*
A simple demo program for sdbd. Reads SQL from stdin and prints
results on stdout. Unlike sdb_client, this program does not use libsdb.
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

extern int sdbd_driver(void *, char *, char *,
		int (*)(int, char **, void *), void *);

static void chomp(char *p)
{
	if ((p = strchr(p, '\n'))) *p = '\0';
}

static int db_callback(int n, char **p, void *closure)
{
	int i;

	if (!n) return 0;
	for (i = 0; i < n; i++) {
		printf("%s ", p[i]);
	}
	printf("\n");
	return 0;
}

int main(int argc, char *argv[])
{
	char b[4096];
	char url[1024];

	if (argc < 4) {
		fprintf(stderr, "usage: %s hostname port url\n", argv[0]);
		exit(0);
	}
	sprintf(url, "%s:%s:url=%s", argv[1], argv[2], argv[3]);
	for (;;) {
		printf("sdbd> ");
		fflush(stdout);
		if (fgets(b, sizeof b, stdin) == NULL) break;
		chomp(b);
		sdbd_driver(NULL, url, b, db_callback, NULL);
	}
	return 0;
}
