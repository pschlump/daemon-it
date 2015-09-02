#!/bin/bash

# check that we ar running as root

if [ "$(whoami)" == "root" ] ; then
	:
else
	echo "Usage: !! run as root"
	exit 1
fi

# First create some directories.  The top one will be the jail.
# Under that we will create directories that look like the ones
# that you would see in a root file system.

D="$1"
U="$2"
G="$3"

if [ -z "${D}" ] ; then
	echo "Usage: create-jail.sh PathToJail UserToCreate GroupToUse - PathToJail can not be empty"
	exit 1
fi
if [ -z "${U}" ] ; then
	echo "Usage: create-jail.sh PathToJail UserToCreate GroupToUse - UserToCreate can not be empty"
	exit 1
fi
if [ -z "${G}" ] ; then
	echo "Usage: create-jail.sh PathToJail UserToCreate GroupToUse - GroupToUse can not be empty"
	exit 1
fi

mkdir -p ${D}/bin
mkdir -p ${D}/home/emailrelay
mkdir -p ${D}/etc
mkdir -p ${D}/dev
mkdir -p ${D}/tmp
mkdir -p ${D}/lib
mkdir -p ${D}/lib64
mkdir -p ${D}/var/log

# Now let's create some devices that programs will need.

cd ${D}/dev
/bin/mknod -m 0666 ${D}/dev/null c 1 3
/bin/mknod -m 0666 ${D}/dev/random c 1 8
/bin/mknod -m 0444 ${D}/dev/urandom c 1 9

# Now the account that the program will run under.

groupadd ${G}
useradd -m -s /bin/false -d ${D}/home/${U} -g ${G} ${U}

# Now the directory where the executable will be.

cd ${D}/home/${U}
mkdir -p Projects/email-relay/log
chown -R ${U}:${G} *

# While I am in the login directory for this new user I will take a look 
# at any extra files that may have been created.  I usually remove them.

ls -a
rm -rf .mozilla

echo "Should copy in the executable now and then run part 2, ./create-jail-pt2.sh"

