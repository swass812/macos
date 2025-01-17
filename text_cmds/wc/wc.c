/*
 * Copyright (c) 1980, 1987, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1980, 1987, 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)wc.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/usr.bin/wc/wc.c,v 1.21 2004/12/27 22:27:56 josef Exp $");

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

/* We allocte this much memory statically, and use it as a fallback for
  malloc failure, or statfs failure.  So it should be small, but not
  "too small" */
#define SMALL_BUF_SIZE (1024 * 8)

uintmax_t tlinect, twordct, tcharct;
int doline, doword, dochar, domulti;

static int	cnt(const char *);
static void	usage(void);

int
main(int argc, char *argv[])
{
	int ch, errors, total;

	(void) setlocale(LC_CTYPE, "");

	while ((ch = getopt(argc, argv, "clmw")) != -1)
		switch((char)ch) {
		case 'l':
			doline = 1;
			break;
		case 'w':
			doword = 1;
			break;
		case 'c':
			dochar = 1;
			domulti = 0;
			break;
		case 'm':
			domulti = 1;
			dochar = 0;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;
	argc -= optind;

	/* Wc's flags are on by default. */
	if (doline + doword + dochar + domulti == 0)
		doline = doword = dochar = 1;

	errors = 0;
	total = 0;
	if (!*argv) {
		if (cnt((char *)NULL) != 0)
			++errors;
		else
			(void)printf("\n");
	}
	else do {
		if (cnt(*argv) != 0)
			++errors;
		else
			(void)printf(" %s\n", *argv);
		++total;
	} while(*++argv);

	if (total > 1) {
		if (doline)
			(void)printf(" %7ju", tlinect);
		if (doword)
			(void)printf(" %7ju", twordct);
		if (dochar || domulti)
			(void)printf(" %7ju", tcharct);
		(void)printf(" total\n");
	}
#ifdef __APPLE__
	if (ferror(stdout) != 0 || fflush(stdout) != 0)
		err(1, "stdout");
#endif
	exit(errors == 0 ? 0 : 1);
}

static int
cnt(const char *file)
{
	struct stat sb;
	struct statfs fsb;
	uintmax_t linect, wordct, charct;
	int fd, len, warned;
	int stat_ret;
	size_t clen;
	short gotsp;
	u_char *p;
	static u_char small_buf[SMALL_BUF_SIZE];
	static u_char *buf = small_buf;
	static off_t buf_size = SMALL_BUF_SIZE;
	wchar_t wch;
	mbstate_t mbs;

	linect = wordct = charct = 0;
	if (file == NULL) {
		file = "stdin";
		fd = STDIN_FILENO;
	} else {
		if ((fd = open(file, O_RDONLY, 0)) < 0) {
			warn("%s: open", file);
			return (1);
		}
	}

	if (fstatfs(fd, &fsb)) {
	    fsb.f_iosize = SMALL_BUF_SIZE;
	}
	if (fsb.f_iosize != buf_size) {
	    if (buf != small_buf) {
		free(buf);
	    }
	    if (fsb.f_iosize == SMALL_BUF_SIZE || !(buf = malloc(fsb.f_iosize))) {
		buf = small_buf;
		buf_size = SMALL_BUF_SIZE;
	    } else {
		buf_size = fsb.f_iosize;
	    }
	}

	if (doword || (domulti && MB_CUR_MAX != 1))
		goto word;
	/*
	 * Line counting is split out because it's a lot faster to get
	 * lines than to get words, since the word count requires some
	 * logic.
	 */
	if (doline) {
		while ((len = read(fd, buf, buf_size))) {
			if (len == -1) {
				warn("%s: read", file);
				(void)close(fd);
				return (1);
			}
			charct += len;
			for (p = buf; len--; ++p)
				if (*p == '\n')
					++linect;
		}
		tlinect += linect;
		(void)printf(" %7ju", linect);
		if (dochar) {
			tcharct += charct;
			(void)printf(" %7ju", charct);
		}
		(void)close(fd);
		return (0);
	}
	/*
	 * If all we need is the number of characters and it's a
	 * regular file, just stat the puppy.
	 */
	if (dochar || domulti) {
		if (fstat(fd, &sb)) {
			warn("%s: fstat", file);
			(void)close(fd);
			return (1);
		}
		if (S_ISREG(sb.st_mode)) {
			(void)printf(" %7lld", (long long)sb.st_size);
			tcharct += sb.st_size;
			(void)close(fd);
			return (0);
		}
	}

	/* Do it the hard way... */
word:	gotsp = 1;
	warned = 0;
	memset(&mbs, 0, sizeof(mbs));
	while ((len = read(fd, buf, buf_size)) != 0) {
		if (len == -1) {
			warn("%s: read", file);
			(void)close(fd);
			return (1);
		}
		p = buf;
		while (len > 0) {
			if (!domulti || MB_CUR_MAX == 1) {
				clen = 1;
				wch = (unsigned char)*p;
			} else if ((clen = mbrtowc(&wch, p, len, &mbs)) ==
			    (size_t)-1) {
				if (!warned) {
					errno = EILSEQ;
					warn("%s", file);
					warned = 1;
				}
				memset(&mbs, 0, sizeof(mbs));
				clen = 1;
				wch = (unsigned char)*p;
			} else if (clen == (size_t)-2)
				break;
			else if (clen == 0)
				clen = 1;
			charct++;
			len -= clen;
			p += clen;
			if (wch == L'\n')
				++linect;
			if (iswspace(wch))
				gotsp = 1;
			else if (gotsp) {
				gotsp = 0;
				++wordct;
			}
		}
	}
	if (domulti && MB_CUR_MAX > 1)
		if (mbrtowc(NULL, NULL, 0, &mbs) == (size_t)-1 && !warned)
			warn("%s", file);
	if (doline) {
		tlinect += linect;
		(void)printf(" %7ju", linect);
	}
	if (doword) {
		twordct += wordct;
		(void)printf(" %7ju", wordct);
	}
	if (dochar || domulti) {
		tcharct += charct;
		(void)printf(" %7ju", charct);
	}
	(void)close(fd);
	return (0);
}

static void
usage()
{
	(void)fprintf(stderr, "usage: wc [-clmw] [file ...]\n");
	exit(1);
}
