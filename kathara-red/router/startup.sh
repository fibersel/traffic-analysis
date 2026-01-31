#!/bin/sh

echo "ROUTER STARTUP EXECUTED" > /tmp/startup_router.log

# Включаем маршрутизацию
sysctl -w net.ipv4.ip_forward=1

# IP-адреса
ip addr add 10.0.0.1/24 dev eth0
ip addr add 10.0.1.1/24 dev eth1

ip link set eth0 up
ip link set eth1 up

# Очистка старых очередей
tc qdisc del dev eth1 root 2>/dev/null

# RED
tc qdisc add dev eth1 root red \
  limit 100000 \
  min 3000 \
  max 9000 \
  avpkt 1000 \
  burst 20 \
  bandwidth 10mbit \
  probability 0.02