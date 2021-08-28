#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINES  22
#define MAXCHARS  1024
#define DEFEDITOR "vi"
#define DEFPAGER  "more"
#define VISUAL    "VISUAL"
#define EDITOR    "EDITOR"
#define PAGER     "PAGER"

enum {
	MODE_HEADER,
	MODE_CAT,
	MODE_EDITOR,
	MODE_PAGER
};

/* show usage and exit */
static void
usage(void)
{
	(void)fprintf(stderr, "usage: readme [-cep] [dir]\n");
	exit(1);
}

/* search for a README* file in dir or up in the directory hierarchy; dir is modified */
static char *
readme(char *file, char *dir)
{
	glob_t g;
	int n;
	char *s;

	for (;;) {
		(void)snprintf(file, PATH_MAX, "%s/README*", dir);
		if (glob(file, 0, NULL, &g) != 0) {
			globfree(&g);
			break;
		}
		if (g.gl_pathc > 0) {
			n = snprintf(file, PATH_MAX, "%s", g.gl_pathv[0]);
			file[PATH_MAX - 1] = '\0';
			globfree(&g);
			if (n >= PATH_MAX) {
				errno = ENAMETOOLONG;
				err(1, "%s", file);
			}
			return file;
		}
		globfree(&g);
		if ((s = strrchr(dir, '/')) != NULL) {
			*s = '\0';
		} else {
			break;
		}
	}
	return NULL;
}

/* print the header of file */
static void
header(const char *file)
{
	FILE *fp;
	int nlines, nchars;
	int ch, prev;
	int blank;

	if ((fp = fopen(file, "r")) == NULL)
		err(1, "%s", file);
	prev = '\0';
	blank = 0;
	nchars = nlines = 0;
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n') {
			nlines++;
			if (prev == '\n') {
				blank++;
			}
		}
		nchars++;
		if (blank > 1 || nchars >= MAXCHARS || nlines >= MAXLINES)
			return;
		prev = ch;
		if (iscntrl((unsigned char)ch) && !isspace((unsigned char)ch))
			continue;
		putchar(ch);
	}
}

/* print file */
static void
cat(const char *file)
{
	FILE *fp;
	int ch;

	if ((fp = fopen(file, "r")) == NULL)
		err(1, "%s", file);
	while ((ch = getc(fp)) != EOF) {
		if (iscntrl((unsigned char)ch) && !isspace((unsigned char)ch))
			continue;
		putchar(ch);
	}
}

/* call editor on file */
static void
editor(const char *file)
{
	char *prog;

	if ((prog = getenv(VISUAL)) == NULL)
		if ((prog = getenv(EDITOR)) == NULL)
			prog = DEFEDITOR;
	execlp(prog, prog, file, NULL);
	err(1, "%s", prog);
}

/* call pager on file */
static void
pager(const char *file)
{
	char *prog;

	if ((prog = getenv(PAGER)) == NULL)
		prog = DEFPAGER;
	execlp(prog, prog, file, NULL);
	err(1, "%s", prog);
}

/* readme: print README file for current project */
int
main(int argc, char *argv[])
{
	void (*func)(const char *);
	int ch;
	char dir[PATH_MAX];
	char file[PATH_MAX];

	func = header;
	while ((ch = getopt(argc, argv, "cep")) != -1) {
		switch (ch) {
		case 'c':
			func = cat;
			break;
		case 'e':
			func = editor;
			break;
		case 'p':
			func = pager;
			break;
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 1)
		usage();
	if (argc == 1) {
		if (realpath(*argv, dir) == NULL) {
			err(1, "%s", *argv);
		}
	} else if (getcwd(dir, sizeof(dir)) == NULL) {
		err(1, NULL);
	}
	if (readme(file, dir) == NULL)
		return 1;
	(*func)(file);
	return 0;
}
