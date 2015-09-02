#!/bin/bash

# check that we ar running as root

if [ "$(whoami)" == "root" ] ; then
	:
else
	echo "Usage: !! run as root"
	exit 1
fi

D="$1"
U="$2"
G="$3"
P="$4"

if [ -z "${D}" ] ; then
	echo "Usage: create-jail-pt2.sh PathToJail UserToCreate GroupToUse ProgramName- ProgramName can not be empty"
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
if [ -z "${P}" ] ; then
	echo "Usage: create-jail-pt2.sh PathToJail UserToCreate GroupToUse ProgramName- ProgramName can not be empty"
	exit 1
fi

# Modify below to copy in the server/program that you need to run.

# Copy in the program that you are going to run.
# My program was written on a remote server so I used ssh to go grab it.

cd ${D}/home/${U}/Projects/${P}
# scp pschlump@www.2c-why.com:/home/pschlump/Projects/${P}/${P} .
chown root:daemon ${P}
chmod -w ${P}
chmod +x ${P}
	
# Now that I have the code I need to look at what libraries it will need to run.
# This is usually a very short list for a Go program.

list="$(ldd ${P} | grep -o '/lib.*\.[0-9]')"
echo $list

# *list* will have the libraries in it.  Now I need to copy them into the
# jail file system.

cd ${D}
for i in $list ; do mkdir -p $( dirname "${D}${i}" ); done
for i in $list ; do cp -v "$i" "${D}${i}"; done

# If I copy in other executables like /bin/echo I will need to do the list=
# and the 2 for loops again.

echo "Your chroot jail should be ready to run."

