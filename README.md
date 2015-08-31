# daemon-it - a C program to start Go server as a chroot daemon

Go has some challenges with Linux and setting UID across all
threads.  For this reason I had to dig up this old chunk of
code.  The original code was for Solaris - I modified it to
work on 14.04 of Ubuntu Linux.   

Basically this program will use some command line options
to set up a chroot jail, change users and groups to a non-root
account, deal with stdin/stdout/stderr so as to create
a daemon.

## Example

This example just runs the code in the chroot jail.
Let's say you have a chroot jail already set up (There are
sections below that detail how to do this).

Let's suppose that you have a jail set up as /home/jail.
It has a user named emailrelay with a group emailrelay.
The directory to run in is /home/jail/home/emailrelay/Projects/email-relay.
The log file will go in /home/jail/home/emailrelay/Projects/email-relay/log/
and be called output.log.  It will have both stdout and stderr in it.
The pid file will be in the current directory where ./daemon-it is.
Our jail has a /home/jail/bin/echo in it.

``` bash

    $ ./daemon-it -R /home/jail -c /home/emailrelay/Projects/email-relay -u emailrelay -g emailrelay \
        -o log/output.log -O - -p emailrealy.pid \
        -- /bin/echo aa bc cc dd ee

```

The world's safest echo!  Yea!

## Step by Step Example

This is a step by step process that includes setting up the jail,
testing it and setting up the init.d script to start running the
server.

### Setting up a Chroot Jail on Linux

First create some directories.  The top one will be the jail.
Under that we will create directories that look like the ones
that you would see in a root file system.

``` bash

	$ D=/home/jail
	$ sudo mkdir -p ${D}/bin
	$ sudo mkdir -p ${D}/home/emailrelay
	$ sudo mkdir -p ${D}/etc
	$ sudo mkdir -p ${D}/dev
	$ sudo mkdir -p ${D}/tmp
	$ sudo mkdir -p ${D}/lib
	$ sudo mkdir -p ${D}/lib64
	$ sudo mkdir -p ${D}/var/log
	
```

Now let's create some devices that programs will need.

``` bash

	$ cd ${D}/dev
 	$ sudu /bin/mknod -m 0666 ${J}/dev/null c 1 3
	$ sudu /bin/mknod -m 0666 ${J}/dev/random c 1 8
	$ sudu /bin/mknod -m 0444 ${J}/dev/urandom c 1 9

```

Now the account that the program will run under.

``` bash

	$ sudo groupadd emailrelay
	$ sudo useradd -m -s /bin/bash -d /home/jail/home/emailrelay -g emailrelay emailrelay

```

Now the directory where the executable will be.

``` bash

	$ cd ~emailrelay
	$ sudo mkdir -p Projects/email-relay/log
	$ chown -R emailrelay:emailrelay *

```

While I am in the login directory for this new user I will take a look 
at any extra files that may have been created.  I usually remove them.

``` bash

	$ ls -a
	$ sudo rm -rf .mozilla

```

Copy in the program that you are going to run.
My program was written on a remote server so I used ssh to go grab it.

``` bash

	cd Projects/email-relay
	$ scp pschlump@www.2c-why.com:/home/pschlump/Projects/email-relay/email-relay .
	Password:
	...
	$ sudo chown root:daemon email-relay
	$ sudo chmod -w email-relay
	$ sudo chmod +x email-relay
	
```

Now that I have the code I need to look at what libraries it will need to run.
This is usually a very short list for a Go program.

``` bash

	$ list="$(ldd email-relay | grep -o '/lib.*\.[0-9]')"

``` 

*list* will have the libraries in it.  Now I need to copy them into the
jail file system.


``` bash

	$ cd /home/jail
	$ for i in $list ; do mkdir -p $( dirname "${D}${i}" ); done
	$ for i in $list ; do cp -v "$i" "${D}${i}"; done

```

If I copy in other executables like /bin/echo I will need to do the list=
and the 2 for loops again.

My chroot jail should be ready to run.

This is the time to add any configuration files that the program needs.

### Setup the program to run as a daemon

First copy in daemon-it into a executable directory on the server.

``` bash

	$ cd
	$ scp pschlump@www.2c-why.com:/home/pschlump/Projects/daemon-it/daemon-it .
	Password:
	...
	$ sudo cp daemon-it /usr/local/bin

```

Now create the script that will run at boot time.
I called this file email-runner.sh

``` bash

#!/bin/sh
#
# email-runner   startup script for the email runner
# processname:       email-runner
# config:            ~/email-runner.cfg
# pidfile:           /var/run/email-runner.pid
# chkconfig: 2345 99 01
# description:       the daemon for the email runner.

# Source function library.
. /etc/rc.d/init.d/functions

PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC="Email Runner"
NAME=email-runner
RUN_DIR=/home/emailrunner/Project/email-runner
DAEMON=$RUN_DIR/$NAME
DAEMON_IT=/usr/local/bin/daemon-it
DAEMON_ARGS="--dir=. --port=80"
DAEMON_USER=emailrunner
DAEMON_GROUP=emailrunner
PIDFILE=/var/run/$NAME.pid

# Exit if the package is not installed
[ -x "$DAEMON" ] || exit 0
[ -x "$DAEMON_IT" ] || exit 0
mkdir -p "$RUN_DIR"/log

# Read configuration variable file if it is present
[ -r /etc/sysconfig/email-runner/$NAME ] && . /etc/sysconfig/email-runner/$NAME

start() {
        echo -n $"Starting $NAME: "
        rm -f "$PIDFILE"
        $DAEMON_IT -R /home/jail -c "$RUN_DIR" -u "$DAERMON_USER" -g "$DAERMON_GROUP" -o log/output.log -O - -p "$PIDFILE" \
			-- $DAEMON $DAEMON_ARGS &
        RETVAL="$?"
        return "$RETVAL"
}
stop() {
        echo -n $"Stopping $NAME: "
        killproc -p "$PIDFILE" -d 10 "$DAEMON"
        RETVAL="$?"
        echo
        [ $RETVAL = 0 ] && rm -f "$PIDFILE"
        return "$RETVAL"
}

case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  restart|reload)
        stop
        start
        ;;
  *)
        echo "Usage: $NAME {start|stop|restart|reload}" >&2
        exit 1
        ;;
esac

exit $RETVAL

```


Now a script, I called install.sh to setup the /etc/inid.d stuff.


``` bash


#!/bin/bash

if [ "$(whoami)" == "root" ] ; then
	:
else
	echo "Usage: !! run as root"
	exit 1
fi

cp email-runner.sh /etc/init.d/tab-server
cd /etc
ln -s /etc/init.d/tab-server ./rc0.d/K98tab-server
ln -s /etc/init.d/tab-server ./rc1.d/K98tab-server
ln -s /etc/init.d/tab-server ./rc2.d/S98tab-server
ln -s /etc/init.d/tab-server ./rc3.d/S98tab-server
ln -s /etc/init.d/tab-server ./rc4.d/S98tab-server
ln -s /etc/init.d/tab-server ./rc5.d/S98tab-server
ln -s /etc/init.d/tab-server ./rc6.d/K98tab-server


```

And make them executable and run them.

``` bash

	$ chmod +x install.sh email-runner.sh
	$ sudo ./install.sh
	$ cd /etc/init.d
	$ ./email-runner start

```


