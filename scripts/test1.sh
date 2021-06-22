#!/bin/bash

echo "Test 01"

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server &
pid=$!

echo "config"

sleep 3s

./client -f /tmp/server_sock -o O_CREATE-O_LOCK:minni.txt -W minni.txt -t 200 -p

./client -h 

sleep 1s

echo $pid
kill -s SIGHUP $pid
wait $pid