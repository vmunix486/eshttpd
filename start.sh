#!/bin/sh

echo "Stopping eshttpd..."
sudo killall -9 eshttpd

echo "Starting eshttpd..."
sudo ./eshttpd
