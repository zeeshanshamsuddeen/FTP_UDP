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
  
#define TRUE   1
#define FALSE  0
#define PORT 8888

struct sockaddr_in serv_addr, cli_addr;

struct in_addr inp;
int temp_sockfd;
struct sockaddr_in temp_serv_addr;
struct hostent *server;

void readsocket(int,char[]);
void writesocket(int,char[]);
void fn_USER(int);
void fn_PASS(int);
void fn_ls(int);
void fn_cd(int);
void fn_pwd(int);
void fn_cd_dot(int);
void fn_put(int);
void fn_get(int);
void fn_bye(int,struct sockaddr_in cli_addr,int);
void fn_invalid_command(int);
void fn_enter_password(int);

int auth_flag[30],check_flag=0,af[1];

int  n;
int temps=0;
int length,i,check=0; 
int iSetOption = 1;
int leng[10];
int srcFD,destFD,nbread,nbwrite;
int pos;

int filesize;

int opt = TRUE;
int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, valread , sd;
int max_sd;

char command[256],attribute[256],names[256],response[256];
char s_command[256];
char temp[256],empty[256]="";
char resp[256];
char temp1[256];
char user[256],pass[256],USER[256],PASS[256],username[256];
char message[256],enquiry[256];
char spath[30][256];



struct stat fileStat;
char *buff[1024];




int main() 
{
  int sockfd, newsockfd, portno, clilen,master_sockfd;

  for(i=0;i<max_clients;i++)
  {
    auth_flag[i]=0;
    spath[i][0]='/';
  }

  
  //set of socket descriptors
  fd_set readfds;

  for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }


  /* First call to socket() function */
  master_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) 
     {
        perror("ERROR opening socket");
        exit(1);
     }


  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = 5001;

  serv_addr.sin_family = AF_INET;                 // always fixed
  serv_addr.sin_addr.s_addr = INADDR_ANY;         // to get IP address of machine on which server is running
  serv_addr.sin_port = htons(portno);

  setsockopt(master_sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

  /* Now bind the host address using bind() call.*/
  if (bind(master_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
     {
        perror("ERROR on binding");
        exit(1);
     }
    
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
    
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
          
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_sockfd, &readfds)) 
        {
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
                    
                  check_flag=0;


                  bzero(message,256);
                  bzero(command,256);
                  bzero(attribute,256);
                  readsocket(newsockfd,message);
                  sscanf(message,"%s%[^\n]",command,attribute);
                  /*
                  attribute will have a white space at the beginning,
                  This white space is removed
                  */
                  strncpy(attribute,attribute+1,strlen(attribute));       



                  if(strcmp(command,"USER")==0&&auth_flag[pos]==0)
                    fn_USER(newsockfd);


                  if(strcmp(command,"PASS")==0)
                    fn_PASS(newsockfd);
                   

                  if(strcmp(command,"ls")==0&&strcmp(attribute,"")==0&&auth_flag[pos]==2)
                    fn_ls(newsockfd);
                  

                  if(strcmp(command,"cd")==0&&strcmp(attribute,"..")!=0&&strcmp(attribute,"")!=0&&auth_flag[pos]==2)
                    fn_cd(newsockfd);
                  

                  if(strcmp(command,"pwd")==0&&strcmp(attribute,"")==0&&auth_flag[pos]==2)
                    fn_pwd(newsockfd);
                 

                  if(strcmp(command,"cd")==0&&strcmp(attribute,"..")==0&&auth_flag[pos]==2)
                    fn_cd_dot(newsockfd);
                  

                  if(strcmp(command,"put")==0&&message[4]!=' '&&strlen(message)>4&&auth_flag[pos]==2)
                    fn_put(newsockfd);
                  

                  if(strcmp(command,"get")==0&&message[4]!=' '&&strlen(message)>4&&auth_flag[pos]==2)
                    fn_get(newsockfd);
                 

                  //Check if it was for closing
                  if (strcmp(command,"bye")==0)
                    fn_bye(newsockfd,cli_addr,clilen);
              

                  if(check_flag==0&&auth_flag[pos]!=1)
                    fn_invalid_command(newsockfd);


                  if(check_flag==0&&auth_flag[pos]==1)
                    fn_enter_password(newsockfd);

            }
         }   
       
       }
    return 0;
 }   


void readsocket(int newsockfd,char buffer[])
{
    bzero(buffer,256);
    char temp[1];
    bzero(temp,1);
    int i=0;
    do
    {
      read(newsockfd,temp,1);
      buffer[i]=temp[0];
      i++;     
    }while(temp[0]!='\n');

   
      
    read(newsockfd,temp,1);
   
      
    if(temp[0]!='\0')
      {
        printf("ERROR no /0 in read after %s",buffer);
        exit(1);
      }
    buffer[i-1]='\0';  

  
  /* At the end of this function, buffer will be a normal string with \0 at the end  */  
}


void writesocket(int newsockfd,char buffer[])
{
  int length=0; 
  length=strlen(buffer);

  buffer[length]='\n';
  buffer[length+1]='\0';
  length++;
  /*
      printf("%d\n\n",length);
      for(int i=0;i<=length;i++)
      if(buffer[i]=='\n')
        printf("new line in %dth position\n",i );
      for(int i=0;i<=length;i++)
      if(buffer[i]=='\0')
        printf("new zero in %dth position",i );
  */
  write(newsockfd,buffer,length+1);

}

void fn_USER(int newsockfd)
{

    check_flag=1;
    FILE *fp;
    fp=fopen("/home/zeeshan/FTP_UDP/obj/userlist.txt","r");

    for(;fscanf(fp,"%s",USER)!=EOF;)
     {
        if(strcmp(attribute,USER)==0)
           {
              
              auth_flag[pos]=1;
              fscanf(fp,"%s",PASS);
              bzero(response,256);
              sprintf(response,"%d%s",209,"Username valid");
          

              break;
              
           }
        else
          fscanf(fp,"%s",PASS);
          
     }     
    if(auth_flag[pos]==0)
      sprintf(response,"%d%s",202,"Username invalid");

    writesocket(newsockfd,response);
    fclose(fp);
    
}

void fn_PASS(int newsockfd)
 {
    check_flag=1;
    bzero(response,256);
    if(auth_flag[pos]==0)
      sprintf(response,"%d%s",211,"Enter username");
    else if(auth_flag[pos]==2)
      sprintf(response,"%d%s",212,"Already logged in");
    else if(strcmp(attribute,PASS)==0)
      {
        sprintf(response,"%d%s",219,USER);
        auth_flag[pos]=2;
        writesocket(newsockfd,response);
        sprintf(response,"%d%s",219,"Authentication successfull");

      } 
    else
      sprintf(response,"%d%s",214,"Invalid password");

    writesocket(newsockfd,response);
}


void fn_ls(int newsockfd)
{
             
    check_flag=1;
    char buff[BUFSIZ];
    bzero(s_command,256);
    strcpy(s_command,"ls /home/zeeshan/FTP_UDP/obj/server");
    strcat(s_command,spath[pos]);

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
    check_flag=1;
    bzero(s_command,256);
    strcpy(s_command,"/home/zeeshan/FTP_UDP/obj/server");
    strcat(s_command,spath[pos]);
    strcat(s_command,attribute);
    //check whether the FOLDER(path is in s_command) exists in the server directory
    if(stat(s_command,&fileStat)==0 && S_ISDIR(fileStat.st_mode))  
      {
        strcat(spath[pos],attribute);
        strcat(spath[pos],"/");
        sprintf(response,"%d%s",239,"Server directory changed");

      }  
    else
      sprintf(response,"%d%s",231,"No such file exists");
    writesocket(newsockfd,response);

}

void fn_pwd(int newsockfd)
{
    check_flag=1;
    bzero(response,256);
    sprintf(response,"%d%s",249,spath[pos]);
    writesocket(newsockfd,response);
}

void fn_cd_dot(int newsockfd)
{
    /*  From spath, all the charecters after the rightmost '/' is cleared   */  
    check_flag=1;
    if(strcmp(spath[pos],"/")!=0)
    {
      length=0;
      length=strlen(spath[pos]);
      for(i=length-2;i>=0;i--)
      {
        if(spath[pos][i]=='/')
          break;
      }
      strcpy(temp,spath[pos]);
      bzero(spath[pos],256);
      strncpy(spath[pos],temp,i+1);
    } 
    sprintf(response,"%d%s",259,"Directory successfully changed");
    writesocket(newsockfd,response);
}

void fn_put(int newsockfd)
{
    check_flag=1;

    readsocket(newsockfd,message);
    sscanf(message,"%d",&filesize);

    strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server");
    strcat(temp,spath[pos]);
    strcat(temp,attribute);


    destFD = open(temp,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    for(i=0;i<filesize;i++)
    {
      read(newsockfd,buff,1);
      write(destFD,buff,1);     //write data to destination file
    }
    close(destFD);
    sprintf(response,"%d%s",329,"File transfer completed");
    writesocket(newsockfd,response);

}

void fn_get(int newsockfd)
 {

    check_flag=1;
    strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server");
    strcat(temp,spath[pos]);
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

      //size of file is calculated in filesize and sent to the server
      filesize = fileStat.st_size;
      sprintf(message,"%d%s",filesize,"Valid filename");
      writesocket(newsockfd,message);

        bzero(message,256);
        int temp_port;
        readsocket(newsockfd,message);
        sscanf(message,"%d",&temp_port);
        printf("%d\n",temp_port );



        /* Create a socket point */
            temp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (temp_sockfd < 0)
            {
              perror("ERROR opening socket");
              exit(1);
            }
            

            setsockopt(temp_sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

            server = gethostbyname("127.0.0.1");      // returns pointer to corresponding host

            if (server == NULL) 
              {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
              }


            /*
            if (inet_aton("127.0.0.1",&inp) ==0)    // returns pointer to corresponding host
            {
              fprintf(stderr,"ERROR, no such host\n");
              exit(0);
            }
            */

            bzero((char *) &temp_serv_addr, sizeof(temp_serv_addr));
            temp_serv_addr.sin_family = AF_INET;
            temp_serv_addr.sin_port = htons(temp_port);
            bcopy((char *)&inp , (char *)&temp_serv_addr.sin_addr.s_addr, sizeof(inp));

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
    }  
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
    auth_flag[pos]=0;
    bzero(spath[pos],256);
    spath[pos][0]='/';
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
