#!/bin/sh

file=.re
 
if [ ! -f "$file" ]; then
    echo "$file不存在"
else
    echo "$file已存在 reboot"
    rm -f "$file"
    reboot -n
fi
[root@localhost data]# 