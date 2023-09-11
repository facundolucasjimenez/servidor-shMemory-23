#include <stdio.h>       
#include <stdlib.h>
#include <unistd.h>   
#include <string.h>   
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include <signal.h>   

#include <sys/types.h>       
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/shm.h>

#define PUERTO 8888  // puerto de escucha del servidor 

#define CANTPAG 16
#define TAMPAG 4

using namespace std;

int crearShm(int) ;
char * vincularShm(int) ;
void mostrarShm(char *);

int main(int argc, char * argv[])
{
	
	signal(SIGINT,SIG_IGN);
    if (argc != 2 )
     {
       printf("Linea de entrada error\n");
       exit(-1) ;
     }
	
	int memoriaLocalDisponible;
	printf("Ingresar cantidad de memoria (ciclos locales): ");
    scanf("%d", &memoriaLocalDisponible);
	
	int ciclos;
    printf("Ingresar cantidad de ciclos de serie de fibonacci: ");
    scanf("%d", &ciclos);
	
	int i = 0;
	
	int siguiente;
	int anterior = 0;
	int actual = 1;
	
	int shmid = crearShm(0xa);
	char * dirbase = vincularShm(shmid);
	
	while (i<memoriaLocalDisponible && i<ciclos){
		*(dirbase+i)=actual;
		siguiente = anterior+actual;
		anterior = actual;
		printf("\n%d",actual);
		actual = siguiente;
		i+=1;
	}
	
	if (i!=ciclos){
				socklen_t addrlen ;
			 int sockclifd ;
			 struct sockaddr_in addrcli_in ; 
			 sockclifd = socket(AF_INET, SOCK_STREAM , 0);
			 
			 //completar la estrutura addr_in 
			 addrcli_in.sin_family = AF_INET ;
			 addrcli_in.sin_port   = htons(PUERTO) ;
			 addrcli_in.sin_addr.s_addr  = inet_addr(argv[1]) ;
			 memset(addrcli_in.sin_zero,0,8) ;
			  
			 addrlen = sizeof(addrcli_in);

			 printf("Soy el cliente y me conecto con el servidor\n");
			 int error = connect(sockclifd, (struct sockaddr *)&addrcli_in, addrlen);
			 if ( error == 0) // connect exitoso
				 {
					  char buffer[1024];
					  memset(buffer,'\0',1024);
					  int nb = recv(sockclifd,buffer,1024,0);
					  buffer[nb] = '\0';
					  printf("%s\n",buffer);
					  
					  
					  
						char str[80];   
						strcpy (str,std::to_string(actual).c_str());
						strcat (str,",");
						strcat (str,std::to_string(anterior).c_str());
						strcat (str,",");
						strcat (str,std::to_string(i).c_str());
						strcat (str, ",");
						strcat (str,std::to_string(ciclos).c_str());
						send(sockclifd,str,strlen(str),0);

						nb = recv(sockclifd,buffer,1024,0);
						buffer[nb] = '\0';
						printf("Mensaje recibido %s\n\n",buffer);
						
						mostrarShm(dirbase);
					  
					  printf(" ***** Cliente Terminado *****\n");
				 }
			 close(sockclifd);
	}
          
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