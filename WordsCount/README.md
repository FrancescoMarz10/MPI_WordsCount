# Words Count

Il seguente progetto si propone di calcolare la ricorrenza delle parole all'interno dei file analizzati, evitando possibili ripetizioni.

## Esecuzione

All'interno della directory viene presentato il file contenente il codice sorgente della soluzione al problema analizzato. Il file wordsCount.c infatti, mostra una corretta implementazione delle funzionalità richieste. Per l'esecuzione di tale programma è necessario specificare il numero di file che si vogliono considerare per il conteggio delle parole in essi contenuti. Risulta necessario, inoltre, per il corretto funzionamento del programma, la presenza (su ogni processore) del file "sequenziale.h", ove viene specificato il codice per lo svolgimento con un singolo processore, e la cartella "Files", all'interno del quale verranno inseriti tutti i file da esaminare e i file listNUM.txt, dove NUM rappresenta il numero di files contenuti e in cui saranno presenti i nomi di tutti i file da conteggiare.

All'interno del package (più precisamente nella cartella Files) per agevolare i test è stato inserito uno script, fileGenerator1.java, ed un dizionario word.txt, con lo scopo di generare file con parole casuali della dimensione inserita all'interno del programma. Tale script crea un file denominato 'fileGenerato.txt'. Dunque, nel caso in cui si desideri generare più file, è necessario rinominare quello appena creato ed aggiornare i file list corrispondenti. Una volta terminata tale procedura sul master, la cartella Files può essere compressa ed inviata agli slaves.

Per effettuate la compilazione del programma è necessario eseguire sul cloud AWS il seguente comando:
```
mpicc -fopenmp wordsCount.c -o wordsCount
```

Dopo aver compilato, diviene possibile l'esecuzione del programma, all'interno del quale viene contenuta sia la versione sequenziale (selezionabile grazie alla richiesta di utilizzo di un unico processore) sia la versione parallela. Inoltre, come specificato in precedenza, risulta fondamentale specificare il numero di file da leggere ed esaminare. 

L'esecuzione potrà realizzarsi tramite la seguente istruzione:
```
mpirun -np 1 --hostfile slavesfile ./wordsCount numero_file
```
## Implementazione
Avendo ottenuto il numero di file, diviene necessario, per il lavoro parallelo dei processori, scomporre i file così da assegnare carichi di lavoro simili a tutti i processori selezionati. La suddivisione è stata realizzata come segue. Uno dei processori, ovvero il master, scorre un file alla volta contando il numero di parole in essi presenti. Individuata tale taglia, divide questo valore per il numero di processori in modo tale da assegnare la stessa quantità di parole ad ognuno di essi. 
Nel caso si presenti una divisione non perfetta, dunque con presenza di resto, tale valore viene diviso equamente tra i processori. In tal modo, la divisione del carico risulta ben bilanciata.
Il carico computato verrà conservato all'interno di una matrice mostrante l'offset di ogni processore per ogni file.
Di seguito mostriamo la porzione di codice che si occupa di tale operazione:
```
char wordfounded[30]="";
int size=0;
     while(fscanf(textfile,"%s",wordfounded)!=EOF){  
            size++;
     }
                    
     if(size>max_size){
            max_size=size;
     }

     int dimensione_offset = size/num_processor;
     int mod = size%num_processor;
 
     for(int i=0;i<num_processor;i++){
            offset[num_f][i]= (i < mod) ? dimensione_offset+1 : dimensione_offset;
     }
```

La comunicazione delle porzioni dedicate ad ogni processore avviene tramite l'operazione di Bcast, grazie alla quale il master invia l'intera matrice contenente gli offset a tutti i processori. Allo stesso modo, grazie al medesimo metodo, il master comunica l'insieme dei file da analizzare.

In tal modo, master e slaves conosceranno non solo i file su cui dovranno esercitare le funzionalità programmate, ma anche quale porzione relativa ad ogni file andare a considerare.

Tutti i conteggi saranno gestiti grazie alla presenza di due array di strutture, globalArr, posseduto dal master per il contenimento e l'aggregazione dei risultati finali, e partialArr, del quale una copia viene mantenuta da ognuno dei processori per conservare i dati parziali. 
 
La struttura utilizzata per il contenimento dei valori viene organizzata come segue:
```
typedef struct
{
  char word[30];
  int count;
} words;
```
Contiene un campo "word", ovvero la stringa ritrovata, ed un campo "count" per il numero di riscontri.

Una volta terminate le operazioni di conteggio, la struttura viene spedita al master, per l'integrazione finale dei risultati. Per fare ciò è stato necessario creare un nuovo metodo, ovvero define_MPI_type(), il quale si occupa di definire un tipo apposito per l'invio della struttura tramite l'operazione di send.
Di seguito è riportato il metodo sviluppato.
```
MPI_Datatype define_MPI_type(){
  //DEFINIZIONE TIPO PER INVIARE STRUTTURA DATI
 
  const int nitems=2;
  int blocklengths[2] = {30,1};
  MPI_Datatype types[2] = {MPI_CHAR, MPI_INT};
  MPI_Datatype mpi_words_type;
  MPI_Aint     offsets1[2];
 
  offsets1[0] = offsetof(words, word);
  offsets1[1] = offsetof(words, count);

  MPI_Type_create_struct(nitems, blocklengths, offsets1, types, &mpi_words_type);
  MPI_Type_commit(&mpi_words_type);
  //fine definizione tipo

  return mpi_words_type;

}
```
## Risultati del benchmarking
Gli esperimenti sono avvenuti tramite l'utilizzo di otto istanze di tipologia "t2.small", ognuna formata da un'unica CPU, di Amazon Web Services. I test sono stati realizzati su un numero di processori variabile (da uno a otto) e sono stati ripetuti cinque volte per istanza, così da ottenere dati più specifici grazie ad una media dei valori risultanti.

Il calcolo delle tempistiche prevede anche il conteggio della sezione riguardante la suddivisione dei compiti per i vari processori, oltre che alla successiva esecuzione.

La rappresentazione dei tempi è rappresentata in **secondi** ed è stata utilizzata l'apposita funzione di MPI, ```MPI_Wtime()```, per la loro raccolta.

I test, inoltre, verranno realizzati tramite due tipologie di test in grado di verificare lo Strong Scaling e il Weak Scaling.

## Strong Scaling 
Lo Strong Scaling viene realizzato mantenendo fisse le dimensioni del problema e incrementando il numero di processori che dividono il carico lavorativo. L'obiettivo di tale tipologia di test è ridurre al minimo il tempo necessario alla soluzione di un determinato problema.

***Calcolo dello speedup***: nello strong scaling il calcolo dello speedup è dettato dalla formula **t1 / (N x tN)**, dove *t1* è il tempo impiegato per completare un'unità di lavoro con un unico elemento di elaborazione e *tN* è il tempo impiegato per completare la medesima unità di lavoro con *N* unità di elaborazione. Per una migliore lettura dei risultati, il risultato di tale formula è stato espresso in percentuale.

I test sono stati effettuati, per ognuna delle istanze pensate, su dei file contententi un milione di parole, in quanto da questa dimensione in poi, i valori delle differenti esecuzioni iniziano a mostrare maggiore risalto.

Di seguito vengono riportati i risultati, in media, delle cinque esecuzioni fatte su ogni tipologia di istanza sia in una tabella che in un grafico:


| N. Files      | N. Core | Tempo (s)   | Speedup | 
| :------------:| :-----: | :---------: | :------:|
| 8             | 1       | 12,6857s    | x       |
| 8             | 2       | 7,4262s     | 85,41%  |
| 8             | 3       | 5,7820s     | 73,13%  |
| 8             | 4       | 4,5586s     | 69,57%  |
| 8             | 5       | 3,9866s     | 63,64%  |
| 8             | 6       | 3,5498s     | 59,56%  |
| 8             | 7       | 3,2379s     | 55,96%  |
| 8             | 8       | 3,1162s     | 50,88%  |

<img src="https://i.imgur.com/WvZ1Yk4.jpg" title="MC_weak" height="350px"/>

In conclusione, dai risultati visibili dello Strong Scalability, si osserva come con l'analisi di otto file composti da un milione di parole, utilizzare più di tre processori non risulta avere enormi vantaggi, in quanto il miglioramento delle prestazioni non risulta essere troppo evidente. Naturalmente la situazione potrebbe evolversi diversamente con dimensione e numero diversi di file. 


##Weak Scaling
Il Weak Scaling prevede il mantenimento del lavoro per lavoratore fisso, con il successivo aumento dei lavoratori, così da ottenere un aumento della taglia totale del problema. L'obiettivo di tale tipologia di testing è la risoluzione di problemi di taglia maggiore.

***Calcolo dello speedup***: nel weak scaling il calcolo dello speedup è dettato dalla formula **t1 / tN**, dove *t1* è il tempo impiegato per completare un'unità di lavoro con un unico elemento di elaborazione e *tN* è il tempo impiegato per completare *N* delle medesime unità di lavoro con *N* unità di elaborazione. Per una migliore lettura dei risultati, il risultato di tale formula è stato espresso in percentuale. 

Di seguito vengono riportati i risultati, in media, delle cinque esecuzioni fatte su ogni tipologia di istanza sia in una tabella che in un grafico:

| N. File       | N. Core | Tempo (s)   | Speedup  | 
| :------------:| :-----: | :---------: | :------: |
| 1             | 1       | 1,7954s     | x        |
| 2             | 2       | 1,9010s     | 94,44%   |
| 3             | 3       | 2,1108s     | 85,05%   |
| 4             | 4       | 2,3141s     | 77,58%   |
| 5             | 5       | 2,4753s     | 72,53%   |
| 6             | 6       | 2,6370s     | 68,08%   |
| 7             | 7       | 2,8400s     | 63,21%   |
| 8             | 8       | 3,1526s     | 56,94%   |

<img src="https://i.imgur.com/QjX5JgG.jpg" title="MC_weak" height="350px"/>

Dai risultati ottenuti grazie al Weak Scaling, è visibile come l'algoritmo risulti scalare più o meno linearmente all'aumentare dei processori.
