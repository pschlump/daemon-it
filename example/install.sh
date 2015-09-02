#!/bin/bash

if [ "$(whoami)" == "root" ] ; then
	:
else
	echo "Usage: !! run as root"
	exit 1
fi

cp email-runner.sh /etc/init.d/email-sender
cd /etc
ln -s /etc/init.d/email-sender ./rc0.d/K98email-sender
ln -s /etc/init.d/email-sender ./rc1.d/K98email-sender
ln -s /etc/init.d/email-sender ./rc2.d/S98email-sender
ln -s /etc/init.d/email-sender ./rc3.d/S98email-sender
ln -s /etc/init.d/email-sender ./rc4.d/S98email-sender
ln -s /etc/init.d/email-sender ./rc5.d/S98email-sender
ln -s /etc/init.d/email-sender ./rc6.d/K98email-sender

