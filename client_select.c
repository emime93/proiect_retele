#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>



pthread_t thread;

char msg[1024];
extern int errno;


int main(int argc, char *argv[]){

        struct sockaddr_in server;
        int sockFD;
        int ok = 1;
        fd_set readfds;
        fd_set actfds;
        int nfds;
        int comment;


        if ((sockFD = socket(AF_INET,SOCK_STREAM,0)) == -1){
                perror("err while socketing\n");
                return 0;
        }

        bzero(&server,sizeof(server));

        int port = atoi(argv[2]);

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(argv[1]);
        server.sin_port = htons(port);


        if (connect(sockFD,(struct sockaddr*) &server,sizeof(server)) == -1){
                perror("err while connecting\n");
                return 0;
        }
        
        char msg[1024];
        bzero(msg,1024);

        FD_ZERO (&actfds);
      	FD_SET (sockFD, &actfds);
     	FD_SET (0, &actfds);
        nfds = sockFD;
        
        while (strcmp(msg,"quit") != 0){
			 
			 bcopy ((char *) &actfds, (char*) &readfds, sizeof(readfds));
      		 
      		 if( select( nfds+1, &readfds, NULL, NULL, NULL) <0 ){
          		perror("eroare la select");
          		return errno;
       		}

            int k = 0;
            bzero(msg,1024);
        

        	if( FD_ISSET(sockFD, &readfds) ){

                if (read(sockFD,msg,1024) <= 0){
                        perror("err while reading from server\n");
                        return 0;
                }
                
                if(strncmp(msg,"---------- MENU ----------",26) == 0 ||
                    strncmp(msg,"--------Comentarii---------",27) == 0 ||
                    strncmp(msg,"---------------------------",27) == 0 ||
                    strncmp(msg,"Nickname",8) == 0
                    ) {} 
                else system("clear");

                printf("\n%s\n",msg);
                if(strncmp(msg,"Ai fost delogat cu succes",25) == 0) strcpy(msg,"quit");
                if(strncmp(msg,"Have a nice one :)",18) == 0) strcpy(msg,"quit");

                if(strncmp(msg,"Comentariu:",11) == 0) comment = 1;
                else comment = 0;
        	}

        	if( FD_ISSET(0, &readfds) ){  

        		if(strcmp(msg,"quit") != 0){
                		if(comment == 1){
                        	int k = 0;
                        	char msg1[10000];
                        	strcpy(msg1,"");
                        	int terminated = 0;
                        	
                        	while(!terminated){

                                fgets(msg,1024,stdin);
                                strcat(msg1,msg);
                                
                                if(strcmp(msg,"\n") == 0) ++k;
                                if(strcmp(msg,"\n") == 0 && k == 1) terminated = 1;
                                if(strcmp(msg,"\n") != 0 && k == 1) k = 0;

                        	}
                        
                        	msg1[strlen(msg1)-2] = '\0';
                        
                        if (write(sockFD,msg1,strlen(msg1)) <= 0){
                        	perror("err while writing to server\n");
                        	return 0;
                        }
                } else {
                    	fgets(msg,1024,stdin);
                		if (write(sockFD,msg,sizeof(msg)) <= 0){
               				perror("err while writing to server\n");
                			return 0;
        		  		}
        			}

        		}

        	}
        
    	}


return 0;
}