#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <netinet/in.h>
#include <arpa/inet.h>
  
#define TRUE   1
#define FALSE  0
#define PORT 8888

struct sockaddr_in serv_addr, cli_addr;

struct in_addr inp;
int temp_sockfd;
struct sockaddr_in temp_serv_addr;
struct hostent *server;

void readsocket(int);
void writesocket(int,char[]);
void fn_USER(int);
void fn_PASS(int);
void fn_ls(int);
void fn_cd(int);
void fn_pwd(int);
void fn_cd_dot(int);
void fn_put(int);
void fn_get(int);
void fn_get_data(int);
void fn_bye(int,struct sockaddr_in cli_addr,int);
void fn_invalid_command(int);
void fn_enter_password(int);

int af[1];

int  n;
int temps=0;
int length,i,check=0; 
int iSetOption = 1;
int leng[10];
int srcFD,destFD,nbread,nbwrite;
int pos;


int opt = TRUE;
int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, valread , sd;
int max_sd;
int filesize;

char names[256],response[256];
char s_command[256];
char temp[256],empty[256]="";
char resp[256];
char temp1[256];
char user[256],pass[256],USER[256],PASS[256],username[256];
char enquiry[256];
char command[256],attribute[256],message[256];


struct stat fileStat;
char *buff[1024];

struct Clients{
  int command_flag; //check whether the whole input from client has reached server .ie. is set when command reaches server
  int check_flag;   //is set if command is invalid or passwod is yet to be entered
  int auth_flag;    //to check whether user is authenticated  
  int data_flag_put;//indicates next msg is filesize
  int data_flag_get;//indicates next msg is PORT number
  int filesize;     //size of file in put data transfer
  int buffer_size;  //size of current buffer
  int port_number;  //port number of CLIENT in get data transfer
  char USER[256],PASS[256]; //username and password
  char filename[256];//file name used in put data transfer
  
  char buffer[256]; //buffer used to store incoming command pieces
  char spath[256];  //stores current server path
};
struct Clients client[50]={0};

struct sockaddr *addr;
socklen_t *addrlen_;

/*
GetPeerName() is used to determine who is the client connected to your socket.

GetSockName() is used to determine your own socket properties like which port what IP addresses is the socket bound to 
and the your own machine's port number.

In this context, sockets are assumed to be associated with a specific socket address, namely the IP address 
and a port number for the local node, and there is a corresponding socket address at the foreign node (other node), 
which itself has an associated socket, used by the foreign process. Associating a socket with a socket address is called binding.

Note that while a local process can communicate with a foreign process by sending or receiving data to or from 
a foreign socket address, it does not have access to the foreign socket itself, nor can it use the foreign socket descriptor, 
as these are both internal to the foreign node. For example, in a connection between 10.20.30.40:4444 and 
50.60.70.80:8888 (local IP address:local port, foreign IP address:foreign port), there will also be an associated socket at each end,
corresponding to the internal representation of the connection by the protocol stack on that node, which are 
referred to locally by numerical socket descriptors, say 317 at one side and 922 at the other. A process on node 10.20.30.40
can request to communicate with node 50.60.70.80 on port 8888 (request that the protocol stack create a socket to communicate 
with that destination), and once it has created a socket and received a socket descriptor (317), it can communicate via this
socket by using the descriptor (317). The protocol stack will then forward data to and from node 50.60.70.80 on port 8888.
However, a process on node 10.20.30.40 cannot request to communicate with "socket 922"
or "socket 922 on node 50.60.70.80": these are meaningless numbers to the protocol stack on node 10.20.30.40.
*/
void get_sock_details_sockname(int sockfd){
  struct sockaddr_in addr_ = {0};
  socklen_t addr_size_ = sizeof(struct sockaddr_in);
  int res = getsockname(sockfd, (struct sockaddr *)&addr_, &addr_size_);
  char clientip[20]= {0};
  strcpy(clientip, inet_ntoa(addr_.sin_addr));
  printf("sockname : %d : %s \n",ntohs(addr_.sin_port), clientip );
}

void get_sock_details_peername(int sockfd){
  struct sockaddr_in addr_ = {0};
  socklen_t addr_size = sizeof(struct sockaddr_in);
  int res = getpeername(sockfd, (struct sockaddr *)&addr_, &addr_size);
  char clientip[20]= {0};
  strcpy(clientip, inet_ntoa(addr_.sin_addr));
  printf("peername : %d : %s \n",ntohs(addr_.sin_port), clientip );

}

/* function used to return foreign ip address */
char* get_sock_details_peername_ip(int sockfd, char* ip){
  struct sockaddr_in addr_ = {0};
  socklen_t addr_size_ = sizeof(struct sockaddr_in);
  int res = getpeername(sockfd, (struct sockaddr *)&addr_, &addr_size_);
  char clientip[20]= {0};
  strcpy(clientip, inet_ntoa(addr_.sin_addr));
  strcpy(ip,clientip);
  return ip;
}


int main() 
{
  int sockfd, newsockfd, portno, clilen,master_sockfd;

  for(i=0;i<max_clients;i++)
  {
    client[i].auth_flag=0;
    client[i].spath[0]='/';
  }

  
  //set of socket descriptors
  fd_set readfds;

  for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }


  /* First call to socket() function 
      socket() fn creates a socket and the port and ip part is left without configuration
  */
  master_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) 
     {
        perror("ERROR opening socket");
        exit(1);
     }


  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = 5002;

  serv_addr.sin_family = AF_INET;                 // always fixed
  serv_addr.sin_addr.s_addr = INADDR_ANY;         // INNADDR_ANY specifies 0(localhost) and has some other advantages
  serv_addr.sin_port = htons(portno);


  setsockopt(master_sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

  /* Now bind the host address using bind() call.
      BINDING the created socket with the specified IP adress and PORT number
      In this case, the present machine IP and port specified is used
  */
  if (bind(master_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
     {
        perror("ERROR on binding");
        exit(1);
     }


  
    get_sock_details_sockname(master_sockfd);
    get_sock_details_peername(master_sockfd);

  /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
  */

  listen(master_sockfd,5);
  clilen = sizeof(cli_addr);


  while(TRUE) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
  
        //add master socket to set
        FD_SET(master_sockfd, &readfds);
        max_sd = master_sockfd;
         
        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) 
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);
             
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
  
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        printf("activity \n");

        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
          
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_sockfd, &readfds)) 
        {
            /* */
            if ((newsockfd = accept(master_sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d and port is %d \n" , newsockfd ,ntohs(cli_addr.sin_port));
              
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) 
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = newsockfd;
                    printf("Adding to list of sockets as %d\n" , i);
                    break;
                }
            }
        }
          
        //else its some IO operation on some other socket :)
        for (pos = 0; pos < max_clients; pos++) 
        {
            newsockfd = client_socket[pos];
              
            if (FD_ISSET( newsockfd , &readfds)) 
            {

                
                 /*
                    auth_flag[pos] is used to indicate whether the user is authenticated or nor.
                    auth_flag[pos] = 0 : not authenticated
                    auth_flag[pos] = 1 : username verified, password required
                    auth_flag[pos] = 2 : user authenticated 
                  */
                    
                  //check flag is reset so that if the command is dirty, the corresponding fn can specify INVALID  
                  client[pos].check_flag=0;
                  client[pos].command_flag=0;

                  
             
                  readsocket(newsockfd);
                  printf("buffer is %s\n",client[pos].buffer );

                  if(client[pos].command_flag == 1){

                    bzero(command,256);
                    bzero(attribute,256);

                    sscanf(client[pos].buffer,"%s%[^\n]",command,attribute);

                    printf("command : %s, attribute: %s\n",command,attribute);
                    /*
                    attribute will have a white space at the beginning,
                    This white space is removed
                    */
                    strncpy(attribute,attribute+1,strlen(attribute));       

                    if(strcmp(command,"USER")==0 && client[pos].auth_flag==0)
                      fn_USER(newsockfd);

                    else if(strcmp(command,"PASS")==0)
                      fn_PASS(newsockfd);
                    
                    else if(strcmp(command,"ls")==0&&strcmp(attribute,"")==0&& client[pos].auth_flag == 2)
                    fn_ls(newsockfd);
                  

                    else if(strcmp(command,"cd")==0&&strcmp(attribute,"..")!=0&& strcmp(attribute,"")!=0&& client[pos].auth_flag ==2)
                      fn_cd(newsockfd);
                    

                    else if(strcmp(command,"pwd")==0&&strcmp(attribute,"")==0&& client[pos].auth_flag ==2)
                      fn_pwd(newsockfd);
                   

                    else if(strcmp(command,"cd")==0&&strcmp(attribute,"..")==0&& client[pos].auth_flag ==2)
                      fn_cd_dot(newsockfd);

                    
                    else if(strcmp(command,"put")==0&& client[pos].buffer[4]!=' '&&strlen(client[pos].buffer)>4&& client[pos].auth_flag==2)
                      {
                        strcpy(client[pos].filename,attribute);
                        client[pos].data_flag_put++;
                      }
                    //when filesize is now in the buffer  
                    else if(client[pos].data_flag_put == 1)
                      fn_put(newsockfd);


                    else if(strcmp(command,"get")==0&& client[pos].buffer[4]!=' '&&strlen(client[pos].buffer)>4&& client[pos].auth_flag==2)
                     fn_get(newsockfd);

                    //when PORT number is in the buffer
                    else if(client[pos].data_flag_get == 1){
                      client[pos].port_number = atoi(attribute);
                      fn_get_data(newsockfd);
                    }
                    
    
                    //Check if it was for closing
                    else if (strcmp(command,"bye")==0)
                      fn_bye(newsockfd,cli_addr,clilen);
                

                    else if(client[pos].check_flag==0 && client[pos].auth_flag!=1)
                      fn_invalid_command(newsockfd);


                    else if(client[pos].check_flag==0 && client[pos].auth_flag==1)
                      fn_enter_password(newsockfd);

                    //buffer is cleared for next command
                    bzero(client[pos].buffer,256);
                    client[pos].buffer_size=0;
                    


                  }
         
             }     

         }   
       
       }
    return 0;
 }   


void readsocket(int newsockfd)
{
    char temp[1];
    bzero(temp,1);
   
    read(newsockfd,temp,1);
    printf("received packet is %s\n",temp );

    //checks whether the obtained string has pattern \n\0, if So, \n is replaced by \0
    if(temp[0] == '\0' &&  client[pos].buffer[client[pos].buffer_size-1] == '\n'){
          client[pos].command_flag = 1;
          client[pos].buffer_size--;
          client[pos].buffer[client[pos].buffer_size] = '\0';
          printf("buffer size %d\n",client[pos].buffer_size);
    }
    else
      strcat(client[pos].buffer,temp);
    client[pos].buffer_size++;

     
}


void writesocket(int newsockfd,char buffer[])
{
  int length=0; 
  length=strlen(buffer);

  buffer[length]='\n';
  buffer[length+1]='\0';
  length++;
  
  write(newsockfd,buffer,length+1);

}

void fn_USER(int newsockfd)
{

    client[pos].check_flag=1;
    FILE *fp;
    fp=fopen("/home/zeeshan/File-Transfer-Protocol/obj/userlist.txt","r");

    for(;fscanf(fp,"%s",client[pos].USER)!=EOF;)
     {
        if(strcmp(attribute,client[pos].USER)==0)
           {
              
              client[pos].auth_flag=1;
              fscanf(fp,"%s",client[pos].PASS);
              bzero(response,256);
              sprintf(response,"%d%s",209,"Username valid");
  
              break;
              
           }
        else
          fscanf(fp,"%s",client[pos].PASS);
          
     }     
    if(client[pos].auth_flag==0)
      sprintf(response,"%d%s",202,"Username invalid");

    writesocket(newsockfd,response);
    fclose(fp);

  
    
}

void fn_PASS(int newsockfd)
 {
    client[pos].check_flag=1;
    bzero(response,256);
    if(client[pos].auth_flag==0)
      sprintf(response,"%d%s",211,"Enter username");
    else if(client[pos].auth_flag==2)
      sprintf(response,"%d%s",212,"Already logged in");
    else if(strcmp(attribute,client[pos].PASS)==0)
      {
        sprintf(response,"%d%s",219,client[pos].USER);
        client[pos].auth_flag=2;
        writesocket(newsockfd,response);
        sprintf(response,"%d%s",219,"Authentication successfull");

      } 
    else
      sprintf(response,"%d%s",214,"Invalid password");

    writesocket(newsockfd,response);

    
}



void fn_ls(int newsockfd)
{
             
    client[pos].check_flag=1;
    char buff[BUFSIZ];
    bzero(s_command,256);
    strcpy(s_command,"ls /home/zeeshan/File-Transfer-Protocol/obj/server");
    strcat(s_command,client[pos].spath);

    FILE *fp = popen(s_command,"r");
    //check whether the filename exists in the server directory
    while ( fgets( buff, BUFSIZ, fp ) != NULL ) 
    {
      /*  Each file/folder name is retrieved and is sent to client   */
      bzero(response,256);
      length=0;
      length=strlen(buff);
      buff[length-1]='\0';
      sprintf(response,"%d%s",221,buff);
      writesocket(newsockfd,response);
    }
    pclose(fp);

    sprintf(response,"%d%s",229,"List of files completed");
    writesocket(newsockfd,response);
}


void fn_cd(int newsockfd)
{
    client[pos].check_flag=1;
    bzero(s_command,256);
    strcpy(s_command,"/home/zeeshan/File-Transfer-Protocol/obj/server");
    strcat(s_command,client[pos].spath);
    strcat(s_command,attribute);
    //check whether the FOLDER(path is in s_command) exists in the server directory
    if(stat(s_command,&fileStat)==0 && S_ISDIR(fileStat.st_mode))  
      {
        strcat(client[pos].spath,attribute);
        strcat(client[pos].spath,"/");
        sprintf(response,"%d%s",239,"Server directory changed");

      }  
    else
      sprintf(response,"%d%s",231,"No such file exists");
    writesocket(newsockfd,response);

}

void fn_pwd(int newsockfd)
{
    client[pos].check_flag=1;
    bzero(response,256);
    sprintf(response,"%d%s",249,client[pos].spath);
    writesocket(newsockfd,response);
}

void fn_cd_dot(int newsockfd)
{
    /*  From spath, all the charecters after the rightmost '/' is cleared   */  
    client[pos].check_flag=1;
    if(strcmp(client[pos].spath,"/")!=0)
    {
      length=0;
      length=strlen(client[pos].spath);
      for(i=length-2;i>=0;i--)
      {
        if(client[pos].spath[i]=='/')
          break;
      }
      strcpy(temp,client[pos].spath);
      bzero(client[pos].spath,256);
      strncpy(client[pos].spath,temp,i+1);
    } 
    sprintf(response,"%d%s",259,"Directory successfully changed");
    writesocket(newsockfd,response);
}


void fn_put(int newsockfd)
{
    client[pos].check_flag=1;

  
    sscanf(client[pos].buffer,"%d",&client[pos].filesize);
    printf("file size is %d\n",client[pos].filesize );

    strcpy(temp,"/home/zeeshan/File-Transfer-Protocol/obj/server");
    strcat(temp,client[pos].spath);
    strcat(temp,client[pos].filename);


    destFD = open(temp,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    for(i=0;i<client[pos].filesize;i++)
    {
      read(newsockfd,buff,1);
      write(destFD,buff,1);     //write data to destination file
    }
    close(destFD);
    sprintf(response,"%d%s",329,"File transfer completed");
    writesocket(newsockfd,response);

    //data_flag_put is reset indicating data transfer completion
    client[pos].data_flag_put=0;
    //filename is also cleared for next put
    bzero(client[pos].filename,256);
    client[pos].filesize=0;


}



void fn_get(int newsockfd)
 {

    client[pos].check_flag=1;
    strcpy(temp,"/home/zeeshan/File-Transfer-Protocol/obj/server");
    strcat(temp,client[pos].spath);
    strcat(temp,attribute);

    srcFD = open(temp,O_RDONLY);
    if(srcFD==-1)
    {
      sprintf(response,"%d%s",331,"Invalid filename");
      writesocket(newsockfd,response);
    }
    else
    {
      stat(temp, &fileStat);

      //size of file is calculated in filesize and sent to the server. This is the filesize of file in server system
      // hence client structre is not used
      filesize = fileStat.st_size;
      sprintf(response,"%d%s",filesize,"Valid filename");
      writesocket(newsockfd,response);

      // data_flag_get is set to indicate that filename is valid and the next message will be the PORT number
      client[pos].data_flag_get++;

    }  
}


void fn_get_data(int newsockfd){


        /* Create a socket point */
        temp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (temp_sockfd < 0)
        {
          perror("ERROR opening socket");
          exit(1);
        }
        

        setsockopt(temp_sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

        char temp_string[25]={};
        /* The ip address of the CLIENT is obtained using newsockfd and connection 
          is established to that IP using the port obtained from CLIENT
         */
        server = gethostbyname(get_sock_details_peername_ip(newsockfd,temp_string)); // returns pointer to corresponding host

        if (server == NULL) 
          {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
          }



        bzero((char *) &temp_serv_addr, sizeof(temp_serv_addr));
        temp_serv_addr.sin_family = AF_INET;
        temp_serv_addr.sin_port = htons(client[pos].port_number);
        printf("%d\n",temp_serv_addr.sin_port );

        bcopy((char *)server->h_addr, (char *)&temp_serv_addr.sin_addr.s_addr, server->h_length);

        //bcopy((char *)&inp , (char *)&temp_serv_addr.sin_addr.s_addr, sizeof(inp));

        /* Now connect to the server */
        if (connect(temp_sockfd, (struct sockaddr*)&temp_serv_addr, sizeof(temp_serv_addr)) < 0) 
        {
          perror("ERROR connecting");
          exit(1);
        }

         printf("socket number is %d\n",temp_sockfd );

      //file is sent byte by byte
      for(i=0;i<filesize;i++)
      {
        read(srcFD,buff,1);
        write(temp_sockfd,buff,1);
      }

      close(srcFD);
      close(temp_sockfd);

      //data_flag is reset so that next get can take place
     client[pos].data_flag_get=0;
}



void fn_bye(int newsockfd,struct sockaddr_in cli_addr,int clilen)
{
    //Somebody disconnected , get his details and print
    getpeername(newsockfd , (struct sockaddr*)&cli_addr , (socklen_t*)&clilen);
    printf("Host disconnected  port %d \n" , ntohs(cli_addr.sin_port));
    sprintf(response,"%d%s",0,"Exit");
    writesocket(newsockfd,response);
      
    //Close the socket and mark as 0 in list for reuse
    close( newsockfd );
    client_socket[pos] = 0;
    client[pos].auth_flag=0;
    bzero(client[pos].spath,256);
    client[pos].spath[0]='/';
}

void fn_invalid_command(int newsockfd)
{
    sprintf(response,"%d%s",999,"Invalid command");
    writesocket(newsockfd,response);
}

void fn_enter_password(int newsockfd)
{
    sprintf(response,"%d%s",999,"Enter password to proceed");
    writesocket(newsockfd,response);
}
