#!/bin/bash

if [ "$(id -u)" == "0" ]; then
   echo "Munadi uninstaller must be run as normal user." 1>&2
   exit 1
fi

rm -f "/home/$USER/.local/bin/munadi"
rm -f "/home/$USER/.local/share/applications/munadi.desktop"
rm -f "/home/$USER/.config/autostart/munadi_autostart.desktop"

rm -rf "/home/$USER/.munadi"
