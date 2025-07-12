export PATH=/bin/:/usr/bin/:/usr/:/opt/netsurf/m68*/cross/m68*/:/opt/netsurf/m68*/env/lib/
export PKG_CONFIG_PATH=/mnt/d/opt/netsurfy/netsurf-all-3.11/inst-amigaos3/lib/pkgconfig:/opt/netsurf/m68k-unknown-amigaos/env/lib/pkgconfig
clear;make -j16 HOST=newer TARGET=morphos PREFIX=/opt/netsurf/ #package
#AR=ar HOST_GCC=gcc CC=/opt/netsurf/m68k-unknown-amigaos/cross/bin/m68k-unknown-amigaos-gcc
