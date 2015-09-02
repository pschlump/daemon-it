#!/bin/sh
#
# email-relay   startup script for the email relay
# processname:       email-relay
# config:            ~/email-relay.cfg
# pidfile:           /var/run/email-relay.pid
# chkconfig: 2345 99 01
# description:       the daemon for the email relay.

# Source function library.
. /lib/lsb/init-functions

PATH=/sbin:/usr/sbin:/bin:/usr/bin
DESC="Email Runner"
NAME=email-relay
RUN_DIR=/home/emailrelay/Projects/email-relay
DAEMON=$RUN_DIR/$NAME
DAEMON_IT=/usr/local/bin/daemon-it
DAEMON_IT=/home/ubuntu/Projects/d-daemon/daemon-it
DAEMON_ARGS="--dir=. --port=80 --emailCfgFile=/home/emailrelay/.email/email-config.json --cfg=/home/emailrelay/.email/email-auth.cfg"
DAEMON_USER=emailrelay
DAEMON_GROUP=emailrelay
PIDFILE=/var/run/$NAME.pid
JAIL=/home/jail/emailrelay

# Exit if the package is not installed
[ -x "$JAIL/$DAEMON" ] || exit 0
[ -x "$DAEMON_IT" ] || exit 0
mkdir -p "$JAIL/$RUN_DIR"/log

# Read configuration variable file if it is present
[ -r /etc/sysconfig/email-relay/$NAME ] && . /etc/sysconfig/email-relay/$NAME

start() {
        echo -n $"Starting $NAME: "
        rm -f "$PIDFILE"
	#echo CC $DAEMON_IT -R "$JAIL" -c "$RUN_DIR" -u "$DAEMON_USER" -g "$DAEMON_GROUP" -o log/output.log -O - -p "$PIDFILE" \
	#			-- $DAEMON $DAEMON_ARGS &
        $DAEMON_IT -R "$JAIL" -c "$RUN_DIR" -u "$DAEMON_USER" -g "$DAEMON_GROUP" -o log/output.log -O - -p "$PIDFILE" \
			-- $DAEMON $DAEMON_ARGS &
        RETVAL="$?"
        return "$RETVAL"
}
stop() {
        echo -n $"Stopping $NAME: "
	# echo DD killproc -p "$PIDFILE" 
        killproc -p "$PIDFILE" 
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
