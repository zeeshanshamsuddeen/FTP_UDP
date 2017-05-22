#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>

#include <string.h>

int main()
{
    char temp[256],temp1[256],buffer[256],lpath[256]="/",command[256]="put test.txt";

        
    FILE *f;
    bzero(temp,256);
    bzero(buffer,100);
    bzero(temp1,256);
    strcpy(temp,"/zeeshan");
    strcat(temp,lpath);
    strncpy(temp1,command+4,strlen(command));    //copy command[4] onwards to temp1
    strcat(temp,temp1);
    printf("%s",temp);
    f=fopen("zeeshan/test.txt","r");
    fscanf(f,"%s",buffer);
    //printf("%s",buffer);
    //fclose(f);


return 0;
 }   