#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>

#include <string.h>

int main() 
{
int sockfd, newsockfd, portno, clilen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
int  n;

int iSetOption = 1;
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
char user[256],pass[256],USER[256],PASS[256];

int temps=0;


FILE *fp;

fp=fopen("userlist.txt","r");
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








char command[256],names[256];
char temp[256],empty[256]="";
char spath[256]="/",lpath[256]="/";
char resp[256];
char temp1[256];
int length,i,check=0; 

do
{
 
bzero(names,256); 
bzero(command,256);
read(newsockfd,command,255);
check=0;


//to display files in server directory
if(strcmp(command,"ls")==0&&strlen(command)<3)
{
    check=1;
    DIR *d;
    struct dirent *dir;
    bzero(temp,256);
    strcpy(temp,"./zeeshan");
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




//to change server directory
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
  strcpy(temp,"./zeeshan");
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



//to show current server directory
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





//to change back to previous server directory
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


















//to display files in client directory
if(strcmp(command,"!ls")==0&&strlen(command)<4)
{
    check=1;
    DIR *d;
    struct dirent *dir;
    bzero(temp,256);
    bzero(names,256);
    strcpy(temp,"/home/zeeshan/FTP_UDP/client");
    strcat(temp,lpath);
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
      write(newsockfd,empty,1);
      bzero(temp,256);
      read(newsockfd,temp,255);
      write(newsockfd,names,strlen(names));

}









//to change client directory
if(command[0]=='l'&&command[1]=='c'&&command[2]=='d'&&command[3]==' '&&command[4]!='.'&&command[4]!=' '&&strlen(command)>4)
{
  check=1;
  bzero(temp,256);
  strcpy(temp1,lpath);
  strncpy(temp,command+4,strlen(command));    //copy command[3] onwards to temp
  strcat(lpath,temp);
  strcat(lpath,"/");
  DIR *d;
  bzero(temp,256);
  strcpy(temp,"/home/zeeshan/FTP_UDP/client");
  strcat(temp,lpath);

  d = opendir(temp);   
  if (d)
  {
    write(newsockfd,"Local directory now ",20);  
    bzero(temp,256);

    read(newsockfd,temp,255);
    write(newsockfd,lpath,strlen(lpath));
    closedir(d);
  }
  else
  { 
    write(newsockfd,"local: ",7);
    bzero(temp,256);
    read(newsockfd,temp,255);
    write(newsockfd,"No such file or directory ",26);
    strcpy(lpath,temp1);
  }  

}






//to show current client directory
if(command[0]=='!'&&command[1]=='p'&&command[2]=='w'&&command[3]=='d'&&strlen(command)<5)
{
  check=1;
  bzero(temp,256);
  write(newsockfd,lpath,strlen(lpath));
  bzero(temp,256);
  read(newsockfd,temp,255);
  write(newsockfd,empty,1);
}





//to change back to previous client directory
if(command[0]=='l'&&command[1]=='c'&&command[2]=='d'&&command[3]==' '&&command[4]=='.'&&command[5]=='.')
{
  check=1;
  if(strcmp(lpath,"/")!=0)
  {
    length=strlen(lpath);
    for(i=length-2;i>=0;i--)
    {
      if(lpath[i]=='/')
        break;
    }
    strcpy(temp,lpath);
    bzero(lpath,256);
    strncpy(lpath,temp,i+1);
  } 
  write(newsockfd,"Local directory now ",20);  
  bzero(temp,256);
  read(newsockfd,temp,255);
  write(newsockfd,lpath,strlen(lpath));


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
