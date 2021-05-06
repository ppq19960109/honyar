#wpa_supplicant -B -i wlan0 -c /data/cfg/wpa_supplicant.conf
#./wakeWordAgent -e gpio &

rmmod rkled
rmmod rkkeyasync

insmod /oem/rkled.ko
insmod /oem/rkkeyasync.ko

killall dnsmasq
rfkill block all

/oem/S99honyar start
