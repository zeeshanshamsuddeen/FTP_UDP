#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

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


char user[256],pass[256],USER[256],PASS[256];
FILE *fp;
fp=fopen("userlist.txt","r");

bzero(user,256);
read(newsockfd,user,255);

int temp=0;
for(;fscanf(fp,"%s",USER)!=EOF;)
   {
      if(strcmp(user,USER)==0)
         {
            write(newsockfd,"username correct",16);
            bzero(pass,256);
            read(newsockfd,pass,255);
            fscanf(fp,"%s",PASS);
            temp=1;
            if(strcmp(pass,PASS)==0)
               write(newsockfd,"success",7);
            else
               write(newsockfd,"invalid password ",17);
         }  
      else
        {
         fscanf(fp,user,PASS);
        }
   }


if(temp==0)
   write(newsockfd,"invalid username ",17);
fclose(fp);

return 0;
}
