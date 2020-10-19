#!/bin/sh
module="0"
device="egan"
mode="664"
# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
sudo /sbin/insmod ./$module.ko $* || exit 1
# remove stale nodes
rm -f /dev/${device}
major="$(grep 'egan' /proc/devices | awk '{print $1}')"
mknod /dev/${device} c $major 0
# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="wheel"
chgrp $group /dev/${device}
chmod $mode  /dev/${device}
