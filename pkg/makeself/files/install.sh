#!/bin/bash

if [ -d "/opt/munadi" ]; then
	echo "An old copy of Munadi is detected, will now remove..."
	sudo rm -r "/opt/munadi"
fi

if [ "$(id -u)" == "0" ]; then
   echo "Munadi installer must be run as normal user." 1>&2
   exit 1
fi

# rm existing 

source ./rm_munadi.sh

# make sure bin exists

if [ ! -d "/home/$USER/.local/bin" ]; then
	mkdir -p "/home/$USER/.local/bin"
fi

# make sure autostart exists

if [ ! -d "/home/$USER/.config/autostart" ]; then
	mkdir -p "/home/$USER/.config/autostart"
fi

# make sure applications exists

if [ ! -d "/home/$USER/.local/share/applications" ]; then
        mkdir -p "/home/$USER/.local/share/applications"
fi

IDIR="/home/$USER/.munadi"

if [ -d "$IDIR" ]; then
    rm -rf "$IDIR"
fi

mkdir -p $IDIR

cp -rf . "$IDIR"
chmod a+rx "$IDIR" -R

echo "Icon=$IDIR/munadi.png" >> "$IDIR/munadi.desktop"
echo "Icon=$IDIR/munadi.png" >> "$IDIR/munadi_autostart.desktop"

echo "Exec=$IDIR/munadi" >> "$IDIR/munadi.desktop"
echo "Exec=$IDIR/munadi -startup" >> "$IDIR/munadi_autostart.desktop"

ln -sf "$IDIR/munadi" "/home/$USER/.local/bin/"
ln -sf "$IDIR/munadi.desktop" "/home/$USER/.local/share/applications/"
ln -sf "$IDIR/munadi_autostart.desktop" "/home/$USER/.config/autostart/"

echo "*** Installation of Munadi 17.02 completed successfully ***"

( $IDIR/munadi & ) > /dev/null 2>&1
