/* A simple server in the internet domain using TCP.
myServer.c
D. Thiebaut
Adapted from http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
The port number used in 51717.
This code is compiled and run on the Raspberry as follows:

    g++ -o myServer myServer.c
    ./myServer

The server waits for a connection request from a client.
The server assumes the client will send positive integers, which it sends back multiplied by 2.
If the server receives -1 it closes the socket with the client.
If the server receives -2, it exits.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include <time.h>

void error( char *msg ) {
  perror(  msg );
  exit(1);
}

int func( int a ) {
   return 2 * a;
}

void sendData( int sockfd, int x ) {
  int n;
  char buffer[32];
  sprintf( buffer, "%d\n", x );
  if ( (n = write( sockfd, buffer, strlen(buffer) ) ) < 0 )
    error( const_cast<char *>( "ERROR writing to socket") );
  buffer[n] = '\0';
}

int getData( int sockfd ) {
  char buffer[32];
  int n;

  if ( (n = read(sockfd,buffer,31) ) < 0 )
    error( const_cast<char *>( "ERROR reading from socket") );
  buffer[n] = '\0';
  return atoi( buffer );
}

char* getDataT( int sockfd ) {
  char buffer[1024];
  int n;

  if ( (n = recv(sockfd,buffer,1024,0) ) < 0 )
    error( const_cast<char *>( "ERROR reading from socket") );
  buffer[n] = '\0';
  return buffer;
}

int main(int argc, char *argv[]) {

     //int sockfd, newsockfd, portno = 51717, clilen;
     int sockfd, newsockfd, portno = 8004, clilen;

     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int data = 0;
     char* dataT;

     printf( "using port #%d\n", portno );

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
         error( const_cast<char *>("ERROR opening socket") );
     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons( portno );
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
       error( const_cast<char *>( "ERROR on binding" ) );
     listen(sockfd,5);
     clilen = sizeof(cli_addr);

	 pid_t parent = -1;  // LIFO
	 pid_t pid;
	 int forkin = 1;
	 int times = 0;
	 
     //--- infinite wait on a connection ---
     while ( 1 ) {
        printf( "waiting for new client...\n" );
        if ( ( newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, (socklen_t*) &clilen) ) < 0 )
            error( const_cast<char *>("ERROR on accept") );
        printf( "opened new communication with client newsockfd %d\n", newsockfd );
        while ( 1 ) {

             //---- wait for a number from client ---
             /*data = getData( newsockfd );
             printf( "got %d\n", data );

             if ( data < 0 )
                break;*/
             data = 0;
             dataT = getDataT( newsockfd );   
			 printf( "got %s\n", dataT );

			/*
				CIEL
				RECIBIMOS JSON
			*/
			rapidjson::Document d;
			d.Parse<0>(dataT);
			printf("pondre %d veces entre %s segundos el archivo %s\n", d["NumeroReproducciones"].GetInt(), d["TiempoEntreReproducciones"].GetString(), d["TonoReproduccion"].GetString());
			times = d["NumeroReproducciones"].GetInt();
			struct tm lude; // TIMESTAMP                                                                                                                                   
   			strptime(d["TiempoEntreReproducciones"].GetString(), "%T", &lude);
   			
             //data = func( data );
             data = 1;

             //--- send new data back ---
             printf( "sending back %d\n", data );
             sendData( newsockfd, data );

			/*
				CIEL
				REPRODUCIMOS SONIDOS EN UN ESCENARIO LIFO
			*/
			if ( parent == -1 ) {
					
				printf("killall me dio %d\n", system("killall play"));
				printf("forkin\n");
				pid = fork();
				
				if ( pid > 0 ) {

					parent = pid;
					char play[1024] = "play";
					for (int i = 0; i < times; i++) {
					
						strcat( play, " /home/pi/player/sounds/");
						strcat( play, d["TonoReproduccion"].GetString() );
						for (int j = 0; j < lude.tm_sec; j++) {
						
							strcat( play, " /home/pi/player/sounds/shutup.mp3");
						}
					}
					
					printf("%s\n", play);
					system(play);

					return 0;
				}
			} 			
        }
		//close( newsockfd );

        //--- if -2 sent by client, we can quit ---
        if ( data == -2 )
          break;
     }

return 0;
}
