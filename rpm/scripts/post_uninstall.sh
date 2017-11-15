#!/bin/sh
export LC_ALL=C

rm -f /etc/systemd/system/kungfu.service

systemctl daemon-reload

rm -f /usr/bin/yjj
rm -f /usr/bin/wingchun
rm -f /usr/bin/kungfuctl

rm -f /usr/lib64/python2.7/site-packages/kungfu.pth
rm -rf /opt/kungfu/master/lib