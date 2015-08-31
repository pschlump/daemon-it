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
