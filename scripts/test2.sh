#!/bin/bash

echo "Test 2"
./server config2 &
pid=$!


sleep 3s
#test sull'overflow di memoria
./client /test/test2 -f /tmp/server_sock -O O_CREATE-O_LOCK,3:test_overflow_bytes -w test_overflow_bytes,3 -p
./client /test/test2 -f /tmp/server_sock -a albero.jpg:ALBERO -o O_CREATE-O_LOCK:riven.jpg -W riven.jpg -D test_D -p

#test sul numero massimo file memorizzati nel server
for i in {0..9}; do
    ./client /test/test2 -f /tmp/server_sock -o O_CREATE:testOverflow_0$i.txt -a testOverflow_0$i.txt:APPENDO$i -p
done

sleep 2s

kill -s SIGHUP $pid
wait $pid