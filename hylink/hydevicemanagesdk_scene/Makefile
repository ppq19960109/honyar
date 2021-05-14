BIN=libhydevicemanagesdk.so
CROSS_COMPILE =arm-rockchip-linux-gnueabihf-#mipsel-openwrt-linux-
CC  := $(CROSS_COMPILE)gcc
AR  := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip
#CC  := /opt/OpenWrt-Toolchain-ramips-mt7688_gcc-5.3.0_musl-1.1.14.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-5.3.0_musl-1.1.14/bin/mipsel-openwrt-linux-gcc
#AR  := /opt/OpenWrt-Toolchain-ramips-mt7688_gcc-5.3.0_musl-1.1.14.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-5.3.0_musl-1.1.14/bin/mipsel-openwrt-linux-ar
#STRIP := /opt/OpenWrt-Toolchain-ramips-mt7688_gcc-5.3.0_musl-1.1.14.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-5.3.0_musl-1.1.14/bin/mipsel-openwrt-linux-strip
#CC  := gcc
#AR  := ar
#STRIP := strip

DIR :=./src
FILES :=$(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

FLAG := -Wall -I./include -L./lib 
#FLAG := -I./include -L./lib_lede

LDFLAGS := -lpthread -ldl -lm -lrexgatewaysdk -lsqlite3

all:
	${CC}   ${FLAG}  ${FILES}  -fPIC -shared -o $(BIN)  ${LDFLAGS}
#	${CC}   ${FLAG}  ${FILES}  -o $(BIN)  ${LDFLAGS}
small: 
	$(STRIP) $(BIN)
.PYTHON:clean
clean:
	-rm -rf $(BIN)  *.o