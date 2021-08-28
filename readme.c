#include <ctype.h>
#include <err.h>
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

/* search for a README* file in dir or up in the directory hierarchy */
static char *
readme(char *file, char *dir)
{
	glob_t g;
	char *s;

	for (;;) {
		(void)snprintf(file, PATH_MAX, "%s/README*", dir);
		if (glob(file, 0, NULL, &g) != 0)
			return NULL;
		if (g.gl_pathc > 0) {
			(void)snprintf(file, PATH_MAX, "%s", g.gl_pathv[0]);
			file[PATH_MAX - 1] = '\0';
			globfree(&g);
			return file;
		} else {
			globfree(&g);
		}
		if ((s = strrchr(dir, '/')) != NULL)
			*s = '\0';
		else
			break;
	}
	return NULL;
}

/* print the header of file */
static void
cat(const char *file, int head)
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
		if (iscntrl((unsigned char)ch) && !isspace((unsigned char)ch))
			continue;
		if (ch == '\n' && head) {
			nlines++;
			if (prev == '\n') {
				blank++;
			}
		}
		if (head) {
			nchars++;
			if (blank > 1 || nchars >= MAXCHARS || nlines >= MAXLINES)
				return;
			prev = ch;
		}
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
}

/* call pager on file */
static void
pager(const char *file)
{
	char *prog;

	if ((prog = getenv(PAGER)) == NULL)
		prog = DEFPAGER;
	execlp(prog, prog, file, NULL);
}

/* readme: print README file for current project */
int
main(int argc, char *argv[])
{
	int mode;
	int ch;
	char dir[PATH_MAX];
	char file[PATH_MAX];

	mode = MODE_HEADER;
	while ((ch = getopt(argc, argv, "cep")) != -1) {
		switch (ch) {
		case 'c':
			mode = MODE_CAT;
			break;
		case 'e':
			mode = MODE_EDITOR;
			break;
		case 'p':
			mode = MODE_PAGER;
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
	switch (mode) {
	case MODE_HEADER:
		cat(file, 1);
		break;
	case MODE_CAT:
		cat(file, 0);
		break;
	case MODE_EDITOR:
		editor(file);
		break;
	case MODE_PAGER:
		pager(file);
		break;
	}
	return 0;
}
