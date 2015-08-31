
all: daemon-it

daemon-it: daemon-it.c
	gcc -o daemon-it daemon-it.c

test00:
	./daemon-it -D -D -R /home/jail -c /home/emailrelay/Projects/email-relay -u emailrelay -g emailrelay -o log/output.log -O - -p emailrealy.pid \
		-- /bin/echo aa bc cc dd ee

test01:
	./daemon-it -R /home/jail -c /home/emailrelay/Projects/email-relay -u emailrelay -g emailrelay -o log/output.log -O - -p emailrealy.pid \
		-- ./email-relay --dir=/home/emailrelay/www/www_default_com --port=80 

# // -R chroot -c chdir -u user -g gorup -o <stdout> -O <stderr|-> -p <pidfile> -- cmd ...

