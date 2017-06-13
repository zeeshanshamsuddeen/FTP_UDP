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

void readsocket(int newsockfd,char buffer[])
{
    bzero(buffer,256);
    char temp[1];
    bzero(temp,1);
    int i=0;
    //printf("reading\n");
    do
    {
      read(newsockfd,temp,1);
      buffer[i]=temp[0];
      i++;
      //printf("%c",temp[0] );
     
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
  //printf("the length is %d\n string is %s\n",length,buffer);
//    buffer[length-1]='\n';


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

int main() 
{
  int sockfd, newsockfd, portno, clilen;
  struct sockaddr_in serv_addr, cli_addr;
//  struct dirent  *dp;
//  struct stat statbuf;
  struct stat fileStat;

  char command[256],attribute[256],names[256],buffer[256],response[256];
  char s_command[256];
  char temp[256],empty[256]="";
  char spath[256]="/";
  char resp[256];
  char temp1[256];
  char user[256],pass[256],USER[256],PASS[256],username[256];
  char message[256],enquiry[256];

  int  n;
  int temps=0;
  int length,i,check=0; 
  int iSetOption = 1;
  int leng[10],pause[10];
  int srcFD,destFD,nbread,nbwrite;

  int auth_flag=0,check_flag=0,af[1];
  int filesize;

  char *buff[1024];


  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

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

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

  /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
     {
        perror("ERROR on binding");
        exit(1);
     }
    
  /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
  */

  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  /* Accept actual connection from the client */
  newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

  if (newsockfd < 0) 
     {
        perror("ERROR on accept");
        exit(1);
     }

do
{
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



      if(strcmp(command,"USER")==0&&auth_flag==0)
      {

        check_flag=1;
        FILE *fp;
        fp=fopen("/home/zeeshan/FTP_UDP/obj/userlist.txt","r");

        for(;fscanf(fp,"%s",USER)!=EOF;)
         {
            if(strcmp(attribute,USER)==0)
               {
                  
                  auth_flag=1;
                  fscanf(fp,"%s",PASS);
                  bzero(response,256);
                  sprintf(response,"%d%s",209,"Username valid");
              

                  break;
                  
               }
            else
              fscanf(fp,"%s",PASS);
              
         }     
        if(auth_flag==0)
          sprintf(response,"%d%s",202,"Username invalid");
 
        writesocket(newsockfd,response);
        fclose(fp);
      }


      if(strcmp(command,"PASS")==0)
        {
          check_flag=1;
          bzero(response,256);
          if(auth_flag==0)
            sprintf(response,"%d%s",211,"Enter username");
          else if(auth_flag==2)
            sprintf(response,"%d%s",212,"Already logged in");
          else if(strcmp(attribute,PASS)==0)
            {
              sprintf(response,"%d%s",219,"Authentication successfull");
              auth_flag=2;
            } 
          else
            sprintf(response,"%d%s",214,"Invalid password");

          writesocket(newsockfd,response);
        }

      if(strcmp(command,"ls")==0&&strcmp(attribute,"")==0&&auth_flag==2)
      {
 
          check_flag=1;
          char buff[BUFSIZ];
          bzero(s_command,256);
          strcpy(s_command,"ls /home/zeeshan/FTP_UDP/obj/server");
          strcat(s_command,spath);

          FILE *fp = popen(s_command,"r");
          //check whether the filename exists in the server directory
          while ( fgets( buff, BUFSIZ, fp ) != NULL ) 
          {
            bzero(response,256);
            length=0;
            length=strlen(buff);
            //printf("length in main is %d\n",length );
            buff[length-1]='\0';
            sprintf(response,"%d%s",221,buff);
            writesocket(newsockfd,response);
          }
          pclose(fp);

          sprintf(response,"%d%s",229,"List of files completed");
          printf("%s\n",response );
          writesocket(newsockfd,response);
      }


      if(strcmp(command,"cd")==0&&strcmp(attribute,"..")!=0&&auth_flag==2)
      {
        check_flag=1;
        bzero(s_command,256);
        strcpy(s_command,"/home/zeeshan/FTP_UDP/obj/server");
        strcat(s_command,spath);
        strcat(s_command,attribute);
        //check whether the FOLDER(path is in s_command) exists in the server directory
        if(stat(s_command,&fileStat)==0 && S_ISDIR(fileStat.st_mode))  
          {
            strcat(spath,attribute);
            strcat(spath,"/");
            sprintf(response,"%d%s",239,"Server directory changed");

          }  
        else
          sprintf(response,"%d%s",231,"No such file exists");
        writesocket(newsockfd,response);

      }

      if(strcmp(command,"pwd")==0&&strcmp(attribute,"")==0&&auth_flag==2)
      {
        check_flag=1;
        bzero(response,256);
        sprintf(response,"%d%s",249,spath);
        writesocket(newsockfd,response);
      }

      if(strcmp(command,"cd")==0&&strcmp(attribute,"..")==0&&auth_flag==2)
      {
        /*
          From spath, all the charecters after the rightmost '/' is cleared
        */  
        check_flag=1;
        if(strcmp(spath,"/")!=0)
        {
          length=0;
          length=strlen(spath);
          for(i=length-2;i>=0;i--)
          {
            if(spath[i]=='/')
              break;
          }
          strcpy(temp,spath);
          bzero(spath,256);
          strncpy(spath,temp,i+1);
        } 
        sprintf(response,"%d%s",259,"Directory successfully changed");
        writesocket(newsockfd,response);
      }



      if(strcmp(command,"put")==0&&auth_flag==2)
      {
          check_flag=1;
          readsocket(newsockfd,message);
          sscanf(message,"%d",&filesize);

          strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server");
          strcat(temp,spath);
          strcat(temp,attribute);


          destFD = open(temp,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

          for(i=0;i<filesize;i++)
          {
           read(newsockfd,buff,1);
           //printf("%s\n",*buff );
           write(destFD,buff,1);     //write data to destination file
          }
          close(destFD);
          sprintf(response,"%d%s",329,"File transfer completed");
          writesocket(newsockfd,response);


      }

      if(strcmp(command,"get")==0&&auth_flag==2)
      {
        check_flag=1;
        strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server");
        strcat(temp,spath);
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

          //file is sent byte by byte
          for(i=0;i<filesize;i++)
          {
            read(srcFD,buff,1);
            write(newsockfd,buff,1);
          }
    
          close(srcFD);
        }  
      }

  

      if(check_flag==0&&auth_flag!=1)
      {
        sprintf(response,"%d%s",999,"Invalid command");
        writesocket(newsockfd,response);
      }

      if(check_flag==0&&auth_flag==1)
      {
        sprintf(response,"%d%s",999,"Enter password to proceed");
        writesocket(newsockfd,response);
      }


      if(strcmp(command,"bye")==0)
      {
        sprintf(response,"%d%s",0,"Exit");
        writesocket(newsockfd,response);
      }

  }while(strcmp(command,"bye")!=0);




  return 0;
}
