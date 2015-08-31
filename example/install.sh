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

