#!/bin/sh

echo "start install......:"`pwd`

#Add operation permission
chmod -R 777 ./
#kill app
oem/S99honyar stop

# cp -a * /

for file in ./*
do
    if test -d $file
    then
        cp -af $file /
    fi
done

sync
#delete files
rm -rf *

echo "Successfully installed"

echo "App reboot......"
reboot
