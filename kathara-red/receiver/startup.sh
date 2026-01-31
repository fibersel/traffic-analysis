#!/bin/sh

echo "RECEIVER STARTUP EXECUTED" > /tmp/startup_receiver.log

ip addr add 10.0.1.2/24 dev eth0
ip link set eth0 up
ip route add default via 10.0.1.1