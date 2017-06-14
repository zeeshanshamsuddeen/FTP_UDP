#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>
#include <arpa/inet.h>


#include <string.h>


#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

void writesocket(int,char[]);
void readsocket(int,char[]);
void readline(char[]);

void fn_ls();
void fn_lcd();
void fn_pwd();
void fn_lcd_dot();
void fn_put(int);
void fn_get(int);

struct sockaddr_in serv_addr;
struct hostent *server;
struct in_addr inp;
struct stat fileStat;

int temp_sockfd, temp_newsockfd, temp_clilen;
struct sockaddr_in temp_serv_addr, temp_cli_addr;


char user[256],pass[256],message[256];
char command[256],command2[256],response[256],attribute[256];
char l_command[256],enquiry[256];
char resp[256],op[256],empty[256]="";
char temp[256],temp1[256],temp_path[256],lpath[256]="/",names[256];
char USER[256];

int length,i,n;
int code;
int iSetOption = 1;
int leng[10];
int filesize;
int auth_flag=0,af[1];
	char buffer[256];
DIR *d;

int srcFD,destFD,nbread,nbwrite;
char *buff[1024];



int main(int argc,char *argv[])
{
	int newsockfd,sockfd, portno;
	
	portno = 5001;




	/* Create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));



	if (inet_aton((argv[1]),&inp) ==0) 		// returns pointer to corresponding host
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	bcopy((char *)&inp , (char *)&serv_addr.sin_addr.s_addr, sizeof(inp));

	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
	{
		perror("ERROR connecting");
		exit(1);
	}

	do
	{

		/*
			auth_flag is used to indicate whether the user is authenticated or nor.
			auth_flag = 0 : not authenticated
			auth_flag = 1 : username verified, password required
			auth_flag = 2 : user authenticated 
		*/
				
		bzero(message,256);
		bzero(command,256);
		bzero(attribute,256);

		readline(message);
		sscanf(message,"%s%s",command,attribute);

		if(strcmp(command,"!ls")==0&&strlen(message)<4&&message[4]!=' '&&auth_flag==2)
			fn_ls();
		else if(strcmp(command,"lcd")==0&&strcmp(attribute,"..")!=0&&message[4]!='.'&&strcmp(attribute,"\0\n")!=0&&auth_flag==2)
			fn_lcd();
		else if(strcmp(command,"!pwd")==0&&strlen(message)<5&&message[4]!=' '&&auth_flag==2)
			fn_pwd();
		else if(strcmp(command,"lcd")==0&&strcmp(attribute,"..")==0&&auth_flag==2)
			fn_lcd_dot();
		else if(strcmp(command,"put")==0&&message[4]!=' '&&strlen(message)>4&&auth_flag==2)
			fn_put(sockfd);
		else if(strcmp(command,"get")==0&&message[4]!=' '&&strlen(message)>4&&auth_flag==2)
			fn_get(sockfd);
		else
		{
			writesocket(sockfd,message);
			bzero(response,256);
			readsocket(sockfd,response);
			sscanf(response,"%d%[^\n]",&code,message);

			//All other commands are inactive untill authentication is successful ie.code is 219
			if(code==219)
			{
				auth_flag=2;
				strcpy(USER,message);
				readsocket(sockfd,response);
				sscanf(response,"%d%[^\n]",&code,message);
			}	
			if(code==209)
				auth_flag=1;
	
			while(code==221)
			{	
				/* When the server is sending the file/folder names for ls command */
				printf("%s\n",message);
				bzero(response,256);
				readsocket(sockfd,response);
				sscanf(response,"%d%[^\n]",&code,message);

			}
		}
		
		if(code==999&&auth_flag==0)
			printf("Please Login to proceed\n");
		else
			printf("%d %s\n",code,message);
	

	}while(code!=0);


	return 0;
}	






void readsocket(int sockfd,char buffer[])
{
    bzero(buffer,256);
    char temp[1];
    bzero(temp,1);
    int i=0;
    do
    {
      read(sockfd,temp,1);
      buffer[i]=temp[0];
      i++;
      //printf("%c",temp[0] );
     
    }while(temp[0]!='\n');

   
      
    read(sockfd,temp,1);
    //if(temp[0]=='\n')
    // printf("new line received" );
      
    if(temp[0]!='\0')
      {
        printf("ERROR no /0 in read after %s",buffer);
        exit(1);
      }
    buffer[i-1]='\0';  
  
  /* At the end of this function, buffer will be a normal string with \0 at the end  */  
}


void writesocket(int sockfd,char buffer[])
{
	int length;	
	length=strlen(buffer);
	
	buffer[length]='\n';
	buffer[length+1]='\0';
	length++;
	/*
		printf("%d",length);
		for(int i=0;i<=length;i++)
		if(buffer[i]=='\n')
			printf("new line in %dth position\n",i );
		for(int i=0;i<=length;i++)
		if(buffer[i]=='\0')
			printf("new zero in %dth position",i );
	*/
	write(sockfd,buffer,length+1);

}


void readline(char buffer[])
{

	int length=0;
	bzero(buffer,256);
	fgets(buffer,255,stdin);
	length=strlen(buffer);
	buffer[length-1]='\0';
	
}



void fn_ls()
{
	char buff[BUFSIZ];
	bzero(l_command,256);
	strcpy(l_command,"ls /home/zeeshan/FTP_UDP/obj/client/");
	strcat(l_command,USER);
	strcat(l_command,lpath);

	FILE *fp = popen(l_command,"r");
	//check whether the filename exists in the server directory
	while ( fgets( buff, BUFSIZ, fp ) != NULL ) 
	{
	    /*  Each file/folder name is retrieved and is printed   */
		bzero(response,256);
		length=0;
		length=strlen(buff);
		buff[length-1]='\0';
		printf("%s\n",buff);
	}
	pclose(fp);
	code=129;
	strcpy(message,"List of files completed");
}


void fn_lcd()
{
	strncpy(attribute,message+4,strlen(message));
	bzero(temp_path,256);
	strcpy(temp_path,"/home/zeeshan/FTP_UDP/obj/client/");
	strcat(temp_path,USER);
	strcat(temp_path,lpath);
	strcat(temp_path,attribute);

	//check whether the FOLDER (path is in temp_path) exists in the server directory
	if(stat(temp_path,&fileStat)==0 && S_ISDIR(fileStat.st_mode))  
	  {
	    strcat(lpath,attribute);
	    strcat(lpath,"/");
	    code=139;
	    strcpy(message,"Server directory changed");
	  }  
	else
	 {
	 	code=131;
	 	strcpy(message,"No such file exists");
	 }

	}
	void fn_pwd()
	{
	code=149;
	strcpy(message,lpath);
}


void fn_lcd_dot()
{
	/*	From lpath, all the charecters after the rightmost '/' is cleared	*/
	if(strcmp(lpath,"/")!=0)
	{
	  length=0;
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
	code=159;
	strcpy(message,"Directory successfully changed");

}


void fn_put(int sockfd)
{
	strncpy(attribute,message+4,strlen(message));

	strcpy(temp_path,"/home/zeeshan/FTP_UDP/obj/client/");
	strcat(temp_path,USER);
	strcat(temp_path,lpath);
	strcat(temp_path,attribute);

	//opens the directory (path is in temp_path)
	srcFD = open(temp_path,O_RDONLY);
	if(srcFD==-1)
	{
		code=321;
		strcpy(message,"Invalid filename");
	}
	else
	{
		writesocket(sockfd,message);
		stat(temp_path, &fileStat);

		//size of file is calculated in filesize and sent to the server
		filesize = fileStat.st_size;
		sprintf(message,"%d",filesize);
		writesocket(sockfd,message);

		//file is sent byte by byte 
		for(i=0;i<filesize;i++)
		{
			read(srcFD,buff,1);
			write(sockfd,buff,1);
		}

		close(srcFD);
		readsocket(sockfd,response);
		sscanf(response,"%d%[^\n]",&code,message);

	}
	
}



void fn_get(int sockfd)
{

	writesocket(sockfd,message);
	strncpy(attribute,message+4,(strlen(message)));
	/*
		After writesocket(), message[] will have \n at its last position,
		This is replaced by \0
	*/
	length=strlen(attribute);
	attribute[length-1]='\0';

	readsocket(sockfd,response);
	sscanf(response,"%d%[^\n]",&code,message);

	if(strcmp(message,"Valid filename")==0)
	{
		sprintf(message,"%d",6000);
		writesocket(sockfd,message);
		filesize=code;


				/* First call to socket() function */
			   temp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
			   
			   if (temp_sockfd < 0) {
			      perror("ERROR opening socket");
			      exit(1);
			   }
			   
			   /* Initialize socket structure */
			   bzero((char *) &temp_serv_addr, sizeof(temp_serv_addr));
			   
			   temp_serv_addr.sin_family = AF_INET;
			   temp_serv_addr.sin_addr.s_addr = INADDR_ANY;
			   temp_serv_addr.sin_port = htons(6000);

			   setsockopt(temp_sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

			   
			   /* Now bind the host address using bind() call.*/
			   if (bind(temp_sockfd, (struct sockaddr *) &temp_serv_addr, sizeof(temp_serv_addr)) < 0) {
			      perror("ERROR on binding");
			      exit(1);
			   }
			      
			   /* Now start listening for the clients, here process will
			      * go in sleep mode and will wait for the incoming connection
			   */
			   
			   listen(temp_sockfd,5);
			   temp_clilen = sizeof(temp_cli_addr);
			   
			   /* Accept actual connection from the client */
			   temp_newsockfd = accept(temp_sockfd, (struct sockaddr *)&temp_cli_addr, &temp_clilen);
				
			   if (temp_newsockfd < 0) {
			      perror("ERROR on accept");
			      exit(1);
			   }


		strcpy(temp_path,"/home/zeeshan/FTP_UDP/obj/client/");
		strcat(temp_path,USER);
		strcat(temp_path,lpath);
		strcat(temp_path,attribute);


		destFD = open(temp_path,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		for(i=0;i<filesize;i++)
		{
			read(temp_newsockfd,buff,1);
			write(destFD,buff,1);     //write data to destination file
		}
		close(destFD);
		close( temp_newsockfd );
		close(temp_sockfd);


		code=339;
		strcpy(message,"File transfer completed");

	}
	else
		code=331;
} 
		
