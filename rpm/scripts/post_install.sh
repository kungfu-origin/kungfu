#!/bin/sh

KUNGFU_HOME_DIR=/opt/kungfu

KUNGFU_DATA_DIR=/shared/kungfu
KUNGFU_SOCKETS_DIR=$KUNGFU_DATA_DIR/socket
KUNGFU_LOGGING_DIR=$KUNGFU_DATA_DIR/log
KUNGFU_PID_DIR=$KUNGFU_DATA_DIR/pid
KUNGFU_RUNTIME_DIR=$KUNGFU_DATA_DIR/runtime
KUNGFU_JOURNAL_DIR=$KUNGFU_DATA_DIR/journal

groupadd -f --system --gid 666 bruce
useradd -p `openssl passwd -1 master` -m -d $KUNGFU_HOME_DIR --system --uid 666 -g bruce -G bruce,wheel --shell=/bin/bash bruce

cat <<EOT > $KUNGFU_HOME_DIR/.bashrc
if [ -f /etc/bashrc ]; then
        . /etc/bashrc
fi
EOT
chown -R bruce:bruce $KUNGFU_HOME_DIR

mkdir -p $KUNGFU_SOCKETS_DIR

mkdir -p $KUNGFU_LOGGING_DIR
mkdir -p $KUNGFU_LOGGING_DIR/supervisor
mkdir -p $KUNGFU_LOGGING_DIR/wingchun
mkdir -p $KUNGFU_LOGGING_DIR/strategy

mkdir -p $KUNGFU_PID_DIR

mkdir -p $KUNGFU_RUNTIME_DIR

mkdir -p $KUNGFU_JOURNAL_DIR
mkdir -p $KUNGFU_JOURNAL_DIR/system
mkdir -p $KUNGFU_JOURNAL_DIR/strategy
mkdir -p $KUNGFU_JOURNAL_DIR/user
mkdir -p $KUNGFU_JOURNAL_DIR/MD/CTP
mkdir -p $KUNGFU_JOURNAL_DIR/MD_RAW/CTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD/CTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD_RAW/CTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD_SEND/CTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD_Q/CTP
mkdir -p $KUNGFU_JOURNAL_DIR/MD/XTP
mkdir -p $KUNGFU_JOURNAL_DIR/MD_RAW/XTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD/XTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD_RAW/XTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD_SEND/XTP
mkdir -p $KUNGFU_JOURNAL_DIR/TD_Q/XTP

chown -R bruce:bruce $KUNGFU_DATA_DIR

chmod +x /opt/kungfu/master/bin/*
ln -sf /opt/kungfu/master/bin/yjj /usr/bin/yjj
ln -sf /opt/kungfu/master/bin/wingchun /usr/bin/wingchun
ln -sf /opt/kungfu/master/bin/kungfuctl /usr/bin/kungfuctl

cat <<EOT > /usr/lib64/python2.7/site-packages/kungfu.pth
/opt/kungfu/master/lib/python2.7/site-packages
EOT

ln -sf /opt/kungfu/master/etc/systemd/user/kungfu.service /etc/systemd/system/kungfu.service

systemctl daemon-reload
systemctl start kungfu