//
// D A E M O N - I T - run a Go program (or other program) as a daemon.
//
// Copyright (C) Philip Schlump, 1996-2015.
// Version: 1.0.0
// BuildNo: 025
//
// Licensed under the terms of the MIT license. 
// Whitchever one is more convenient for the user.
//
// License in LICENSE
// How to use in README.md
// Examples for Linux, a /etc/init.d start up script in ./example/init.d/email-relay
//

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>


//
// Notes: -- Not implemented yet
// 		-E e1=Val1 e2=Val2 ...
//

void stdIOfiles(char *stdout_fn, char *stderr_fn, char *stdin_fn);
void usage(void);

int main(int argc, char** argv) {

    pid_t pid;
	int debug = 0;
	char *chroot_dir = "";
	char *cd_dir = "";
	char *username = "";
	char *groupname = "";
	char *stdout_fn = "";
	char *stderr_fn = "";
	char *stdin_fn = "";
	char *pid_fn = "";
	int cmd_start = 1;
	int i, j, c;
	char *s;
	FILE *pid_fp = NULL;
	uid_t uid;
	gid_t gid;

	for ( i = 1; i < argc; i++ ) {
		s = argv[i];
		c = *s++;
		if ( c != '-' ) {
			cmd_start = i;
			goto done;
		} else if ( i+1 >= argc ) {
			usage();
		} else {
			c = *s;
			switch ( c ) {
			case 'R':
				chroot_dir = argv[++i];
				break;
			case 'c':
				cd_dir = argv[++i];
				break;
			case 'u':
				username = argv[++i];
				break;
			case 'D':
				debug++;
				break;
			case 'g':
				groupname = argv[++i];
				break;
			case 'o':
				stdout_fn = argv[++i];
				break;
			case 'O':
				stderr_fn = argv[++i];
				break;
			case 'p':
				pid_fn = argv[++i];
				break;
			case 'i':
				stdin_fn = argv[++i];
				break;
			// case 'E':
			case '-':
				cmd_start = ++i;
				goto done;
			default:
				usage();
			}
		}
	}
done:;


    if (getuid() != 0) {
        fprintf(stderr, "Error: only root can execute %s, exiting\n", argv[0]);
        exit(1);
    }

	// Processing before chroot - all things that require the full OS and file system

	if ( strcmp ( pid_fn, "" ) != 0 ) {
		if ( ( pid_fp = fopen ( pid_fn, "w" ) ) == NULL ) {
			fprintf(stderr, "Error: unable to open %s for writing - pid file, exiting\n", pid_fn );
			exit(1);
		}
	}

	uid = 0;
	gid = 0;
	if ( strcmp ( username, "" ) != 0 ) {
		struct passwd *pw = getpwnam ( username );
		if ( pw == NULL ) {
			fprintf ( stderr, "Error: invalid username >%s<, exiting\n", username );
			exit(1);
		}
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}

	if ( strcmp ( groupname, "" ) != 0 ) {
		struct group *gr = getgrnam ( groupname );
		if ( gr == NULL ) {
			fprintf ( stderr, "Error: invalid groupname >%s<, exiting\n", groupname );
			exit(1);
		}
		gid = gr->gr_gid;
	}

	if ( debug ) {
		printf ( "uid = %ld gid = %ld\n", (long)uid, (long)gid );
	}

	// Processing after chroot - do the work now

	if ( strcmp ( chroot_dir, "" ) != 0 ) {
		if (chdir(chroot_dir) != 0) {
			fprintf(stderr, "Error: chroot dir %s did not exist, exiting\n", chroot_dir);
			exit(1);
		}
		if (chroot(chroot_dir) != 0) {
			fprintf(stderr, "Error: unable to chroot to %s, exiting\n", chroot_dir);
			exit(1);
		}
	}

	if (setgid(gid) == -1) {
		fprintf(stderr, "Error: unable to switch groupid, exiting\n");
		exit(1);
	}
	if (setuid(uid) == -1) {
		fprintf(stderr, "Error: unable to switch userid, exiting\n");
		exit(1);
	}

	pid = fork();
	if (pid == 0) {

		if ( strcmp ( cd_dir, "" ) != 0 ) {
			if (chdir(cd_dir) != 0) {
				fprintf(stderr, "Error: unable to change working directory, exiting\n");
				exit(1);
			}
		}

		if (setsid() == -1) {
			fprintf(stderr, "Error: Unable to create new processgroup, exiting\n");
			exit(1);
		}

		stdIOfiles(stdout_fn, stderr_fn, stdin_fn);

		char *prog = argv[cmd_start];
		char **args = ( char ** ) malloc ( argc * sizeof ( char * ) );		// Over allocate but simple.
		for ( i = cmd_start, j = 0; i < argc; i++, j++ ) {
			args[j] = argv[i];
		}
		args[j] = NULL;

		// -E option - not implemented yet - empty environment.
		char* eenv[0 + 1];
		eenv[0] = NULL;

		return execve(prog, args, eenv);
	} else if (pid == -1) {
		fprintf(stderr, "Error: unable to fork the wrapped process, exiting\n");
		exit(1);
	}

	// parent
	// fprintf ( stderr, "Line: %d\n", __LINE__ );
	fprintf(stderr, "PID of child = %d\n", pid );
	if ( pid_fp ) {
		fprintf(pid_fp, "%ld\n", (long)pid );
		fclose ( pid_fp );
	}

	exit(0);
}

void stdIOfiles(char *stdout_fn, char *stderr_fn, char *stdin_fn) {
    int fe = -1, fi = -1, fo = -1, fm = -1;
    if ((fm = open("/dev/null", O_RDWR, 0)) != -1) {
		// --------- stdin -----------------------------------------------------------------------------------------------
		if ( strcmp ( stdin_fn, "-" ) == 0 ) {			// if a "-" for stdin, then just let it clone to forked child
		} else if ( strcmp ( stdin_fn, "" ) != 0 ) {	// if it is a file name, we will need to open it.
			if ((fi = open(stdin_fn, O_RDONLY, 0)) != -1) {
				fprintf(stderr, "Info: Unable to open %s for stdin input, duping /dev/null intead\n", stdin_fn );
				(void)dup2(fi, STDIN_FILENO);
			} else {
				(void)dup2(fm, STDIN_FILENO);
			}
		} else {										// Else let's close it - no stdin
			(void)dup2(fm, STDIN_FILENO);
		}
		// --------- stdout ----------------------------------------------------------------------------------------------
		if ( strcmp ( stdout_fn, "-" ) == 0 ) {			// if a "-" for stdout, then just let it clone to forked child
		} else if ( strcmp ( stdout_fn, "" ) != 0 ) {	// if it is a file name, we will need to open it.
			if ((fo = open(stdout_fn, O_WRONLY|O_CREAT, 0764)) != -1) {
				(void)dup2(fo, STDOUT_FILENO);
			} else if ((fo = open(stdout_fn, O_RDWR|O_APPEND, 0764)) != -1) {
				(void)dup2(fo, STDOUT_FILENO);
			} else {
				fprintf(stderr, "Info: Unable to open %s for stdout output, duping /dev/null intead\n", stdout_fn );
				(void)dup2(fm, STDOUT_FILENO);
			}
		} else {										// Else let's close it - no stdout
			(void)dup2(fm, STDOUT_FILENO);
		}
		// --------- stderr ----------------------------------------------------------------------------------------------
		if ( strcmp ( stderr_fn, "&1" ) == 0 ) {		// if a "&1" then dup of stdout
			if ( fo != -1 ) {
				(void)dup2(fo, STDERR_FILENO);
			} else {
				(void)dup2(fm, STDERR_FILENO);
			}
		} else if ( strcmp ( stderr_fn, "-" ) == 0 ) {	// if a "-" for stderr, then just let it clone to forked child
		} else if ( strcmp ( stderr_fn, "" ) != 0 ) {	// if it is a file name, we will need to open it.
			if ((fe = open(stderr_fn, O_WRONLY|O_CREAT, 0764)) != -1) {
				(void)dup2(fe, STDERR_FILENO);
			} else if ((fe = open(stderr_fn, O_RDWR|O_APPEND, 0764)) != -1) {
				(void)dup2(fe, STDERR_FILENO);
			} else {
				fprintf(stderr, "Info: Unable to open %s for stderr error, duping /dev/null intead\n", stderr_fn );
				(void)dup2(fm, STDERR_FILENO);
			}
		} else {										// Else let's close it - no stderr
			(void)dup2(fm, STDERR_FILENO);
		}

        if (fm > STDERR_FILENO) {
            (void)close(fm);
		}
    } else {
        fprintf(stderr, "Info: unable to redirect stdout, stderr, stdin - oh well...\n");
    }
}


void usage(void) {
	fprintf ( stderr, "Usage: daemon-it [ -R chrootdir ] [ -u user ] [ -g group ] [ -o stdout ] [ -O stderr ] [ -i stdin ] [ -p pidfile ] [ -E env=val ... ] -- cmd args ...\n" );
	fprintf ( stderr, "  -p pidfile         if specified a file will be opend for the sub-process PID - this should be a hard path - chroot will not effect this path.  1st\n" );
	fprintf ( stderr, "  -R chrootdir       direcotry to chagne to if you are going to chroot - this happens 2nd.\n" );
	fprintf ( stderr, "  -u user, -g group	Set the user/group - the names will be looked up and the UID, GID will be set.  3rd\n" );
	fprintf ( stderr, "  -o <fn>            Point stdout at a file or '-' for passing stdout through.\n" );
	fprintf ( stderr, "  -i <fn>            Point stdin at a file or '-' for passing stdout through.\n" );
	fprintf ( stderr, "  -O <fn>            Point stderr at a file or '-' for passing stdout through, '&1' to dup stdout.\n" );
	fprintf ( stderr, "  -E name=val        Set environment, not implemented yet\n" );


	exit(1);
}

