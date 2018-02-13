#include <dirent.h>

#include <stdio.h>

#include <string.h>

int main(void)

{

    char lpath[256]="/lfolder1/";
    char temp[256];
    int length,i;

   
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

    printf("%s",lpath);


    return(0);

}