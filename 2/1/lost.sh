#!/bin/bash

XVFB="Xvfb"
XVFB_OPTIONS=":32 -ac -screen 0 1024x768x24 -fbdir /tmp"
XDT="xdotool"

button_click() {
	RES=$(compare -metric RMSE -subimage-search /tmp/Xvfb_screen0 button.png res.png 2>&1 | perl -lne '/^.*@\s(.*),(.*)/; print $1." ".$2')
	sleep 20
	echo $RES
	$XDT mousemove $RES click 1
	$XDT click 1
	sleep 30
}

download() {
	cd ~/Downloads
	export FILE=$(ls -t | head -1)
	NAME=$(perl -le '$ENV{"FILE"}=~ /^(.*).torrent$/;print $1')
	transmission-gtk $FILE &
	TM_ID=$!
	while [ 1 ]
	do 
		if [ -e "$NAME.avi" ]
		then
			echo "$NAME.avi download"
			kill -9 $LUAKIT_ID
			kill -9 $XVFB_ID
			kill -9 $TM_ID
			exit 0
		fi
	done
}


if [ $# -ne 2 ]
then
	echo "Program needs 2 arguments: login password"
else
	LOGIN=$1
	PASS=$2
fi


if [ -e /tmp/.X32-lock ]
then
	echo "Xvfb on 32.0 is all ready running"
	killall Xvfb
	sleep 2
	echo "Xvfb on 32.0 is killed"
fi

$XVFB $XVFB_OPTIONS &
XVFB_ID=$!

export DISPLAY=:32.0
luakit &
LUAKIT_ID=$!
sleep 3
$XDT type ":o www.lostfilm.tv"
$XDT key KP_Enter
sleep 30

$XDT type "gi"
$XDT type $LOGIN
$XDT key Tab
$XDT type $PASS
$XDT key KP_Enter
sleep 20
button_click
button_click
$XDT key KP_Enter
sleep 30
download

exit 2









