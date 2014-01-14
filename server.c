#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "email.c"

pthread_t thread;
pthread_t thread1;
int clientFD;
int sizeOfFrom;
char msg[1024];
char menuString[] = "---------- MENU ----------\nSign up (s) \nLogin (l) \nExit (q)\n--------------------------";
char menuUser[] = "---------- MENU ----------\nVoteaza (v) \nAfisare top general (a)\nAfisare top dupa gen (g)\nComenteaza melodie (c)\nCauta melodie(s)\nLogout (l)\n--------------------------";
char menuAdmin[] ="---------- MENU ----------\nVoteaza (v) \nAfisare top general (a)\nAfisare top dupa gen (g)\nComenteaza melodie (c)\nCauta melodie(s)\nAdauga melodie(m)\nSterge melodie(d)\nRestrictioneaza \\ derestrictioneaza vot(r)\nLogout (l)\n--------------------------";

typedef struct thread_data{
        int fd;
        int *clientID;
        char nickname[250];
        char pass[250];
        int pos;
        char name[250];
        int terminated;
        int admin;
};

char* getFileContent(char* fileName){

  FILE * pFile;
		long lSize;
 		char * buffer;
  		size_t result;

  		pFile = fopen ( fileName , "rb" );
  		if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  		fseek (pFile , 0 , SEEK_END);
  		lSize = ftell (pFile);
  		rewind (pFile);

  		buffer = (char*) malloc (sizeof(char)*lSize);
  		if (buffer == NULL) {
  			fputs ("Memory error",stderr); exit (2);
  		}

  		result = fread (buffer,1,lSize,pFile);
  		if (result != lSize) {
  			fputs ("Reading error",stderr); exit (3);
  		}

  fclose (pFile);
return buffer;
}

void notifyUser(struct thread_data* data,char* msg){
        if(write(data->fd,msg,strlen(msg)) < 0) perror("err while responding to client\n");
}
int isAdmin(char* nickname){
        FILE* fin;

        fin = fopen("useri.txt","r");

        size_t len;
        char* line = NULL;

        int ok,k;

                while(getline(&line,&len,fin) != -1){
                                char* p;
                                p = strtok(line,"|");
                                k = 0;
                                ok = 0;
                                while(p){
                                        if(k == 0 && strcmp(nickname,p) == 0) ok = 1;
                                        ++k;
                                        if(k==3 && strcmp(p,"admin") == 0 && ok == 1) return 1;
                                        p = strtok(NULL,"|");
                                }
                }
return 0;
}
char* readFromClient(struct thread_data* data, char* msg1){
        char msg[1024];

        bzero(msg,1024);
        if (data->fd < 0){
                        perror("err from client\n");
        }
        
        int bytes = read(data->fd,msg,sizeof(msg));

            if (bytes <= 0){
                close(data->fd);
                *(data->clientID+data->pos) = -1;
                bzero(msg,1024);
                strcpy(msg,data->nickname);
                strcat(msg," has left the chat\n");        
                data->terminated = -1;
                return "bout";
           } else 
               if(strcmp(msg,"") == 0){
                        close(data->fd);
                                *(data->clientID+data->pos) = -1;
                                //the client has been disconected
                } else return msg; 
return "nothing";
}
char *replacestr(char *string, char *sub, char *replace){
             
              if(!string || !sub || !replace) return NULL;
              
              char *pos = string; 
              int found = 0;
              while((pos = strstr(pos, sub))){
                        pos += strlen(sub);
                        found++;
              }
              if(found == 0) return NULL;
              int size = ((strlen(string) - (strlen(sub) * found)) + (strlen(replace) * found)) + 1;
              char *result = (char*)malloc(size);
              pos = string; 
              char *pos1;
              while((pos1 = strstr(pos, sub))){
                        int len = (pos1 - pos);
                        strncat(result, pos, len);
                        strncat(result, replace, strlen(replace));
                        pos = (pos1 + strlen(sub));
              }
              if(pos != (string + strlen(string)))
                        strncat(result, pos, (string - pos));
return result;
}

int stergeMelodie(char* melodie){
              
                char *buffer = getFileContent("melodie.txt");
                char song[250];
        
                strcpy(song,"");
                strcat(song,melodie);
                strcat(song,"|");           
                                       
                char *a = strstr(buffer,song);
                char *b = strstr(a,"\n");

                if((a == NULL) || (b == NULL ) || (b < a)) return 0;
                memmove(a,b+1,strlen(b)+1);

                FILE* fout;
                	fout = fopen("melodie.txt","w");
                	fprintf(fout,buffer);
                fclose(fout);
                free(buffer);
return 1;
}
int restrictUser(char* userToRestrict){

        int restricted;
        char* buffer = getFileContent("useri.txt");
        char user[250];
        strcpy(user,""); 

        strcat(user,userToRestrict);
        strcat(user,"|");

        char* p = strstr(buffer,user);
        int begin = p-buffer+1;
        int i = begin;

        if(strstr(buffer,user) == NULL) return 1;
                if(buffer[i-2] == '\n' || i == 0){
                        if(isAdmin(userToRestrict)) return 0;
                                while(buffer[i+1] != '\n') ++i;
                                if (buffer[i] == '0') { buffer[i] = '1'; restricted = 2;}
                                else { buffer[i] = '0'; restricted = 1; }
        
                                FILE* fout;
                                	fout = fopen("useri.txt","w");
                                	fprintf(fout,buffer);
                                fclose(fout);
                                free(buffer);
                } else return 0; //userul nu exista sau fisierul este gol
return restricted; //returneaza 2 daca a fost ridicata restrictia si 1 daca a fost restrictionat
}
char* getComm(char* song1){

        FILE* fin;
        fin = fopen("comentarii.txt","r");

        size_t len;
        char* line = NULL;
        char user[250],song[250],comment[5000];
        char output1[5500];
        strcpy(output1,"");
                while(getline(&line,&len,fin) != -1){
                        char* p;
                        p = strtok(line,"|");
                        int pos = 0;
                        while(p){
                                switch(pos){
                                        case 0:
                                        strcpy(user,p);
                                        break;
                                        case 1:
                                        strcpy(song,p);
                                        break;
                                        case 2:
                                        strcpy(comment,p);
                                        break;
                                }
                                ++pos;
                                p = strtok(NULL,"|");
                        }
                        if(strcmp(song,song1) == 0){

                                strcat(output1,"User: ");
                                strcat(output1,user);
                                strcat(output1,"\n");
                                
                                strcat(output1,"Melodie: ");
                                strcat(output1,song);
                                strcat(output1,"\n");

                                strcat(output1,"Comentariu: ");
                                strcat(output1,comment);
                                strcat(output1,"\n");
                                
                        }
                }
                if(strcmp(output1,"") == 0) return "";
return replacestr(output1,"\\n","\n            "); //formatam output-ul ca sa deosebim comentariile mai bine
}
char* searchSong(char* song){
       
        FILE* fout;
        fout = fopen("melodie.txt","r");

        size_t len;
        char* line = NULL;
        char output1[1000];
        char* p;
        int l,j;
        int numberOfGendres;
        char* songAttributes[] = {"Nume","Descriere","Gen","Link","Vot"};
        int ok = 0;

        char* pch;
                while (getline(&line,&len,fout) != -1) {
                        pch = strchr(line,'|');
                        int pos = pch-line+1;
                        pos--;
                        if (strncmp(line,song,pos) == 0) {
                                ok = 1;
                                p = strtok(line,"|");
                                j = 0;
                                l = 0;
                                while (p){
                                        if (j > 2 && j < 2+numberOfGendres){ 
                                        	strcat(output1,songAttributes[l]);
                                        	strcat(output1,": ");
                                        	strcat(output1,p);
                                  			strcat(output1,"\n");
                                      } else {
                                                if (j != 2){
                                                	strcat(output1,songAttributes[l]);
                                                	strcat(output1,": ");
                                                	strcat(output1,p);
                                                	strcat(output1,"\n");
                                                	++l;
                                              } else numberOfGendres = parseInt(p);
                                        }
                                j++;
                                p = strtok(NULL,"|");
                                }
                        }
                }
        fclose(fout);
return output1;
}
int checkIfSongExists(char song[100]){
        
        char toSeek[110];
        strcpy(toSeek,song);
        strcat(toSeek,"|");
        int ok = 1;
        char *buffer = getFileContent("melodie.txt");

                       char *a = strstr(buffer,toSeek);
                       if(a == NULL) ok = 0;
return ok;
}
int addSong(struct thread_data* data){

        FILE* fout;

        fout = fopen("melodie.txt","a");

        int nrGen;
        char buffer[5000];
        strcpy(buffer,"");

                char input[500];
                int i,j;
                for(i = 1; i <= 5; ++i){
                        switch(i){
                                case 1:
                                		notifyUser(data,"Nume:");
                                        strcpy(input,readFromClient(data,input));
                                        input[strlen(input)-1] = '\0';


                                        if(checkIfSongExists(input) == 1) return -1;

                                                strcat(buffer,input);
                                                strcat(buffer,"|");
                                break;
                                case 2:
                                        notifyUser(data,"Descriere:");
                                        strcpy(input,readFromClient(data,input));
                                        input[strlen(input)-1] = '\0';
                                                strcat(buffer,input);
                                break;
                                case 3:
                                        notifyUser(data,"Numar genuri:");
                                        strcpy(input,readFromClient(data,input));
                                        input[strlen(input)-1] = '\0';
                                        if(!isdigit(input[0])) return 0;
                                        		strcat(buffer,"|");
                                        		strcat(buffer,input);
                                        		nrGen = atoi(input);

                                        		for (j = 1; j <= nrGen; ++j){
                                                
                                                		notifyUser(data,"Gen:");
                                                		strcpy(input,readFromClient(data,input));
                                                		input[strlen(input)-1] = '\0';
                                                		strcat(buffer,"|");
                                                		strcat(buffer,input);
                                        }
                                break;
                                case 4:
                                        notifyUser(data,"Link:");
                                        strcpy(input,readFromClient(data,input));
                                        input[strlen(input)-1] = '\0';
                                                strcat(buffer,"|");
                                                strcat(buffer,input);
                                                strcat(buffer,"|0\n");
                                break;
                        }
                }

        fprintf(fout,buffer);
        fclose(fout);

return 1;
}
int add_comm(char* user, char* nume, char* comm){

        if (!checkIfSongExists(nume)){
                return 0;
        }
        
        FILE* fout;
        fout = fopen("comentarii.txt","a");
		char buffer[1024];

        strcpy(buffer, user);
        strcat(buffer, "|");
        strcat(buffer, nume);
        strcat(buffer, "|");
        strcat(buffer, comm);
        strcat(buffer, "\n");
        
        if (fprintf(fout, "%s", buffer) == -1){
                perror("err while saving the song\n");
                return 0;
        }
        fclose(fout);

return 1;
}
void markVote(char* nickname,char* song){
  FILE* fout;

    fout = fopen("voturi.txt","a");
    char output1[500];

    strcpy(output1,nickname);
    strcat(output1,"|");
    strcat(output1,song);
  
   	if(fprintf(fout,"%s",output1) == -1) perror("err while writing to voturi.txt\n");

  fclose(fout);
}
int canIVote(char* nickname, char* melodie){
  
  FILE* fin;
  fin = fopen("voturi.txt","r");

  size_t len;
  char* line = NULL;

    while(getline(&line,&len,fin) != -1) {

      char* pch = strchr(line,'|');
      int position = pch - line + 1;
      char user[250],song[250];

      strncpy(user,line,position-1);
      strncpy(song,line+position,strlen(line));

      if (strcmp(user,nickname) == 0 && strcmp(song,melodie) == 0) return 0;
    }
return 1;
}
char* showTopByGendre(char* buffer1,char *gen){

       int vizitat[500];
       char output1[25000];
       strcpy(output1,"");
       char* songAttributes[] = {"Nume","Descriere","Gen","Link","Vot"};
       int i = 0,j,l;
       int max = -999;
       int linesCount = 0;

       		for(i = 1; i < 500; ++i) vizitat[i] = 0;
       
       		char *buffer = strdup(buffer1);
       		char* p = strtok(buffer,"\n");
       
       		while(p){
        		linesCount++;
        		p = strtok(NULL,"\n");
       		}

       		i = 0;
       		while(i < linesCount){
       			char *buffer = strdup(buffer1);
       			char* p = strtok(buffer,"\n");
       			int lineNo = 0;
       			int lineNoMax;
       			max = -999;

       			while(p){
        			lineNo++;
        			char* pch = strrchr(p,'|');
        			char* pch1 = strchr(p,'\n');

        			int start = (int) (pch - p);
        			start++;
        			int end = (int) (pch1 - p);

        			char number[5];
        			sprintf(number,"%.*s\n", end - start, &p[start]);
        			int n = parseInt(number);

					if(n > max && vizitat[lineNo] == 0){ 
							max = n; 
							lineNoMax = lineNo;
					}
        		p = strtok(NULL,"\n");
       }

       buffer = strdup(buffer1);
       p = strtok(buffer,"\n");
       int secondCount = 0;

       while(p){
        	secondCount ++;
        	if(secondCount == lineNoMax){
            	char *buf = strdup(p);
            	char* p1 = strtok(buf,"|");
                j = 0;
                int numberOfGendres;
                l = 0;

                char* pch = strstr(p,gen);
       
                if(pch != NULL){

                while (p1){
                        
                        if (j > 2 && j < 2+numberOfGendres) {
                                strcat(output1,songAttributes[l]);
                                strcat(output1,": ");
                                strcat(output1,p1);
                                strcat(output1,"\n");
                        }
                        else {
                        if (j != 2){
                                strcat(output1,songAttributes[l]);
                                strcat(output1,": ");
                                strcat(output1,p1);
                                strcat(output1,"\n");
                                ++l;
                        }
                        else numberOfGendres = parseInt(p1);
                        }
                      

                    
                        p1 = strtok(NULL,"|");
                        j++;
                 }
                
                strcat(output1,"---------------------\n");
              }
            }
         p = strtok(NULL,"\n");
         }

       vizitat[lineNoMax] = 1;
       ++i;
       }

return output1;
}
char* showTopFromVote(char *buffer1){
       

       int vizitat[500];
       char output1[25000];
       strcpy(output1,"");
       char* songAttributes[] = {"Nume","Descriere","Gen","Link","Vot"};
            

       int i = 0;
       int j;
       int l;

       for(i = 1; i < 500; ++i) vizitat[i] = 0;
       int max = -999;

       char *buffer = strdup(buffer1);
       char* p = strtok(buffer,"\n");
       int linesCount = 0;

       while(p){
        linesCount++;
        p = strtok(NULL,"\n");
       }

       i = 0;
       while(i < linesCount){
       		char *buffer = strdup(buffer1);
       		char* p = strtok(buffer,"\n");
       		int lineNo = 0;
       		int lineNoMax;
       		max = -999;

       			while(p){
        			lineNo++;
        			char* pch = strrchr(p,'|');
        			char* pch1 = strchr(p,'\n');

        			int start = (int) (pch - p);
        			start++;
        			int end = (int) (pch1 - p);

        			char number[5];
        			sprintf(number,"%.*s\n", end - start, &p[start]);
        			int n = parseInt(number);

        	 		if(n > max && vizitat[lineNo] == 0){ max = n; lineNoMax = lineNo;}
        		p = strtok(NULL,"\n");
       			}	

       		buffer = strdup(buffer1);
       		p = strtok(buffer,"\n");
       		int secondCount = 0;
       
       		while(p){
        		secondCount ++;
        			if(secondCount == lineNoMax){
            			char *buf = strdup(p);
            			char* p1 = strtok(buf,"|");

                		j = 0;
                		int numberOfGendres;
                		l = 0;
                		
                		while (p1){
                        
                        	if (j > 2 && j < 2+numberOfGendres) {
                                	strcat(output1,songAttributes[l]);
                                	strcat(output1,": ");
                                	strcat(output1,p1);
                                	strcat(output1,"\n");
                       		 }
                       		 else {
                        			if (j != 2){
                                		strcat(output1,songAttributes[l]);
                                		strcat(output1,": ");
                                		strcat(output1,p1);
                                		strcat(output1,"\n");
                                		++l;
                        			}
                        			else numberOfGendres = parseInt(p1);
                        	}
							p1 = strtok(NULL,"|");
                        	j++;
                		}
                	strcat(output1,"---------------------\n");
        			}
         		p = strtok(NULL,"\n");
         	}

       	vizitat[lineNoMax] = 1;
       ++i;
      }
return output1;
}
int is_restricted(char* user){
        
        FILE* fin;

        fin = fopen("useri.txt","r");

        size_t len;
        char* line = NULL;

        while(getline(&line,&len,fin) != -1){
                if (strncmp(user,line,strlen(user)) == 0){
                        char* p;
                        p = strrchr(line,'|');
                        if (line[p-line+1] != '1') return 0; 
                }
        }
        fclose(fin);
return 1;
}
int parseInt(char *ch){
	int temp=0,neg=0;
	
	while(*ch!=NULL){
    	
    	if(temp==0&&*ch=='-')neg=1;

    	if(*ch>='0'&&*ch<='9'){
        	if(temp==0){
             temp=*ch-'0';
        	}
       		 else {
             	temp*=10;
             	temp+=*ch-'0';
        	}
    	}
      ++ch;
	}
	if(neg==1)temp*=-1;

return temp;
}
int vote(char* song, int vot){

  	char* buffer = getFileContent("melodie.txt");
  	char *p;
  	p = strtok(buffer,"\n");
  	char array[200][500];

  	int k  = 0,i;
  	char aux[100];

  		while(p){
         	++k;
          	strcpy(array[k],p);
        	p = strtok(NULL,"\n");
  		}	
  
  	char* pch;
  	int found = 0;

  	for (i = 1; i <= k; ++i){

      		pch = strchr(array[i],'|');
      		int pos = pch-array[i]+1;
      		pos--;
          
          	if (strncmp(array[i],song,pos) == 0){
                  found = 1;
                  pch = strrchr(array[i],'|');
                  int last = pch-array[i]+1;
                  
                  char *x = array[i];
                  char existingVoteStr[5];
                  strcpy(existingVoteStr,x+last);

                          if (vot == -1){
                                  vot = parseInt(existingVoteStr);
                                  vot--;
                          }
                          else{
                                  vot = parseInt(existingVoteStr);
                                  vot++;
                          }
                  char vote[5];
                  sprintf(vote,"%d",vot);
                  strcpy(x+last,vote);
          	}

  	}

  	free (buffer);

  	FILE* fout;
  	fout = fopen("melodie.txt","w");

  	for (i = 1; i <= k; ++i){
                if (fprintf(fout,array[i]) == -1) perror("err while updating\n");
                if (fprintf(fout,"\n") == -1) perror("err while updating\n");
    }
 
    fclose(fout);
return found;
}

int login_user(char* nickname, char* pass){
	
	FILE* fin;
	fin = fopen("useri.txt","r");
	char* buffer = NULL;
    size_t len;

		while(getline(&buffer,&len,fin) != -1){
              char* p;
			  p = strtok(buffer,"|");
              int k = 0, ok = 0;
                    
                    while(p){
                          ++k;
                          switch(k){
                                case 1:{
                                	if (strcmp(p,nickname) == 0) ok = 1;
                                }
                                break;
                                case 2:{
                                   if (strcmp(p,pass) == 0 && ok) return 1;
                                }
                                break;
                           }
                        p = strtok(NULL, "|");
                    }
        }

return 0;
}
/*char* generateCode(){

        int g = rand()%4;

        FILE* fin;
        fin = fopen("codes.txt","r");

        size_t len;
        char* line = NULL;
        int k = 0;
        while(getline(&line,&len,fin) != -1){
                ++k;
                if(k == g) return line;
        }        
}
void emailCode(char* email){

        int p;
        p = fork();
        if(p == 0){
        int pid;
        int fd[2];

        if(pipe(fd) == -1) perror("err while piping\n");

        if((pid = fork()) == -1) perror("err while creating child\n");
        else 
                if (pid == 0){
                        dup2(fd[1],1);
                        execlp("echo","echo",generateCode(),NULL);

                }
                else if (pid != 0){
                        close(fd[1]);
                        dup2(fd[0],0);
                        execlp("mail","mail","-s","cod admin",email,NULL);
                }
        wait(NULL);
}
}
*/


int checkCode(char* code){
        
        FILE* fin;
        fin = fopen("codes.txt","r");

        size_t len;
        char* line = NULL;

        while(getline(&line,&len,fin) != -1){
                if(strcmp(line,code) == 0) return 1;
        } 

return 0;
}
int checkIfRegistered(char* nickname){

        FILE* fin;
        fin = fopen("useri.txt","r");

        size_t len;
        char* line = NULL;

        while(getline(&line,&len,fin) != -1){

                char* pch = strchr(line,'|');
                char pos = pch-line;
                char name[250];
                strncpy(name,line,pos);
                
                if(strcmp(name,nickname) == 0) return 0;
        }

return 1;
}
int register_user(char* nickname, char* parola, char* tip){
        
                FILE* fout;

                int output1Length = strlen(nickname)+strlen(parola)+strlen(tip)+7;
                char output1[output1Length];

                	strcpy(output1,nickname);
                	strcat(output1,"|");
                	strcat(output1,parola);
                	strcat(output1,"|");
                	strcat(output1,tip);
                	strcat(output1,"|1|1\n");

                fout = fopen("useri.txt","a");
                if (fwrite (output1 , sizeof(char), sizeof(output1), fout) <= 0){
                        perror("Err while writing to file \n");
                        return 0;
                }
                fclose(fout);

return 1;
}

void* solve_request(void *argv){

        pthread_t id = pthread_self();
        #define data ((struct thread_data*) argv)

        if (pthread_equal(id,thread)){

                data->terminated = 1;

                while(data->terminated != -1){

                int bytes = write(data->fd,menuString,sizeof(menuString));
                if(bytes <= 0) {perror("err while responding to client\n");}
                
                char* response = readFromClient(data,msg);
                
                if (response[1] == '\n')
                        switch(response[0]){
                                case 's':{
                                        //########################## SIGN UP ############################
                                        
                                        strcpy(msg,"Doriti un cont de admin? y/n\n");
                                        
                                        if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
                                        response = readFromClient(data,msg);

                                                switch(response[0]){
                                                        case 'y':{

                                                                notifyUser(data,"Aveti codul de inregistrare ca admin? y/n");
                                                                response = readFromClient(data,msg);

                                                                switch(response[0]){
                                                                        case 'y':{
                                                                        	after_inserting_code: //goto dupa ce trimite mail
                                                                        	notifyUser(data,"Va rugam introduceti codul primit prin email..");
                                                                        	char* code = readFromClient(data,msg);
                                                                        	

                                                                        	if(checkCode(code)){

                                                                                	char nick_name[250];
                                                                                	char pass[250];

                                                                                	int finish = 0;

                                                                                	while(finish == 0){
                                                                                		
                                                                                    if_nickname_not_available: 

                                                                                		notifyUser(data,"Nickname:");
                                                                                		strcpy(nick_name,readFromClient(data,msg));
                                                                                		nick_name[strlen(nick_name)-1] = '\0';

                                                                                		notifyUser(data,"Parola:");
                                                                                		strcpy(pass,readFromClient(data,msg));
                                                                                		pass[strlen(pass)-1] = '\0';

                                                                                		if (checkIfRegistered(nick_name) == 0) {
                                                                                        	notifyUser(data,"Nickname-ul nu este disponibil, va rugam incercati alt nickname \n");
                                                                                          goto if_nickname_not_available;
                                                                                		} else 
                                                                                	    	if(register_user(nick_name,pass,"admin") == 1){
                                                                                                finish = 1;
                                                                                                notifyUser(data,"Inregistrat cu succes ca admin");

                                                                                                strcpy(data->nickname,nick_name);
                                                												strcpy(data->pass,pass);

                                                												goto after_register_login_point;

                                                                        					}
                                                                                	}
                                                                        	} else notifyUser(data,"Codul este invalid \n");
                                                                        
                                                                    	}
                                                                        break;

                                                                        case 'n':{
                                                                        
                                                                        notifyUser(data,"Va rugam introduceti email-ul");

                                                                        char email[250];
                                                                        strcpy(email,readFromClient(data,msg));
                                                                        email[strlen(email)-1] = '\0';

                                                                        notifyUser(data,"se trimite codul..\n");
                                                                        emailCode(email);
                                                                        goto after_inserting_code;
                                                                        

                                                                        }
                                                                        break;
                                                                }
                                                        }
                                                        break;

                                                        case 'n':{

                                                        	char nick_name[250];
                                                        	char pass[250];

                                                            if_nickname_not_available_user:

                                                                notifyUser(data,"Nickname:");
                                                                strcpy(nick_name,readFromClient(data,msg));
                                                                nick_name[strlen(nick_name)-1] = '\0';

                                                                notifyUser(data,"Parola");
                                                                strcpy(pass,readFromClient(data,msg));
                                                                pass[strlen(pass)-1] = '\0';

                                                                if (checkIfRegistered(nick_name) == 0) {
                                                                	notifyUser(data,"Nickname-ul nu este disponibil, va rugam incercati alt nickname \n");
                                                                  goto if_nickname_not_available;
                                                                }
                                                                else

                                                                if(register_user(nick_name,pass,"user") == 1){
                                                                                printf("successfully registered\n");
                                                                                strcpy(data->nickname,nick_name);
                                                								strcpy(data->pass,pass);
                                                								goto if_nickname_not_available_user;
                                                                }
                                                                else printf("Eroare la logare\n");
                                                                
                                                        }
                                                        break;
                                                }
                                }
                                break;
                                
                                case 'l':{
                                        //########################## LOGIN ############################
                                        notifyUser(data,"User:");
                                                                        
                                        char user[250];
                                        strcpy(user,readFromClient(data,msg));

                                        notifyUser(data,"Parola:");
                                        char pass[250];
                                        strcpy(pass,readFromClient(data,msg));

                                        user[strlen(user) - 1] = '\0';
                                        pass[strlen(pass) - 1] = '\0';
                                        
                                        if(login_user(user,pass) == 1) {

                                        	strcpy(data->nickname,user);
                                            strcpy(data->pass,pass);

                                        	after_register_login_point:

                                                

                                                data->admin = isAdmin(data->nickname);

                                                notifyUser(data,"Ati fost logat cu succes!\n");

                                                char answer[250];
                                                while(answer[0] != 'l' && data->terminated != -1){
                                                if(data->admin == 0){
                                                        if(write(data->fd,menuUser,sizeof(menuUser)) < 0) perror("err while responding to client\n");
                                                }
                                                else 
                                                	if(write(data->fd,menuAdmin,sizeof(menuAdmin)) < 0) perror("err while responding to client\n");
                                                        
                                                strcpy(answer, readFromClient(data,msg));
                                                
                                                if(answer[1] == '\n')
                                                        switch (answer[0]){
                                                                case 'v':{
                                                                        //########################## VOTE ############################
                                                                        if (is_restricted(data->nickname) == 0){
                                                                                notifyUser(data,"Ne pare rau dar nu mai puteti vota..:(\n");
                                                                        } else {
                                                                                
                                                                                char numeMelodie[250];
                                                                                char votC[10];

                                                                                numeMelodie[strlen(numeMelodie) - 1] = '\0';
                                                                                votC[strlen(votC) - 1] = '\0';

                                                                                notifyUser(data,"Nume melodie:");
                                                                                strcpy(numeMelodie,readFromClient(data,msg));

                                                                                notifyUser(data,"Vot:");
                                                                                strcpy(votC,readFromClient(data,msg));
                                                                                
                                                                                int vot = parseInt(votC);

                                                                                if(canIVote(data->nickname,numeMelodie) == 0){
                                                                                        notifyUser(data,"Nu mai poti vota aceeasi melodie :( \n");
                                                                                } else 
                                                                                	if(vote(numeMelodie,vot) != 0){
                                                                                        notifyUser(data,"Ati votat cu succes!");
                                                                                        markVote(data->nickname,numeMelodie);
                                                                                	} else {
                                                                                        notifyUser(data,"Melodia nu a fost gasita in top :( \n");
                                                                                	}

                                                                        }
                                                                }
                                                                break;

                                                                case 'a':{
                                                                        //########################## SHOW TOP ############################
                                                                        char* buf = showTopFromVote(getFileContent("melodie.txt"));
                                                                        if(write(data->fd,buf,strlen(buf)) < 0) perror("err while responding to client\n");                   
                                                                }
                                                                break;

                                                                case 'g':{
                                                                        //########################## SHOW TOP FROM GENDRE ############################
                                                                        
                                                                        notifyUser(data,"Introdu gen:");
                                                                        char gen[250];
                                                                        strcpy(gen,readFromClient(data,msg));
                                                                        gen[strlen(gen)-1] = '\0';

                                                                        char buffer[25000];
                                                                        strcpy(buffer,showTopByGendre(getFileContent("melodie.txt"),gen));
                                                                        if (strcmp(buffer,"") == 0) 
                                                                                notifyUser(data,"Ne pare rau dar genul introdus nu se afla in baza de date\n");
                                                                        else notifyUser(data,buffer);
                                                                }
                                                                break;

                                                                case 'c':{
                                                                        //########################## COMMENT SONG ############################
                                                                       
                                                                        notifyUser(data,"Nume melodie:");
                                                                        char nume[250];
                                                                        strcpy(nume,readFromClient(data,msg));

                                                                        nume[strlen(nume)-1] = '\0';
                                                                        
                                                                        if(checkIfSongExists(nume) == 0) {
                                                                          notifyUser(data,"oops..melodia nu se afla in top :(\n");
                                                                          break;
                                                                        }
                                                                      
                                                                        strcpy(msg,"Comentariu:");
                                                                        if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
                                                                        
                                                                        char comment[5000];
                                                                        strcpy(comment,readFromClient(data,msg));

                                                                        strcpy(comment,replacestr(comment,"\n","\\n"));

                                                                        if(add_comm(data->nickname,nume,comment))
                                                                                notifyUser(data,"Va multumim pentru comentariu :)\n");
                                                                        else notifyUser(data,"oops..ceva nu a functionat la adaugarea comentariului :(\n");
                                                                        
                                                                }
                                                                break;

                                                                case 's':{
                                                                        //########################## SEARCH SONG ############################
                                                                       
                                                                        notifyUser(data,"Nume melodie:");
                                                                        char nume[250];
                                                                        strcpy(nume,readFromClient(data,msg));

                                                                        nume[strlen(nume)-1] = '\0';
                                                                        char *output1 = searchSong(nume);
                                                                        if(strcmp(output1,"") == 0) notifyUser(data,"Melodia nu a fost gasita");
                                                                        else {
                                                                                notifyUser(data,output1);
                                                                                if(strcmp(getComm(nume),"") == 0){
                                                                                	//do nothing
                                                                                }
                                                                                else{
                                                                                	notifyUser(data,"--------Comentarii---------\n");
                                                                                	notifyUser(data,getComm(nume));
                                                                                	notifyUser(data,"---------------------------\n");
                                                                                }
                                                                        }
                                                                }
                                                                break;

                                                                case 'r':{
                                                                        //########################## RESTRICT USER ############################
                                                                        
                                                                        if (!data->admin) notifyUser(data,"Nu poti executa comanda intrucat nu ai drept de admin ;)\n");
                                                                        else {
                                                                                notifyUser(data,"Introduceti nickname-ul pentru user-ul pe care vreti sa-l restrictionati...");
                                                                                char name[250];
                                                                                strcpy(name,readFromClient(data,msg));
                                                                                name[strlen(name)-1] = '\0';

                                                                                int restricted = restrictUser(name);

                                                                                if(restricted == 1) notifyUser(data,"Userul a fost restrictionat :) \n");
                                                                                else 
                                                                                	if (restricted == 2) notifyUser(data,"Userul a fost scos de sub restrictie :) \n");
                                                                                	else notifyUser(data,"User-ul nu a putut fi restrictionat...\n");

                                                                        }
                                                                }
                                                                break;

                                                                case 'd':{
                                                                        //########################## DELETE ############################
                                                                       
                                                                        if (!data->admin) notifyUser(data,"Nu poti executa comanda intrucat nu ai drept de admin ;)\n");
                                                                        else {
                                                                                notifyUser(data,"Introduceti numele melodiei pentru stergere...");
                                                                                char nume[250];
                                                                                strcpy(nume,readFromClient(data,msg));
                                                                                nume[strlen(nume)-1] = '\0';
                                                                                if(checkIfSongExists(nume)){
                                                                                    int _ok = stergeMelodie(nume);
                                                                                        if(_ok) {
                                                                                                notifyUser(data,"Melodia a fost stearsa cu succes :)\n");
                                                                                        }
                                                                                        else notifyUser(data,"crack..ceva a mers gresit...:(\n");

                                                                                }
                                                                                else notifyUser(data,"Melodia nu se afla in top\n");
    
                                                                        }
                                                                }
                                                                break;

                                                                case 'm':{
                                                                        //########################## ADD_SONG ############################
                                                                       
                                                                        int succes;

                                                                        if (!data->admin) notifyUser(data,"Nu poti executa comanda intrucat nu ai drept de admin ;)\n");
                                                                        else succes = addSong(data);
                                                                        

                                                                          switch (succes){
                                                                                case 1:
                                                                                notifyUser(data,"Cantecul a fost adaugat cu succes :)\n");
                                                                                break;

                                                                                case 0:
                                                                                notifyUser(data,"Crack..ceva a mers prost :(\n");
                                                                                break;

                                                                                case -1:
                                                                                notifyUser(data,"Cantecul exista deja in baza de date :)\n");
                                                                                break;
                                                                          }

                                                                        
                                                                }
                                                                break;

                                                                case 'l':{
                                                                        //logout
                                                                        strcpy(msg,"Ai fost delogat cu succes, ");
                                                                        strcat(msg,data->nickname);
                                                                        if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
                                                                                
                                                                        close(data->fd);
                                                                        *(data->clientID+data->pos) = -1;
                                                                        bzero(msg,1024);
                                                                        strcpy(msg,data->nickname);
                                                                        strcat(msg," has closed the connection\n");        
                                                                        printf("%s",msg);
                                                                        data->terminated = -1;
                                                                }
                                                                break;

                                                                default:{
                                                                        strcpy(msg,"Va rugam introduceti o comanda valida!");
                                                                        if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
                                                                }
                                                                break;
                                                        }

                                                }


                                        } else {
                                                strcpy(msg,"Logare esuata!");
                                                if(write(data->fd,msg,sizeof(msg)) < 0) perror("err while responding to client\n");
                                        }                       
                                }
                                break;

                                case 'q':{

                                        //########################## QUIT ############################

                                        notifyUser(data,"Have a nice one :)");
                                        close(data->fd);
                                        *(data->clientID+data->pos) = -1;
                                        bzero(msg,1024);
                                        strcpy(msg,data->nickname);
                                        strcat(msg," has closed the connection\n");        
                                        printf("%s",msg);
                                        data->terminated = -1;
                                }
                                break;
                        }
                }
        }
return NULL;
}
int main(int argc, char *argv[]){

        struct sockaddr_in server;
        struct sockaddr_in from;
        int sockFD;


        if ((sockFD = socket(AF_INET,SOCK_STREAM,0)) == -1){
                perror("err while socketing..\n");
                return 0;
        }

        int opt = 1;
        int port = atoi(argv[1]);

        setsockopt(sockFD,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //refolosim adresa 

        bzero(&server,sizeof(server));
        bzero(&from,sizeof(from));

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(port);

        if (bind(sockFD,(struct sockaddr*)&server,sizeof(struct sockaddr)) == -1){

                perror("err while binding\n");
                return 0;
        }

        if (listen(sockFD,5) < 0){
                perror("err while listening\n");
        }

        struct thread_data *data1;
        int clientID[100];
        int clientIterator = 0;

        while(1){
                
                sizeOfFrom = sizeof(from);
                printf("waiting at port %d\n", port);

                clientFD = accept(sockFD,(struct sockaddr*)&from,&sizeOfFrom);

                data1 = (struct thread_data*)malloc(sizeof(struct thread_data));

                data1->clientID = clientID;
                data1->fd = clientFD;
                clientID[++clientIterator] = clientFD;
                data1->pos = clientIterator;

                //pasam thread-urilor fiecare noua conexiune cu clientul

                int err = pthread_create(&thread,NULL,&solve_request,(void*) data1);

                if (err){
                        perror("err while creating thread\n");
                        return 0;
                }
                else printf("thread created successfully\n");
        }

return 0;
}