#!/bin/bash

echo "Test 1"

  ./server config2 &
pid=$!



sleep 3s

./client /test/test1 -f /tmp/server_sock -o O_CREATE-O_LOCK:minni.txt -W minni.txt -r minni.txt -t 200 -p
./client /test/test1 -f /tmp/server_sock -O O_CREATE-O_LOCK,0:test_w -w test_w -R,2 -d test-d -t 200 -p
./client /test/test1 -f /tmp/server_sock -o O_CREATE:paperino.txt -a paperino.txt:PAPERO -r paperino.txt -C paperino.txt -r paperino.txt  -t 200 -p
./client /test/test1 -f /tmp/server_sock -G test_w -r pippo.txt -t 200 -p
./client /test/test1 -f /tmp/server_sock -o NO_FLAGS:pippo -r pippo.txt -W pippo.txt -c pippo.txt -r pippo.txt -t 200 -p
./client /test/test1 -f /tmp/server_sock -t 200 -p -o O_CREATE-O_LOCK:pippo.txt -W pippo.txt -a pippo.txt:"TESTO TO APPEND SU FILE_PIPPO_9999" -r pippo.txt
./client /test/test1 -h 

sleep 2s

echo $pid
kill -s SIGHUP $pid
wait $pid