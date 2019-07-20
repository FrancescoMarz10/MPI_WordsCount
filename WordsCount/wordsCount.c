#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "sequenziale.h"

//struttura per racchiudere le parole trovate ed i conteggi
typedef struct
{
  char word[30];
  int count;
} words;

//definizione delle firme dei metodi
void contaParole(char fname[200], int offset, int bound, words* partialArr, int rank);
MPI_Datatype define_MPI_type();



//main
int main(int argc, char** argv)
{
    //definizione variabili
    int rank;
    int num_processor;
    int max_size=0;
    FILE* fptr;
    char word[100];
    char nomeFile[20];
    double startTime;

    int num_files;

    //inizializzazione MPI
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processor);

    //istanza del tipo struttura per MPI.
    MPI_Datatype mpi_words_type = define_MPI_type();


    if(argc==1){
        //controllo inserimento dei parametri
        printf("Errore nell'inserimento dei parametri (specificare il numero di file da contare! Riprova!\n");
        
        MPI_Finalize();
        return 0;
    }
    
    else if(argc==2){
        //prendo dai parametri il numero di file che si vuole contare, e creo il nome del file da leggere.
        num_files= atoi(argv[1]);
        sprintf(nomeFile,"Files/list%d",num_files);
        strcat(nomeFile,".txt");
    }


      if(num_processor==1){
         //SEQUENZIALE
        //se è stato richiesto il lavoro di un solo processore richiamo il codice sequenziale, analizzandone il tempo.
        printf("Esecuzone in sequenziale\n");
        startTime= MPI_Wtime();

        //richiamo funzione calcolo sequenziale con parametro num_files.
        sequenziale(num_files);
       
        double endTime= MPI_Wtime();
        printf("TEMPO IMPIEGATO %f\n",endTime-startTime);
        MPI_Finalize();
        return 0;
      }

    char filenames[num_files][20];
    int offset[num_files][num_processor];

    if(rank == 0){

      fptr=fopen(nomeFile,"r");

      if(fptr==NULL){ 
            printf(" File does not exist or can not be opened."); 
            return 0;
        }
        else{
          startTime= MPI_Wtime();
          FILE *textfile;
            char stringa[300];

            int num_f = 0;

            //scorro il file all'interno del quale sono presenti i nomi dei file da contare e per ognuno di essi calcolo la mole di lavoro per ogni processore.
            while(fscanf(fptr, "%s", word) != EOF){
            
              sprintf(stringa,"Files/%s",word);
              textfile=fopen(stringa,"r");

              if(textfile == NULL){
                printf(" File does not exist or can not be opened."); 
                return 0;
              }
              else{

                strcpy(filenames[num_f],word);

                //calcolo delle porzioni da assegnare ad ogni processore in base al numero di parole presenti nel file.
                char wordfounded[30]="";
                int size=0;

                    //rilevo numero di parole totali.
                    while(fscanf(textfile,"%s",wordfounded)!=EOF){  
                        size++;
                    }
                    
                    //calcolo la massima dimensione del file, utile successivamente alla dimensione della struttura tramite malloc.
                    if(size>max_size){
                        max_size=size;
                    }

                    //calcolo della dimensione dell'offset grazie alla divisione del numero di parole per il numero di processori.
                    int dimensione_offset = size/num_processor;

                    //calcolo il modulo nel caso in cui la size non fosse divisibile per il numero di processori
                    int mod = size%num_processor;
 
                    //aggiorno la matrice offset inserendo l'offset di ogni processore per ogni file ed incrementando opportunamente per il modulo risultante.
                    for(int i=0;i<num_processor;i++){
                        offset[num_f][i]= (i < mod) ? dimensione_offset+1 : dimensione_offset;
                    }

                  fclose(textfile);
              }

              num_f++;
            }

        }

        fclose(fptr);
    
    }

    //invio in broadcast la massima taglia di file trovata per permettere una giusta allocazione delle risorse di memoria.
    MPI_Bcast(&max_size,1,MPI_INT,0,MPI_COMM_WORLD);

    int sizeArr = max_size*num_files;

    words* partialArr = malloc(sizeof(words)*sizeArr);

    //invio in broadcast la collezione contenente i nomi dei file, e gli offset sui quali sono destinati a lavorare i vari processori.
    MPI_Bcast(*filenames,num_files*20,MPI_CHAR,0,MPI_COMM_WORLD);
    MPI_Bcast(*offset,num_files*num_processor,MPI_INT,0,MPI_COMM_WORLD);
        
    MPI_Barrier(MPI_COMM_WORLD);

    words* globalArr= malloc(sizeof(words)*sizeArr);
    int lp=0, grandezzalista=0;

    //richiamo i metodi per il conteggio delle parole sulle rispettive porzioni.
    for(int i = 0; i < num_files; i++){

      if(rank == 0)
        contaParole(filenames[i], 0, offset[i][rank], partialArr, rank);
      else{
      int offSum=0;
            int boundSum=0;

            //calcolo offsum e boundsum, variabili che permettono di definire il punto di partenza e di arrivo di ogni processore.
            for(int pro=0;pro<rank;pro++){
                offSum = offSum+offset[i][pro];
            }

            for(int pro=0;pro<=rank;pro++){
                boundSum = boundSum+offset[i][pro];
            }

            contaParole(filenames[i],offSum,boundSum,partialArr,rank);
      }

        int lastposition=0;
            
        //raccolgo la restituzione dei dati calcolati in parallelo e ne effettuo l'aggregazione.
        if(rank==0){
          MPI_Status status;
                 
          for (int i = 0; i < num_processor; i++){
              //attendo la ricezione di lastposition, ovvero la variabile indicante il numero di parole differenti riscontrate dal processore durante l'analisi.
              MPI_Recv(&lastposition,1,MPI_INT,i,11, MPI_COMM_WORLD,&status);
            
              //ricevo le strutture parziali calcolate dai vari processori
              for(int j=0; j<lastposition;j++){            
                  MPI_Recv(&partialArr[j],1,mpi_words_type,i,11, MPI_COMM_WORLD,&status);
              }
              
              for (int proc = 0; proc < lastposition; proc++){

                  char stringa[30] ="";
                  strcpy(stringa,partialArr[proc].word);

                  //porzione di codice per l'aggiornamento della struttura globale, controllando la previa presenza della parola ed aggiornando soltanto il coounter, o inserendo una nuova parola.
                  for(int prox=0;prox<=lp;prox++){
                    
                      if((strcmp(partialArr[proc].word,globalArr[prox].word))==0){  
                        
                          int temp=globalArr[prox].count;
                          temp=temp+partialArr[proc].count;
                          globalArr[prox].count=temp;
                         
                          break;
                      }
                      else{
                        if(prox==lp){
                              
                              strcpy(globalArr[lp].word,stringa);
                              globalArr[prox].count=partialArr[proc].count;
                              lp++;
                              break;
                          }
                      }
                  }
              
                  //mantengo il conteggio della dimensione totale della struttura globale.
                  if(i==num_processor-1){
                      grandezzalista=lp;
                  }
              
              }
                
          }  
      }

      
    }
   
    if(rank==0){
        //calcolo delle tempistiche grazie MPI_Wtime().
        double endTime= MPI_Wtime();
        printf("TEMPO IMPIEGATO %f\n",endTime-startTime);
    }
    for(int i=0;i<grandezzalista;i++){
      printf("SONO %d Stringa %s con conto %d\n", rank, globalArr[i].word,globalArr[i].count );
    }

    
    MPI_Type_free(&mpi_words_type);
    MPI_Finalize();
    return 0;
}

void contaParole(char fname[20], int offset, int bound, words* partialArr,int rank){
    
    MPI_Datatype mpi_words_type= define_MPI_type();
    int lastposition=0;
    int flag=0;
    int wrd=0,charctr=0;
    char ch; 
    FILE *newFile;
    char stringa[300];
    sprintf(stringa,"Files/%s",fname);
    newFile=fopen(stringa,"r"); 
    
    if(newFile==NULL) 
     { 
         printf(" File does not exist or can not be opened."); 
      } 
    else{
          char w[30];
          int indice=0; 
          
          
          //ciclo while per lo scorrimento delle stringhe del file
           while(wrd<bound) {      
                
                fscanf(newFile,"%s",w);
                if(wrd>=offset){
                    //controllo per aggiungere  in struttura
                    for(indice=0;indice<=lastposition;indice++){
                        //controllo per capire se la parola nel file è già presente nella struttura, se si incremento il contatore
                        if(strcmp(partialArr[indice].word,w)==0){
                            partialArr[indice].count++;
                            break;

                            }
                        else{ 
                                //altrimenti se è stata controllata tutta la struttura e non è presente si aggiunge come nuovo campo e si pone il contatore ad 1
                                //aggioriamo il campo flag per aggiornare  il campo lastposition 
                                if(indice==lastposition){
                                    strcpy(partialArr[lastposition].word,w);
                                    partialArr[lastposition].count = 1;
                                    flag=1;
                                    
                                    break;
                                }
             
                            }
                            
                        }
                        
                    //controllo per l'aggiornamento della posizione dell'ultima aggiunta nella struttura
                    if(flag==1){
                        lastposition++;
                        flag=0;
                    }
                }
                //conteggio delle parole 
                wrd++; 
                     
            }
           
        
            MPI_Send(&lastposition,1,MPI_INT,0,11, MPI_COMM_WORLD);
           
            for(int i=0;i<lastposition;i++){
              MPI_Send(&partialArr[i],1,mpi_words_type,0,11, MPI_COMM_WORLD);
            
            }

            //Svuoto struttura parziale.
            for(int i=0;i<lastposition;i++){
              strcpy(partialArr[i].word,"");
              partialArr[i].count=0;
            }
            
            //MPI_Type_free(&mpi_words_type);
            fclose(newFile); 
    }


}

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
