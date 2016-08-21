#!/bin/bash

OS=`uname`

echo -n "Building for $OS..."

case $OS in

 Darwin )
	./pirate-loader --dev=/dev/tty.usbmodem00000001 --hello
	;;
 Linux )
	./pirate-loader --dev=/dev/ttyACM0 --hello
	;;
 FreeBSD )
	./pirate-loader --dev=/dev/ttyACM0 --hello
 	;;
 *)
	echo "ERROR"
	echo "Unsupported operating system $OS"
	exit -1
	;;
esac

echo "DONE"
exit 0
