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
int main() 
{
int sockfd, newsockfd, portno, clilen;
struct sockaddr_in serv_addr, cli_addr;
char command[256],names[256],buffer[256];
char temp[256],empty[256]="";
char spath[256]="/";
char resp[256];
char temp1[256];
char user[256],pass[256],USER[256],PASS[256];

int  n;
int temps=0;
int length,i,check=0; 
int iSetOption = 1;
int leng[10],pause[10];
int srcFD,destFD,nbread,nbwrite;
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
portno = 5004;

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






//username and password check
FILE *fp;
fp=fopen("/home/zeeshan/FTP_UDP/obj/userlist.txt","r");
temps=0;
bzero(user,256);
read(newsockfd,user,255);
bzero(USER,256);

for(;fscanf(fp,"%s",USER)!=EOF;)
   {
      if(strcmp(user,USER)==0)
         {
            write(newsockfd,"username correct",16);
            bzero(pass,256);

            read(newsockfd,pass,255);
            bzero(PASS,256);
            fscanf(fp,"%s",PASS);
            temps=1;
            if(strcmp(pass,PASS)==0)
               write(newsockfd,"230 Login successful.",21);
            else
               write(newsockfd,"invalid password",16);
         }  
      else
        {
         fscanf(fp,"%s",PASS);
        }
   }

if(temps==0)
{
   write(newsockfd,"invalid username",16);
}
fclose(fp);








// When user is authenticated
do
{
 
bzero(names,256); 
bzero(command,256);
read(newsockfd,command,255);
check=0;

//to display files in server directory ie. "ls"
if(strcmp(command,"ls")==0)
{
    check=1;
    DIR *d;
    struct dirent *dir;
    bzero(temp,256);
    strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server");
    strcat(temp,spath);
    d = opendir(temp);   
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            strcat(names,dir->d_name);
            strcat(names,"\n"); 
        }
        closedir(d);
    }
      write(newsockfd,"150 Here comes the directory listing.\n",38);
      bzero(temp,256);
      read(newsockfd,temp,255);
      write(newsockfd,names,strlen(names));

}




//to change server directory ie. "cmd xx"
if(command[0]=='c'&&command[1]=='d'&&command[3]!='.'&&command[3]!=' '&&strlen(command)>3)
{
  check=1;
  bzero(temp,256);
  strcpy(temp1,spath);
  strncpy(temp,command+3,strlen(command));    //copy command[3] onwards to temp
  strcat(spath,temp);
  strcat(spath,"/");
  DIR *d;
  bzero(temp,256);
  strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server/");
  strcat(temp,spath);

  d = opendir(temp);   
  if (d)
  {
    write(newsockfd,empty,1);  
    bzero(temp,256);

    read(newsockfd,temp,255);
    write(newsockfd,"250 Directory successfully changed.",35);
    closedir(d);
  }
  else
  { 
    write(newsockfd,empty,1);
    bzero(temp,256);
    read(newsockfd,temp,255);
    write(newsockfd,"550 Failed to change directory.",31);
    strcpy(spath,temp1);
  }  

}



//to show current server directory ie. "pwd"
if(command[0]=='p'&&command[1]=='w'&&command[2]=='d'&&strlen(command)<4)
{
  check=1;
  bzero(temp,256);
  strcpy(temp,"257 \"");;
  strcat(temp,spath);
  write(newsockfd,temp,strlen(temp));
  bzero(temp,256);
  read(newsockfd,temp,255);
  write(newsockfd,"\" is the current directory.",27);
}





//to change back to previous server directory ie. "cd .."
if(command[0]=='c'&&command[1]=='d'&&command[2]==' '&&command[3]=='.'&&command[4]=='.')
{
  check=1;
  if(strcmp(spath,"/")!=0)
  {
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
  write(newsockfd,empty,1);  
  bzero(temp,256);
  read(newsockfd,temp,255);
  write(newsockfd,"250 Directory successfully changed.",35);


}


//to recieve files send by client ie. "put xxx"
if(command[0]=='p'&&command[1]=='u'&&command[2]=='t'&&strlen(command)>4)
{
  check=1;
  write(newsockfd,empty,1);
  bzero(temp1,256);
  bzero(temp,256);
  bzero(leng,10);
  bzero(pause,10);
  pause[0]=0;
  /*
    used to indicate whether client is sending data to server
    pause = -1 ::: no such file exists
    pause = 0  ::: sending data
    pause = 1  ::: end of data  
  */
  strncpy(temp1,command+4,strlen(command));    //copy command[4] onwards to temp1

  strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server/");
  strcat(temp,spath);
  strcat(temp,temp1);


  read(newsockfd,pause,10);
  if(pause[0]==-1)
  {
    write(newsockfd,empty,1);
  }
  else
  {
    /*
      Open destination file with respective flags & modes
      O_CREAT & O_TRUNC is to truncate existing file or create a new file
      S_IXXXX are file permissions for the user,groups & others
    */
    destFD = open(temp,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    while(pause[0]==0)
    {   
      write(newsockfd,empty,1);
      bzero(leng,10);
      read(newsockfd,leng,10);
      write(newsockfd,empty,1);   
      read(newsockfd,buff,1024);
      write(newsockfd,empty,1);
      
      write(destFD,buff,leng[0]);     //write data to destination file
      
      read(newsockfd,pause,10);


    }
   close(destFD);
   write(newsockfd,empty,1);
  }

} 



//to send files to client machine ie. "get xxx"
if(command[0]=='g'&&command[1]=='e'&&command[2]=='t'&&strlen(command)>4)
{
  check=1;

  bzero(temp1,256);
  bzero(temp,256);
  bzero(leng,10);
  pause[0]=0;   
  /*
    used to indicate whether client is sending data to server
    pause = -1 ::: no such file exists
    pause = 0  ::: sending data
    pause = 1  ::: end of data  
  */
  strncpy(temp1,command+4,strlen(command));    //copy command[4] onwards to temp

  strcpy(temp,"/home/zeeshan/FTP_UDP/obj/server/");
  strcat(temp,spath);
  strcat(temp,temp1);

  /*Open source file*/
  srcFD = open(temp,O_RDONLY);
  if(srcFD==-1)
  {
    pause[0]=-1;
    write(newsockfd,pause,10);  
  }
  else
  {
    /*Start reading data from src file till it reaches EOF*/
    while((nbread = read(srcFD,buff,1024)) > 0)
    {

      write(newsockfd,pause,10);
      bzero(temp1,256);
      read(newsockfd,temp1,255);
      
      leng[0]=nbread; 
      write(newsockfd,leng,10);
      read(newsockfd,temp,255);  
      write(newsockfd,buff,nbread) ;
      read(newsockfd,temp,255);
    }

    //data transfer complete
    pause[0]=1;           //end of data
    write(newsockfd,pause,10);

    close(srcFD);
  }

}  






//end of FTP
if(strcmp(command,"bye")==0)
{
  check=1;
  write(newsockfd,empty,1);
  bzero(temp,256);
  read(newsockfd,temp,255);
  write(newsockfd,empty,1);
}


//invalid command
if(check==0)
{
  write(newsockfd,"invalid command",15);
  bzero(temp,256);
  read(newsockfd,temp,255);
  write(newsockfd,empty,1);
}


}while(strcmp(command,"bye")!=0);

return 0;
}
