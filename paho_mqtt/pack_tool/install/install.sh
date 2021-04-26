#!/bin/sh

echo "start install......"
SYS_UPDATE_CONF_PATH="/etc"
SYS_INIT_CONF_PATH="/etc/rc.d"

APP_PATH="app"
HYAPP_PATH="hyapp"
IOTAPP_PATH="iotapp"

LIB_PATH="libs"
TARGET_LIB_PATH="/usr/lib"

USER_PATH="/userdata"
#Add operation permission
chmod -R 777 ./

./S99honyar stop
#kill app
if [ ! -d "$USER_PATH" ]; then
    mkdir "$USER_PATH"
fi

cp -a honyar.conf $SYS_UPDATE_CONF_PATH
cp -a S99honyar $SYS_INIT_CONF_PATH

cp -a $APP_PATH $USER_PATH
cp -a $HYAPP_PATH $USER_PATH
cp -a $IOTAPP_PATH $USER_PATH

cp -a $LIB_PATH/* $TARGET_LIB_PATH

#delete files
rm -rf *

sync
echo "Successfully installed"

echo "App reboot......"
reboot
