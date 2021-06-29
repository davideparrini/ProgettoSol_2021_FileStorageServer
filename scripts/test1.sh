#!/bin/bash

echo "Test 1"

valgrind --leak-check=full ./server config &
pid=$!



sleep 3s

#tutte le operazioni eseguite ritornano con successo correttamente
./client /test/test1 -f /tmp/server_sock -o O_CREATE-O_LOCK:minni.txt -W minni.txt -r minni.txt -d test-d -t 200 -p

#tutte le operazioni eseguite ritornano con successo correttamente, ovviamente l'apertura e la scrittura minni.txt sono ignorate perche è già aperto e scritto il file
./client /test/test1 -f /tmp/server_sock -O O_CREATE-O_LOCK:test_w -w test_w -R,2 -d test-d -t 200 -p

#tutte le operazioni eseguite ritornano con successo correttamente,eccetto l'ultima -r paperino.txt risulta fallire correttamente (tende a sottolineare il funzionamento corretto dell'opzione di chiusara file '-C')
./client /test/test1 -f /tmp/server_sock -o O_CREATE:paperino.txt -a paperino.txt:PAPERO -r paperino.txt -C paperino.txt -r paperino.txt  -t 200 -p

#l'opzione -G chiude con successo correttamente tutti i file della directory test_w, ovviamente l'opzione -r fallisce perchè il file pippo.txt è stato appena chiuso
./client /test/test1 -f /tmp/server_sock -G test_w -r pippo.txt -t 200 -p

#tutte le operazioni eseguite ritornano con successo correttamente, eccetto dell'ultima -r (fallisce) che tende a sottolineare il corretto funzionamente dell'opzione -c (rimozione del file)
./client /test/test1 -f /tmp/server_sock -o NO_FLAGS:pippo.txt -r pippo.txt -W pippo.txt -c pippo.txt -r pippo.txt -t 200 -p

#tutte le operazioni eseguite ritornano tutte con successo correttamente, in particolare O_CREATE-O_LOCK:pippo.txt sottolinea il fatto che il file pippo rimosso in precedenza con l'opzione '-c' ha avuto un buon esito
./client /test/test1 -f /tmp/server_sock -t 200 -p -o O_CREATE-O_LOCK:pippo.txt -W pippo.txt -a pippo.txt:"TESTO TO APPEND SU FILE_PIPPO_9999" -r pippo.txt

#l'opzione -h funziona correttamente chiudendo il client senza eseguire le seguenti operazioni
./client /test/test1 -h  -f /tmp/server_sock -t 200 -p -o O_CREATE-O_LOCK:PossoScrivereQuelloCheVoglio,tantoIlClientSiChiuderàConLOpzione-h.txt

sleep 2s

kill -s SIGHUP $pid
wait $pid