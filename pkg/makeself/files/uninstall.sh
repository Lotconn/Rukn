#!/bin/bash

if [ "$(id -u)" == "0" ]; then
   echo "Munadi uninstaller must be run as normal user." 1>&2
   exit 1
fi

source /home/$USER/.munadi/rm_munadi.sh

echo "*** Uninstallation of Munadi 17.02 completed successfully ***"
