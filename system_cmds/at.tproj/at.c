/*
 *  at.c : Put file into atrun queue
 *  Copyright (C) 1993, 1994 Thomas Koenig
 *
 *  Atrun & Atq modifications
 *  Copyright (C) 1993  David Parsons
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author(s) may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: /usr/local/www/cvsroot/FreeBSD/src/usr.bin/at/at.c,v 1.34 2011/11/06 20:30:21 ed Exp $");

#define _USE_BSD 1

/* System Headers */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#ifndef __FreeBSD__
#include <getopt.h>
#endif
#include <glob.h>
#ifdef __FreeBSD__
#include <locale.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <get_compat.h>
#else  /* !__APPLE */
#define COMPAT_MODE(a,b) (1)
#endif /* __APPLE__ */

/* Local headers */

#include "at.h"
#include "panic.h"
#include "parsetime.h"
#include "pathnames.h"
#include "perm.h"

#define MAIN
#include "privs.h"

/* Macros */

#ifndef ATJOB_DIR
#define ATJOB_DIR _PATH_ATJOBS
#endif

#ifndef LFILE
#define LFILE ATJOB_DIR ".lockfile"
#endif

#ifndef ATJOB_MX
#define ATJOB_MX 255
#endif

#define ALARMC 10 /* Number of seconds to wait for timeout */

#define SIZE 255
#define TIMESIZE 50

enum { ATQ, ATRM, AT, BATCH, CAT };	/* what program we want to run */

/* File scope variables */

static const char *no_export[] = {
    "TERM", "TERMCAP", "DISPLAY", "_"
};
static int send_mail = 0;
static char *atinput = NULL;	/* where to get input from */
static char atqueue = 0;	/* which queue to examine for jobs (atq) */

/* External variables */

extern char **environ;
int fcreated;
char atfile[] = ATJOB_DIR "12345678901234";
char atverify = 0;		/* verify time instead of queuing job */
char *namep;
int posixly_correct;		/* Behave as per POSIX */
/* http://www.opengroup.org/onlinepubs/009695399/utilities/at.html */

/* Function declarations */

static void sigc(int signo);
static void alarmc(int signo);
static char *cwdname(void);
static void writefile(time_t runtimer, char queue);
static void list_jobs(long *, int);
static long nextjob(void);
static time_t ttime(const char *arg);
static int in_job_list(long, long *, int);
static long *get_job_list(int, char *[], int *);

/* Signal catching functions */

static void
sigc(int signo __unused)
{
/* If the user presses ^C, remove the spool file and exit
 */
    if (fcreated)
    {
	PRIV_START
	    unlink(atfile);
	PRIV_END
    }

    _exit(EXIT_FAILURE);
}

static void
alarmc(int signo __unused)
{
    char buf[1024];

    /* Time out after some seconds. */
    strlcpy(buf, namep, sizeof(buf));
    strlcat(buf, ": file locking timed out\n", sizeof(buf));
    write(STDERR_FILENO, buf, strlen(buf));
    sigc(0);
}

/* Local functions */

static char *
cwdname(void)
{
/* Read in the current directory; the name will be overwritten on
 * subsequent calls.
 */
    static char *ptr = NULL;
    static size_t size = SIZE;

    if (ptr == NULL)
	if ((ptr = malloc(size)) == NULL)
	    errx(EXIT_FAILURE, "virtual memory exhausted");

    while (1)
    {
	if (ptr == NULL)
	    panic("out of memory");

	if (getcwd(ptr, size-1) != NULL)
	    return ptr;

	if (errno != ERANGE)
	    perr("cannot get directory");

	free (ptr);
	size += SIZE;
	if ((ptr = malloc(size)) == NULL)
	    errx(EXIT_FAILURE, "virtual memory exhausted");
    }
}

static long
nextjob(void)
{
    long jobno;
    FILE *fid;

    if ((fid = fopen(ATJOB_DIR ".SEQ", "r+")) != NULL) {
	if (fscanf(fid, "%5lx", &jobno) == 1) {
	    rewind(fid);
	    jobno = (1+jobno) % 0xfffff;	/* 2^20 jobs enough? */
	    fprintf(fid, "%05lx\n", jobno);
	}
	else
	    jobno = EOF;
	fclose(fid);
	return jobno;
    }
    else if ((fid = fopen(ATJOB_DIR ".SEQ", "w")) != NULL) {
	fprintf(fid, "%05lx\n", jobno = 1);
	fclose(fid);
	return 1;
    }
    return EOF;
}

static void
writefile(time_t runtimer, char queue)
{
/* This does most of the work if at or batch are invoked for writing a job.
 */
    long jobno;
    char *ap, *ppos, *mailname;
    struct passwd *pass_entry;
    struct stat statbuf;
    int fdes, lockdes, fd2;
    FILE *fp, *fpin;
    struct sigaction act;
    char **atenv;
    int ch;
    mode_t cmask;
    struct flock lock;
    char * oldpwd_str = NULL;

#ifdef __FreeBSD__
    (void) setlocale(LC_TIME, "");
#endif

/* Install the signal handler for SIGINT; terminate after removing the
 * spool file if necessary
 */
    act.sa_handler = sigc;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    sigaction(SIGINT, &act, NULL);

    ppos = atfile + strlen(ATJOB_DIR);

    /* Loop over all possible file names for running something at this
     * particular time, see if a file is there; the first empty slot at any
     * particular time is used.  Lock the file LFILE first to make sure
     * we're alone when doing this.
     */

    PRIV_START

    if ((lockdes = open(LFILE, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR)) < 0)
	perr("cannot open lockfile " LFILE);

    lock.l_type = F_WRLCK; lock.l_whence = SEEK_SET; lock.l_start = 0;
    lock.l_len = 0;

    act.sa_handler = alarmc;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    /* Set an alarm so a timeout occurs after ALARMC seconds, in case
     * something is seriously broken.
     */
    sigaction(SIGALRM, &act, NULL);
    alarm(ALARMC);
    fcntl(lockdes, F_SETLKW, &lock);
    alarm(0);

    if ((jobno = nextjob()) == EOF)
	perr("cannot generate job number");

    sprintf(ppos, "%c%5lx%8lx", queue,
	    jobno, (unsigned long) (runtimer/60));

    for(ap=ppos; *ap != '\0'; ap ++)
	if (*ap == ' ')
	    *ap = '0';

    if (stat(atfile, &statbuf) != 0)
	if (errno != ENOENT)
	    perr("cannot access " ATJOB_DIR);

    /* Create the file. The x bit is only going to be set after it has
     * been completely written out, to make sure it is not executed in the
     * meantime.  To make sure they do not get deleted, turn off their r
     * bit.  Yes, this is a kluge.
     */
    cmask = umask(S_IRUSR | S_IWUSR | S_IXUSR);
    if ((fdes = creat(atfile, O_WRONLY)) == -1)
	perr("cannot create atjob file");

    if ((fd2 = dup(fdes)) <0)
	perr("error in dup() of job file");

    if(fchown(fd2, real_uid, real_gid) != 0)
	perr("cannot give away file");

    PRIV_END

    /* We no longer need suid root; now we just need to be able to write
     * to the directory, if necessary.
     */

    REDUCE_PRIV(DAEMON_UID, DAEMON_GID)

    /* We've successfully created the file; let's set the flag so it
     * gets removed in case of an interrupt or error.
     */
    fcreated = 1;

    /* Now we can release the lock, so other people can access it
     */
    lock.l_type = F_UNLCK; lock.l_whence = SEEK_SET; lock.l_start = 0;
    lock.l_len = 0;
    fcntl(lockdes, F_SETLKW, &lock);
    close(lockdes);

    if((fp = fdopen(fdes, "w")) == NULL)
	panic("cannot reopen atjob file");

    /* Get the userid to mail to, first by trying getlogin(),
     * then from LOGNAME, finally from getpwuid().
     */
    mailname = getlogin();
    if (mailname == NULL)
	mailname = getenv("LOGNAME");

    if ((mailname == NULL) || (mailname[0] == '\0')
	|| (strlen(mailname) >= MAXLOGNAME) || (getpwnam(mailname)==NULL))
    {
	pass_entry = getpwuid(real_uid);
	if (pass_entry != NULL)
	    mailname = pass_entry->pw_name;
    }

    if (atinput != (char *) NULL)
    {
	fpin = freopen(atinput, "r", stdin);
	if (fpin == NULL)
	    perr("cannot open input file");
    }
    fprintf(fp, "#!/bin/sh\n# atrun uid=%ld gid=%ld\n# mail %.*s %d\n",
	(long) real_uid, (long) real_gid, MAXLOGNAME - 1, mailname,
	send_mail);

    /* Write out the umask at the time of invocation
     */
    fprintf(fp, "umask %lo\n", (unsigned long) cmask);

    /* Write out the environment. Anything that may look like a
     * special character to the shell is quoted, except for \n, which is
     * done with a pair of "'s.  Don't export the no_export list (such
     * as TERM or DISPLAY) because we don't want these.
     */
    for (atenv= environ; *atenv != NULL; atenv++)
    {
	int export = 1;
	char *eqp;

	eqp = strchr(*atenv, '=');
	if (ap == NULL)
	    eqp = *atenv;
	else
	{
	    size_t i;

	    if(strncmp(*atenv, "OLDPWD", (size_t) (eqp-*atenv)) == 0) {
		oldpwd_str = *atenv;
	    }
	    if (!posixly_correct) {
		/* Test 891 expects TERM, etc. to show up in "at" env
		   so exclude them only when not posixly_correct */
		for (i=0; i<sizeof(no_export)/sizeof(no_export[0]); i++)
		{
		    export = export
			&& (strncmp(*atenv, no_export[i],
				(size_t) (eqp-*atenv)) != 0);
		}
	    }
	    eqp++;
	}

	if (export)
	{
	    fwrite(*atenv, sizeof(char), eqp-*atenv, fp);
	    for(ap = eqp;*ap != '\0'; ap++)
	    {
		if (*ap == '\n')
		    fprintf(fp, "\"\n\"");
		else
		{
		    if (!isalnum(*ap)) {
			switch (*ap) {
			  case '%': case '/': case '{': case '[':
			  case ']': case '=': case '}': case '@':
			  case '+': case '#': case ',': case '.':
			  case ':': case '-': case '_':
			    break;
			  default:
			    fputc('\\', fp);
			    break;
			}
		    }
		    fputc(*ap, fp);
		}
	    }
	    fputs("; export ", fp);
	    fwrite(*atenv, sizeof(char), eqp-*atenv -1, fp);
	    fputc('\n', fp);

	}
    }
    /* Cd to the directory at the time and write out all the
     * commands the user supplies from stdin.
     */
    fprintf(fp, "cd ");
    for (ap = cwdname(); *ap != '\0'; ap++)
    {
	if (*ap == '\n')
	    fprintf(fp, "\"\n\"");
	else
	{
	    if (*ap != '/' && !isalnum(*ap))
		fputc('\\', fp);

	    fputc(*ap, fp);
	}
    }
    /* Test cd's exit status: die if the original directory has been
     * removed, become unreadable or whatever
     */
    fprintf(fp, " || {\n\t echo 'Execution directory "
	        "inaccessible' >&2\n\t exit 1\n}\n");

    /* Put OLDPWD back, since the cd has set it	*/
    /* Although this is added to fix conformance test at.ex 891, it seems like	*/
    /* the right thing to do always, so the code is not posix_pedantic only	*/
    if (oldpwd_str) {
	    fprintf(fp, "%s; export OLDPWD\n", oldpwd_str);
    } else {
	    fprintf(fp, "unset OLDPWD\n");
    }

    while((ch = getchar()) != EOF)
	fputc(ch, fp);

    fprintf(fp, "\n");
    if (ferror(fp))
	panic("output error");

    if (ferror(stdin))
	panic("input error");

    fclose(fp);

    /* Set the x bit so that we're ready to start executing
     */

    if (fchmod(fd2, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
	perr("cannot give away file");

    close(fd2);
    if (posixly_correct) {
	struct tm runtime;
	char timestr[TIMESIZE];
	runtime = *localtime(&runtimer);
	strftime(timestr, TIMESIZE, "%a %b %e %T %Y", &runtime);
	fprintf(stderr, "job %ld at %s\n", jobno, timestr);
    } else
	fprintf(stderr, "Job %ld will be executed using /bin/sh\n", jobno);
}

static int
in_job_list(long job, long *joblist, int len)
{
    int i;

    for (i = 0; i < len; i++)
	if (job == joblist[i])
	    return 1;

    return 0;
}

static void
list_one_job(char *name, long *joblist, int len, int *first)
{
    struct stat buf;
    struct tm runtime;
    unsigned long ctm;
    char queue;
    long jobno;
    time_t runtimer;
    char timestr[TIMESIZE];

    if (stat(name, &buf) != 0)
	perr("cannot stat in " ATJOB_DIR);

    /* See it's a regular file and has its x bit turned on and
     * is the user's
     */
    if (!S_ISREG(buf.st_mode)
	|| ((buf.st_uid != real_uid) && ! (real_uid == 0))
	|| !(S_IXUSR & buf.st_mode || atverify))
	return;

    if(sscanf(name, "%c%5lx%8lx", &queue, &jobno, &ctm)!=3)
	return;

    /* If jobs are given, only list those jobs */
    if (joblist && !in_job_list(jobno, joblist, len))
	return;

    if (atqueue && (queue != atqueue))
        return;

    runtimer = 60*(time_t) ctm;
    runtime = *localtime(&runtimer);
    strftime(timestr, TIMESIZE, "%a %b %e %T %Y", &runtime);
    if (*first) {
	if (!posixly_correct)
	    printf("Date\t\t\t\tOwner\t\tQueue\tJob#\n");
	*first=0;
    }
    if (posixly_correct)
	printf("%ld\t%s\n", jobno, timestr);
    else {
	struct passwd *pw = getpwuid(buf.st_uid);

	printf("%s\t%s\t%c%s\t%s\n",
	       timestr,
	       pw ? pw->pw_name : "???",
	       queue,
	       (S_IXUSR & buf.st_mode) ? "":"(done)",
	       name);
    }
}

static void
list_jobs(long *joblist, int len)
{
    /* List all a user's jobs in the queue, by looping through ATJOB_DIR,
     * or everybody's if we are root
     */
    DIR *spool;
    struct dirent *dirent;
    int first=1;

#ifdef __FreeBSD__
    (void) setlocale(LC_TIME, "");
#endif

    PRIV_START

    if (chdir(ATJOB_DIR) != 0)
	perr("cannot change to " ATJOB_DIR);

    if (joblist) {		/* Force order to match POSIX */
	char jobglob[32];
	glob_t g;
	int i;

	sprintf(jobglob, "?%05lx*", joblist[0]);
	g.gl_offs = 0;
	glob(jobglob, GLOB_DOOFFS, NULL, &g);
	for (i = 1; i < len; i++) {
	    sprintf(jobglob, "?%05lx*", joblist[i]);
	    glob(jobglob, GLOB_DOOFFS | GLOB_APPEND, NULL, &g);
	}
	for (i = 0; i < g.gl_pathc; i++) {
	    list_one_job(g.gl_pathv[i], joblist, len, &first);
	}
	globfree(&g);
    } else {
	if ((spool = opendir(".")) == NULL)
	    perr("cannot open " ATJOB_DIR);

	/*	Loop over every file in the directory
	 */
	while((dirent = readdir(spool)) != NULL) {
	   list_one_job(dirent->d_name, joblist, len, &first);
	}
	closedir(spool);
    }
    PRIV_END
}

static void
process_jobs(int argc, char **argv, int what)
{
    /* Delete every argument (job - ID) given
     */
    int i;
    struct stat buf;
    DIR *spool;
    struct dirent *dirent;
    unsigned long ctm;
    char queue;
    long jobno;

    PRIV_START

    if (chdir(ATJOB_DIR) != 0)
	perr("cannot change to " ATJOB_DIR);

    if ((spool = opendir(".")) == NULL)
	perr("cannot open " ATJOB_DIR);

    PRIV_END

    /*	Loop over every file in the directory
     */
    while((dirent = readdir(spool)) != NULL) {

	PRIV_START
	if (stat(dirent->d_name, &buf) != 0)
	    perr("cannot stat in " ATJOB_DIR);
	PRIV_END

	if(sscanf(dirent->d_name, "%c%5lx%8lx", &queue, &jobno, &ctm)!=3)
	    continue;

	for (i=optind; i < argc; i++) {
	    if (atoi(argv[i]) == jobno || strcmp(argv[i], dirent->d_name)==0) {
		if ((buf.st_uid != real_uid) && !(real_uid == 0))
		    errx(EXIT_FAILURE, "%s: not owner", argv[i]);
		switch (what) {
		  case ATRM:

		    PRIV_START

		    if (unlink(dirent->d_name) != 0)
		        perr(dirent->d_name);

		    PRIV_END

		    break;

		  case CAT:
		    {
			FILE *fp;
			int ch;

			PRIV_START

			fp = fopen(dirent->d_name,"r");

			PRIV_END

			if (!fp) {
			    perr("cannot open file");
			}
			while((ch = getc(fp)) != EOF) {
			    putchar(ch);
			}
			fclose(fp);
		    }
		    break;

		  default:
		    errx(EXIT_FAILURE, "internal error, process_jobs = %d",
			what);
	        }
	    }
	}
    }
    closedir(spool);
} /* process_jobs */

#define	ATOI2(ar)	((ar)[0] - '0') * 10 + ((ar)[1] - '0'); (ar) += 2;

static time_t
ttime(const char *arg)
{
    /*
     * This is pretty much a copy of stime_arg1() from touch.c.  I changed
     * the return value and the argument list because it's more convenient
     * (IMO) to do everything in one place. - Joe Halpin
     */
    struct timeval tv[2];
    time_t now;
    struct tm *t;
    int yearset;
    char *p;

    if (gettimeofday(&tv[0], NULL))
	panic("Cannot get current time");

    /* Start with the current time. */
    now = tv[0].tv_sec;
    if ((t = localtime(&now)) == NULL)
	panic("localtime");
    /* [[CC]YY]MMDDhhmm[.SS] */
    if ((p = strchr(arg, '.')) == NULL)
	t->tm_sec = 0;		/* Seconds defaults to 0. */
    else {
	if (strlen(p + 1) != 2)
	    goto terr;
	*p++ = '\0';
	t->tm_sec = ATOI2(p);
    }

    yearset = 0;
    switch(strlen(arg)) {
    case 12:			/* CCYYMMDDhhmm */
	t->tm_year = ATOI2(arg);
	t->tm_year *= 100;
	yearset = 1;
	/* FALLTHROUGH */
    case 10:			/* YYMMDDhhmm */
	if (yearset) {
	    yearset = ATOI2(arg);
	    t->tm_year += yearset;
	} else {
	    yearset = ATOI2(arg);
	    t->tm_year = yearset + 2000;
	}
	t->tm_year -= 1900;	/* Convert to UNIX time. */
	/* FALLTHROUGH */
    case 8:				/* MMDDhhmm */
	t->tm_mon = ATOI2(arg);
	--t->tm_mon;		/* Convert from 01-12 to 00-11 */
	t->tm_mday = ATOI2(arg);
	t->tm_hour = ATOI2(arg);
	t->tm_min = ATOI2(arg);
	break;
    default:
	goto terr;
    }

    t->tm_isdst = -1;		/* Figure out DST. */
    tv[0].tv_sec = tv[1].tv_sec = mktime(t);
    if (tv[0].tv_sec != -1)
	return tv[0].tv_sec;
    else
terr:
	panic(
	   "out of range or illegal time specification: [[CC]YY]MMDDhhmm[.SS]");
}

static long *
get_job_list(int argc, char *argv[], int *joblen)
{
    int i, len;
    long *joblist;
    char *ep;

    joblist = NULL;
    len = argc;
    if (len > 0) {
	if ((joblist = malloc(len * sizeof(*joblist))) == NULL)
	    panic("out of memory");

	for (i = 0; i < argc; i++) {
	    errno = 0;
	    if ((joblist[i] = strtol(argv[i], &ep, 10)) < 0 ||
		ep == argv[i] || *ep != '\0' || errno)
		panic("invalid job number");
	}
    }

    *joblen = len;
    return joblist;
}

int
main(int argc, char **argv)
{
    int c;
    char queue = DEFAULT_AT_QUEUE;
    char queue_set = 0;
    char *pgm;

    int program = AT;			/* our default program */
    const char *options = "q:f:t:rmvldbc"; /* default options for at */
    time_t timer;
    long *joblist;
    int joblen;

    posixly_correct = COMPAT_MODE("bin/at", "Unix2003");
    joblist = NULL;
    joblen = 0;
    timer = -1;
    RELINQUISH_PRIVS

    if (argv[0] == NULL)
	usage();
    /* Eat any leading paths
     */
    if ((pgm = strrchr(argv[0], '/')) == NULL)
	pgm = argv[0];
    else
        pgm++;

    namep = pgm;

    /* find out what this program is supposed to do
     */
    if (strcmp(pgm, "atq") == 0) {
	program = ATQ;
	options = "q:v";
    }
    else if (strcmp(pgm, "atrm") == 0) {
	program = ATRM;
	options = "";
    }
    else if (strcmp(pgm, "batch") == 0) {
	program = BATCH;
	options = "f:q:mv";
    }

    /* process whatever options we can process
     */
    opterr=1;
    while ((c=getopt(argc, argv, options)) != -1)
	switch (c) {
	case 'v':   /* verify time settings */
	    atverify = 1;
	    break;

	case 'm':   /* send mail when job is complete */
	    send_mail = 1;
	    break;

	case 'f':
	    atinput = optarg;
	    break;

	case 'q':    /* specify queue */
	    if (strlen(optarg) > 1)
		usage();

	    atqueue = queue = *optarg;
	    if (!(islower(queue)||isupper(queue)))
		usage();

	    queue_set = 1;
	    break;

	case 'd':
	    warnx("-d is deprecated; use -r instead");
	    /* fall through to 'r' */

	case 'r':
	    if (program != AT)
		usage();

	    program = ATRM;
	    options = "";
	    break;

	case 't':
	    if (program != AT)
		usage();
	    timer = ttime(optarg);
	    break;

	case 'l':
	    if (program != AT)
		usage();

	    program = ATQ;
	    options = "q:";
	    break;

	case 'b':
	    if (program != AT)
		usage();

	    program = BATCH;
	    options = "f:q:mv";
	    break;

	case 'c':
	    program = CAT;
	    options = "";
	    break;

	default:
	    usage();
	    break;
	}
    /* end of options eating
     */

    /* select our program
     */
    if(!check_permission())
	errx(EXIT_FAILURE, "you do not have permission to use this program");
    switch (program) {
    case ATQ:

	REDUCE_PRIV(DAEMON_UID, DAEMON_GID)

	if (queue_set == 0)
	    joblist = get_job_list(argc - optind, argv + optind, &joblen);
	list_jobs(joblist, joblen);
	break;

    case ATRM:

	REDUCE_PRIV(DAEMON_UID, DAEMON_GID)

	process_jobs(argc, argv, ATRM);
	break;

    case CAT:

	process_jobs(argc, argv, CAT);
	break;

    case AT:
	/*
	 * If timer is > -1, then the user gave the time with -t.  In that
	 * case, it's already been set. If not, set it now.
	 */
	if (timer == -1)
	    timer = parsetime(argc, argv);

	if (atverify)
	{
	    struct tm *tm = localtime(&timer);
	    fprintf(stderr, "%s\n", asctime(tm));
	}
	writefile(timer, queue);
	break;

    case BATCH:
	if (queue_set)
	    queue = toupper(queue);
	else
	    queue = DEFAULT_BATCH_QUEUE;

	if (argc > optind)
	    timer = parsetime(argc, argv);
	else
	    timer = time(NULL);

	if (atverify)
	{
	    struct tm *tm = localtime(&timer);
	    fprintf(stderr, "%s\n", asctime(tm));
	}

        writefile(timer, queue);
	break;

    default:
	panic("internal error");
	break;
    }
#ifdef __APPLE__
    if (ferror(stdout) != 0 || fflush(stdout) != 0)
        err(1, "stdout");
#endif
    exit(EXIT_SUCCESS);
}
