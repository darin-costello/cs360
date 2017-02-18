#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAXGET		    1000

//tests if a character is whitespace;
// whitespace is defined as '\r' \n' ' ' or '\0'
bool isWhitespace(char c)
{
	switch (c)
	{
		case '\r':
		case '\n':
		case ' ':
		case '\0':
			return true;
		default:
			return false;
	}
}

//removes whitespace at the end of a string, and replaces it with '\0'
void trimEndWhite(char *str)
{
	int len = strlen(str);
	while(isWhitespace(str[len]))
	{
		str[len--] = '\0';
	}
}

// reads in a single line from the file pointed to by the passed in file
// descirptor, the char* is creaed via malloc, it is up to the caller
// to free it
char* ReadLine(int fds)
{
	int messagesize = 0;
	int numread = 0;
	char* result;
	char destination[MAXGET];
	while((numread = read(fds, destination + messagesize, 1)) < MAXGET)
	{//reads only one letter 
		if(numread >= 0) 
			messagesize += numread;
		 else
		{
			perror("Socket Error ");
			fprintf(stderr, "Read Failed on file %d", fds);
			exit(2);
		}

		if (destination[messagesize - 1] == '\n')
			break;
	}
	destination[messagesize] = '\0';
		trimEndWhite(destination);
	result = (char*) malloc((strlen(destination) + 1) * sizeof(char));
	if(result == NULL) // we copy the message in a string allocated on the heap
		return NULL;
	strcpy(result, destination);
	return result;
}

// tests if the entire string is a number decimal number containing
// digits 0 - 9. returns false if any letters or non digit symbols are found
bool isnum(char* str){
	int le = strlen(str);
	for(int i = 0; i < le; i++){
		if(!isdigit(str[i]))
			return false;
	}
	return true;
}



int  main(int argc, char* argv[])
{
    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;		// the numberical addres of the host
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    char stringformat[] = "GET %s HTTP/1.0\r\nHOST:%s:%d\r\n\r\n"; // the format of our get request
    extern char *optarg; // used for parsing command line args
    int c, timesToDl = 1, err = 0;
    bool debug = false, print = true;
    extern int optind;
    char page[MAXGET];
    char usage[] = "Usage: [-d] [-c num]  download host port url";
	   


    if(argc < 4) // for is the min number of arguments
    {
	    printf("%s\n", usage);
	    return 0;
    }
    while(( c = getopt(argc, argv, "c:d")) != -1)
    {
	    switch(c)
	    { 
		    case 'c':
			    timesToDl = atoi(optarg);
			    if(!isnum(optarg))
				    err = 1;
			    print = false;
			    break;
		   case 'd':
			    debug = true;
			    break;
		   case '?':
			    err = 1;
			    break;
	    }
    }
    if(err)
    {
	    printf("%s\n", usage);
	    return 0;
    }
   if(optind + 2 >= argc ){
	   printf("%s\n", usage);
	   return 0;
   }
	

     strcpy(strHostName, argv[optind]);
     nHostPort = atoi(argv[optind + 1]);
     strcpy(page, argv[optind +2]); 
     //tests if port is a number, and in valid range
     if(!isnum(argv[optind + 1]) || nHostPort < 1 || nHostPort > 65535 ){
	    fprintf(stderr, "Invalid Port");
	     return 0;
     }

 for(int j = 0; j < timesToDl; j++ ){


    if(debug)
    	printf("\nMaking a socket");

    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    

    if(hSocket == SOCKET_ERROR)
    {
        fprintf(stderr, "\nCould not make a socket\n");
        return 0;
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    if(pHostInfo ==NULL)
    {
	    fprintf(stderr, "\nCan not locate host");
	    return 0;
    }


    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    if(debug)
	    printf("\nconnecting to host:%s",strHostName);

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
       == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
   }

    char message[MAXGET];
    //copies the get request into message
    sprintf(message, stringformat, page, strHostName, nHostPort);
    
    //writes the get requst to the socket
    write(hSocket,message,strlen(message));
    if(debug)
	    printf("\nSending to server:\n%s", message);

    /* read from socket into buffer
    ** number returned by read() and write() is the number of bytes
    ** read or written, with -1 being that an error occured */
    char* line = ReadLine(hSocket); 
	   if(line == NULL){
		  fprintf(stderr, "OUT of memory\n");
		  return 0;
	   }
   if(!strstr(line, "200 OK")) // if first line isn't ok report it
   {
	   perror(line + 9);
   }

    if(debug)
	    printf("\n%s", line);

   while(!strstr(line, "Content-Length")) // read the rest of the header a line at a time. until Contnet length is found
   {
	   free(line);
	   line= ReadLine(hSocket);
	   if(line == NULL){
		  fprintf(stderr, "OUT of memory\n");
		  return 0;
	   }
   	    if(debug)
	   	 printf("\n%s", line);
   }
//parse the length
   char* length = (char*)malloc(MAXGET);
   if(length == NULL) {
	   fprintf(stderr, "Out of memory\n");
	   free(line);
	   return 0;
   } 
		   
   strcpy(length, line + 16);
   int contentL = atoi(length);
   free(length);
   //read till we find an empty line
   while(strlen(line) > 0)
   {
	   free(line);
	   line = ReadLine(hSocket);
	   if(line == NULL){
		  fprintf(stderr, "OUT of memory\n");
		  return 0;
	   }
           if(debug)
		    printf("\n%s", line);
   }
   free(line);

   //grab body of the http response
   char* body = (char*)malloc(sizeof(char) * contentL+1);
   if(body == NULL){
	   fprintf(stderr, "Out of memory\n");
	   return 0;
   }
   char* bodyInd = body;

   body[contentL] = '\0';
   while(contentL){

  	 int numRead = read(hSocket, bodyInd, contentL);
 	contentL -= numRead;
	bodyInd = bodyInd + contentL;
 } 
  if(print || debug)
  	 printf("%s\n", body);
   free(body);
   if(debug)
  	 printf("\nClosing socket\n");

    /* close socket */                       
    if(close(hSocket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
        return 0;
    }
 }
 printf("\nDownloaded file %d times.\n", timesToDl);
}

