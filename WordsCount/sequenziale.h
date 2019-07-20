#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#define ARRAY 70000

void trovaFiles(int numero_files);
void findWords(char fname[200]);

//struttura per racchiudere le parole trovate ed i conteggi
struct wordsNew
{
  char word[100];
  int count;
} wordsNew;

//VARIABILI GLOBALI, utilizzabili durante tutte le esecuzioni.
int lastposition=0;
int flag=0;
int value=1;

int sequenziale(int numero_files) {
	
	//richiamo della funzione per la ricerca dei file e l'avvio del conteggio
	trovaFiles(numero_files);

	return EXIT_SUCCESS;
}

void trovaFiles(int numero_files){

	
	char names[numero_files][20];
	FILE *newFile;
	char parola[100];
	char nomeFile[25];
 	sprintf(nomeFile,"Files/list%d",numero_files);
	strcat(nomeFile,".txt");

	newFile=fopen(nomeFile,"r");
	if(newFile==NULL){
		 printf("Errore nell'apertura del file.");
		 exit -1;
	} 
	else{
		//scorrimento e salvataggio di tutti i nomi dei files nella matrice names.
		int i=0;
		while(i<numero_files){  
		 	fscanf(newFile,"%s",parola);
		 	strcpy(names[i],parola);
		 	i++;
		}

}
		//richiamo della funzione per il conteggio delle parole di ogni file
		for(int i=0;i<numero_files;i++){
				printf("%s\n",names[i]);
				findWords(names[i]);
		}
			
}

void findWords(char fname[200]) 
{ 
    FILE *fptr; 
    char ch; 
    int wrd=1,charctr=1;
    char stringa[1000];
    sprintf(stringa,"Files/%s",fname);
    struct wordsNew arr[ARRAY]; 
  	
  	//apertura del file in lettura
    fptr=fopen(stringa,"r"); 


    if(fptr==NULL) 
     { 
         printf(" File does not exist or can not be opened."); 
         exit -1;
      } 
    else  { 
     	  
     	  char w[100];
		  int indice=0;	
      	  
      	  
      	  //ciclo while per lo scorrimento delle stringhe del file
      	  while(fscanf(fptr,"%s",w)!=EOF){  	
            
                //controllo per aggiungere  in struttura
                for(indice=0;indice<sizeof(arr);indice++){
 					
 					//controllo per capire se la parola nel file è già presente nella struttura, se si incremento il contatore
                  	if(strcmp(arr[indice].word,w)==0){
                   		arr[indice].count++;
                   		break;

                   		}
                   	else{ 
                   			//altrimenti se è stata controllata tutta la struttura e non è presente si aggiunge come nuovo campo e si pone il contatore ad 1
                   			//aggioriamo il campo flag per aggiornare  il campo lastposition
	                   		if(indice==lastposition){
	                   			strcpy(arr[lastposition].word,w);
		                   		arr[lastposition].count = 1;
	                    		flag=1;
	                   		}
	     
                   		}
                    	
                    }
                   	
				//controllo per l'aggiornamento della posizione dell'ultima aggiunta nella struttura
				if(flag==1){
					lastposition++;
					flag=0;
				}
				
				//conteggio delle parole 
				wrd++; 
                     
            }
       
       	 int q=0;
       	 int conto=0;

       	 //ciclo per la stampa della struttura dopo ogni esecuzione
         while(strcmp(arr[q].word,"")!=0){
        	//printf("STRINGA:%s, COUNT: %d\n",arr[q].word, arr[q].count);
        	conto+=arr[q].count;
        	q++;
        }

        //stampa del numero di parole totale e del conteggio delle ripetizioni
        //printf("\nNUM PAROLE: %d, COUNT: %d\n",q, conto);
   		value++;

        } 
    fclose(fptr); 
}

