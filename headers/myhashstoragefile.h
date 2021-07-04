#ifndef MYHASHSTORAGEFILE_H_
#define MYHASHSTORAGEFILE_H_


#include <utils.h>

#define NUMERO_PRIMO_ENORME 93199

//struttura dati di rappresentazione di un file nello storage file
typedef struct node{
	int fd; //descrittore specifico del file, memorizzato all'apertura del file, altrimenti -2
	char* abs_path; //path assoluto univoco del file
	void* content; //contenuto del file
	size_t dim_bytes; //dimensione in bytes del file
	int modified_flag; //flag di modifica, se il file è stato modificato == 1, altrimenti 0
	int open_flag; //flag di apertura, se il file è aperto == 1, altrimenti 0
	int o_create_flag;  //flag di creazione, se il file è stato aperto con O_CREATE == 1, altrimenti 0
	int locked_flag;  //flag di lock, se il file è lockato == 1, altrimenti 0
	int inCache_flag; //flag di cache, se il file è situato nella cache dello storage file == 1, altrimenti 0
	struct node *next;
    struct node *prec;
}file_t;

//struttura dati per l'implementazione di una lista di file_t
typedef struct _list{
	int size; //cardinalità della lista
	size_t dim_bytes; //dimensione in bytes della somma della dimensione dei file nella lista
	file_t *head;
	file_t *tail;
}list_file;

typedef struct _hash{
	int len; //lunghezza della hash, numero di liste di trabocco della hash
	size_t memory_used; //memoria utilizzata in bytes
	size_t memory_used_from_modified_files; //memoria usata da file modificati in bytes
    size_t memory_capacity; //capacità massima della hash in bytes
	int n_file;	//cardinalità attuale della hash
	int n_files_free; // cardinalità dei file "disponibili" (file aperti e non lockati) della hash
	int n_file_modified; // cardinalità dei file modificati nella hash
	int max_n_file; // massima cardinalità della hash
    int max_n_file_cache; // massima cardinalità della cache

	int stat_max_n_file; // statistica della cardinalità massima raggiunta nella hash
	size_t stat_dim_file; //statistica della massima dimensione raggiunta in bytes della hash
	int stat_n_replacing_algoritm; // numero di volte che l'argoritmo di rimpiazzamento è andato a buon fine, e ha rimpiazzato uno o più file
	
	list_file *cell; //liste di trabocco della hash
	list_file cache; //cache della hash, (lista diversa dalle liste di trabocco della hash)
}hashtable;



////////////inizializzazioni server:

//inizialiazzazione di file_t
file_t* init_file(char *namefile);

//scrittura della variabile "content" del file_t f, scrivendo il contenuto in esso (tramite writeFile)
int writeContentFile(file_t* f);

//append al content del file_t, anche se content == NULL (quindi si può inizializzare la variabile content con questo metodo)
void appendContent(file_t * f,void *buff,size_t size);

//inizializzazione di list_file l
void init_list(list_file* l);

//inizialiazzazione della hashtable tramite le informazioni fornite dalla variabile configurazione_server
void init_hash(hashtable *table, config configurazione_server);

//funzione hash
int hash(hashtable table,char *namefile);

///////////operzioni su liste:

//inserimento in coda della list_file cell del file_t file
void ins_tail_list(list_file *cell,file_t *file);

//inserimento in testa della list_file cell del file_t file
void ins_head_list(list_file *cell, file_t*file);

//rimozione in testa della list_file cell, ritorna la testa della lista espulsa da cell
file_t* pop_head_list(list_file *cell);

//rimozione in coda della list_file cell, ritorna la coda della lista espulsa da cell
file_t* pop_tail_list(list_file *cell);


////////insermento file nel server

//inserisci il file_t file in testa della cache
void ins_file_cache(hashtable *table,file_t* file); 

//inserisci il file_t file nella hash
void ins_file_hashtable(hashtable *table, file_t* file);

////ins e rimozione file server seguendo politica lru

/*	
	inizializzazione del file_t f (come se fosse una dichiarazione del file_t f, senza quindi contenuto e dimesione in bytes) nella hash,
	viene fatto un controllo sulla cardinalità della hash. In caso la hash sia piena, 
	viene trovato un file_t vittima da espellere dalla hash per far posto al nuovo file, seguendo una politica LRU.
	Il file_t vittima viene messo in coda alla list_file list_reject. Il metodo ritorna 1, se il file è stato inizializzato nella hash, 0 altrimenti.

*/
int init_file_inServer(hashtable* table,file_t *f,list_file* list_reject);

/*	
	scrittura del contenuto del file_t f già presente nella hash,
	viene fatto un controllo sulla cardinalità e sulla capacità massima di memoria della hash. In caso la hash sia piena, 
	viene trovato uno o più file_t vittima da espellere dalla hash per far posto al contenuto del file_t f, seguendo una politica LRU.
	I file_t vittima vengono messi in coda alla list_file list_reject. Il metodo ritorna 1, se il file è stato scritto nella hash, 0 altrimenti.

*/
int ins_content_file_server(hashtable* storage, file_t *f,list_file* list_reject);

/*
	aggiunta di dimensione (size_inplus) del file_t f nella hash.
	viene fatto un controllo sulla capacità massima di memoria della hash. In caso la hash sia piena, 
	viene trovato uno o più file_t vittima da espellere dalla hash per far posto al contenuto del file_t f, seguendo una politica LRU.
	I file_t vittima vengono messi in coda alla list_file list_reject. Il metodo ritorna 1, se il file è stato scritto nella hash, 0 altrimenti.
*/
int modifying_file(hashtable* table,file_t* f,size_t size_inplus,list_file* list_reject);

//rimozione del file_t f dalla hash
file_t* remove_file_server(hashtable* table, file_t* f);

//estrazione del file_t file dalla hash
void extract_file_from_server(hashtable *table,file_t* file);

//estrazione del file_t file dalla list_file cell
void extract_file_from_list(list_file* cell,file_t* file);

//aggiorna la posizione del file_t file, lo estrae dalla sua posizione attuale e lo mette in cima alla cache
void update_file(hashtable *table,file_t* file);

//ricerca file_t, tramite il paramentro namefile cerca nella list_file un file con l' abspath uguale al namefile, ritorna il file_t o NULL in caso di fallimento
file_t* research_file_list(list_file cell,char* namefile);

//ricerca file_t, tramite il paramentro namefile cerca nella hash un file con l' abspath uguale al namefile, ritorna il file_t o NULL in caso di fallimento
file_t* research_file(hashtable table,char* namefile);


////////utilità:

//controlla se il file_t file è contenuto nella hash, ritorna 1 se è contenuto, 0 altrimenti
int isContains_hash(hashtable table, file_t* file);

//controlla se la list_file cella è vuota, 1 se è vuota, 0 altrimenti
int isEmpty(list_file cella);

//controlla se la cache è piena( a livello di cardinalità), ritorna 1 se è piena, 0 altrimenti
int isCacheFull(hashtable table);

//////frees:

//libera memoria del file_t
void free_file(file_t* file);

//libera memoria della list_file l
void free_list(list_file* l);

//libera memoria della hash
void free_hash(hashtable* table);

//stampa a schermo una list_file in modo ricorsivo, passando la testa della list_file
void print_list(file_t* head);

//stampa a schermo la hash
void print_storageServer(hashtable table);

//inserisce in coda della list_file l, tutti i file della list_file l2 svuotandola completamente ( i file rimossi da l2 vengono rimossi dalla testa della lista) 
void concatList(list_file *l,list_file *l2);



#endif