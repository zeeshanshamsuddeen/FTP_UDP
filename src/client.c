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
int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;
char user[256],pass[256],message[256];
char command[256],command2[256];
char resp[256],op[256],empty[256]="";
char temp[256],temp1[256],lpath[256]="/",names[256];
int length,i;
int iSetOption = 1;

char buffer[256];
portno = 5004;


/* Create a socket point */
sockfd = socket(AF_INET, SOCK_STREAM, 0);
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

if (sockfd < 0)
	 {
	  perror("ERROR opening socket");
	  exit(1);
	 }

server = gethostbyname("127.0.0.1");			// returns pointer to corresponding host

if (server == NULL) 
	{
	  fprintf(stderr,"ERROR, no such host\n");
	  exit(0);
	}

bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
serv_addr.sin_port = htons(portno);

/* Now connect to the server */
if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
	{
	  perror("ERROR connecting");
	  exit(1);
	}





//username and password check 
printf("Enter username : ");
bzero(user,256);
fgets(user,255,stdin);
length=strlen(user);
user[length-1]='\0';
write(sockfd,user,strlen(user));
bzero(buffer,256);
read(sockfd,buffer,255);


//if the entered username is username is correct
if(strcmp(buffer,"username correct")==0)	
{
	printf("Enter password : ");
	bzero(pass,256);
	//scanf("%s",pass);
	fgets(pass,255,stdin);
	length=strlen(pass);
	pass[length-1]='\0';
	write(sockfd,pass,strlen(pass));
	bzero(message,256);
	read(sockfd,message,255);
	printf("%s",message);
}
else
{
	printf("%s",buffer);		
}



//if successfull authentication
if(strcmp(message,"230 Login successful.")==0)
{	
do
{

printf("\nftp> ");
fgets(command,255,stdin);
length=strlen(command);
command[length-1]='\0';


if(strcmp(command,"!ls")==0)
{
	//to display files in client directory ie. "!ls"
    DIR *d;
    struct dirent *dir;
    bzero(temp,256);
    bzero(names,256);
    strcpy(temp,"./client");
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
     printf("%s",names);





}
else if(command[0]=='l'&&command[1]=='c'&&command[2]=='d'&&command[3]==' '&&command[4]!='.'&&command[4]!=' '&&strlen(command)>4)
{
	//to change client directory ie. "!lcd xx"
	bzero(temp,256);
	strcpy(temp1,lpath);
	strncpy(temp,command+4,strlen(command));    //copy command[4] onwards to temp
	strcat(lpath,temp);
	strcat(lpath,"/");
	DIR *d;
	bzero(temp,256);
	strcpy(temp,"./client");
	strcat(temp,lpath);

	d = opendir(temp);   
	if (d)
	{
		printf("Local directory now ");
	printf("%s",lpath);
	closedir(d);
	}
	else
	{ 
		printf("local : No such file or directory ");
	strcpy(lpath,temp1);
	}  







}
else if(command[0]=='!'&&command[1]=='p'&&command[2]=='w'&&command[3]=='d'&&strlen(command)<5)
{
	//to show current client directory ie. "!pwd"
	printf("%s",lpath);



}
else if(command[0]=='l'&&command[1]=='c'&&command[2]=='d'&&command[3]==' '&&command[4]=='.'&&command[5]=='.')
{


	//to change back to previous server directory ie. "lcd .."
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
	printf("Local directory now %s",lpath);




}
else
	{
		// If the entered command is not a local directory based command
		write(sockfd,command,strlen(command));

		bzero(resp,256);
		read(sockfd,resp,255);
		printf("%s",resp);

		write(sockfd,empty,1);

		bzero(op,256);
		read(sockfd,op,255);
		printf("%s",op);
	}	



}while(strcmp(command,"bye")!=0);

}
return 0;
}
