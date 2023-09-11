#include <stdio.h>       
#include <stdlib.h>
#include <pthread.h>      
#include <unistd.h>   
#include <string.h>

#include <sys/types.h>       
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define PUERTO 8888

#define CANTPAG 16
#define TAMPAG 4

int crearShm(int) ;
char * vincularShm(int) ;
void inicializarShm(char *);
void mostrarShm(char *);
void inicializarVecPag(char * , char * []);
void mostrarVecPag(char * []);
void mostrarShmDesdeVec(char * []);

void atender(void *) ;

char * vecPag[CANTPAG];
char * dirbase;

int main(int argc, char * argv[])
{
	if (argc != 2 )
     {
       printf("Linea de entrada error\n");
       exit(-1) ;
     }
	 socklen_t addrlen ;
     int sockfd ;
     struct sockaddr_in addr_in , addrcli_in ; 
     sockfd = socket(AF_INET, SOCK_STREAM , 0);
     printf("socket = %d\n",sockfd);
     
     //completar la estrutura addr_in 
     addr_in.sin_family = AF_INET ;
     addr_in.sin_port   = htons(PUERTO) ;
     addr_in.sin_addr.s_addr  = inet_addr(argv[1]) ; // inet_addr convierte char * a valor entero con formato de ip, le paso la ip pasada en la terminal
     memset(addr_in.sin_zero,0,8) ; // esto no hace falta
      
     addrlen = sizeof(addr_in);
     int bn = bind(sockfd, (struct sockaddr *)&addr_in , addrlen); // bind asocia el struct con el socket. Ya castea sockaddr_in a sockaddr
     printf("bind = %d\n",bn); // si es 0 está todo piola

     int lst = listen(sockfd,5) ; // listen marca al socket como pasivo (va a usar accept), 2do parámetro es tamaño de cola
     printf("listen = %d\n",lst);

     int shmid = crearShm(0xa) ; 
     dirbase = vincularShm(shmid) ;
     inicializarShm(dirbase) ;
     mostrarShm(dirbase) ;
     //inicializarVecPag(dirbase, vecPag);
     //mostrarVecPag(vecPag);
     //mostrarShmDesdeVec(vecPag);

     while(1)
     {
       printf("servidor esperando conexion del cliente\n");
       int sockclifd = accept(sockfd, (struct sockaddr *)&addrcli_in, &addrlen);
       if ( sockclifd > 0)
         {
            pthread_t hilo ;
    pthread_create(&hilo,NULL,(void*)&atender,(void*)&sockclifd);                      
         }
     }     
	
}


void atender(void * sockclifd) 
{
   printf("sockclifd = %d\n",*((int*)sockclifd));
   int * sockcli = (int *) sockclifd ;
   char buffer[1024];

   memset(buffer,'\0',1024);
   strcpy(buffer,"El Servidor de SHM va a continuar el proceso\0");
   send(*sockcli, buffer, strlen(buffer),0);


   memset(buffer,'\0',1024);
   int nb = recv(*sockcli, buffer, 1024, 0);
   buffer[nb] = '\0';
   
   int actual = atoi(strtok(buffer,","));
   int anterior = atoi(strtok(NULL,","));
   int indice = atoi(strtok(NULL,","));
   int veces = atoi(strtok(NULL,"\n"));
   
   char* resto[1024];
   memset(resto, '\0', 1024);
   
   char* buffer2[1024];
   memset(buffer2, '\0', 1024);
   
   int proximo;
   
   while (indice<veces){
	   *(dirbase+indice) = actual;
	   proximo = actual+anterior;
	   anterior = actual;
	   sprintf(buffer2,"\n%d",actual);
	   strcat(resto, buffer2);
	   actual = proximo;
	   indice+=1;
   }

   send(*sockcli, resto, strlen(resto),0);
   printf("Mensaje enviado --> %s\n",resto);
   
   mostrarShm(dirbase);
   
   close(*sockcli);
   pthread_exit(NULL);
}

int crearShm(int key) 
{
   int shmid = shmget(key,CANTPAG*TAMPAG,IPC_CREAT|IPC_EXCL|0600);
   if (shmid == -1)
     shmid = shmget(key,0,0);
   return shmid ;
}

char * vincularShm(int shmid) 
{
   char * dirbase = (char *) shmat(shmid,0,0);
   return dirbase ;
}

void inicializarShm(char * dirbase)
{
   char letra = '0' ;
   int i ;
   for(i = 0 ; i < (CANTPAG*TAMPAG) ; i++)
    {
      *(dirbase+i) = 0 ;
      //letra++ ;
    }
}

void mostrarShm(char * dirbase)
{
   int i ;
   for(i = 0 ; i < (CANTPAG*TAMPAG) ; i++)
    {
      printf(" %d",*(dirbase+i)) ;
      if (!((i+1)%TAMPAG))
        printf("\n");
    }
}

void inicializarVecPag(char * dirbase, char * vecPag[])
{
    int i ;
    int j = 0;
    for(i = 0 ; i < (CANTPAG*TAMPAG) ; i++)
       {
          if (!(i%TAMPAG))
             {
               vecPag[j] = dirbase+i ;
               j++ ;
             }  
       }
}

void mostrarVecPag(char * vecPag[])
{
       int j ;
       for(j = 0 ; j < CANTPAG ; j++)
         printf(" vecPag[%d]=%p\n",j,vecPag[j]) ;
       printf("\n");
}

void mostrarShmDesdeVec(char * vecPag[])
{
         int j ;
         for(j = 0 ; j < CANTPAG ; j++)
           {
              write(1,vecPag[j],TAMPAG) ;
              printf("\n");
           }
}