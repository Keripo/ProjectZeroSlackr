#!/bin/sh
#set -vx
#exec >> /opt/Emulators/iAtari800/Misc/Launch.log 2>&1

# Format: $binary $disk_image_path -refresh $#
killall -15 ZeroLauncher >> /dev/null 2>&1
cpu 78
if [ -z "$1" ]; then
	cd /opt/Emulators/iAtari800
	exec /opt/Emulators/iAtari800/iAtari800 "\"/opt/Emulators/iAtari800/Disks/Dog Daze Deluxe (APX20264).atr\"" -refresh 6
else
	cd /opt/Emulators/iAtari800
	exec /opt/Emulators/iAtari800/iAtari800 "\"$1\"" -refresh "$2"
fi