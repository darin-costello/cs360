#include <math.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define HOST_NAME_SIZE      255


double mean(double[], int);
double sd(double[], int);


int  main(int argc, char* argv[])
{

    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    char path[HOST_NAME_SIZE];
    int NSOCKETS = 10;
    int nHostPort;
    bool debug = false;
    char usage[] = "webtest hostname port path [-d] <count>";
    int c, err = 0;
     extern int optind;
     if(argc < 5)
      {
        printf("\n%s\n", usage);
        return 0;
      }
      while(( c = getopt(argc, argv, "d")) != -1)
    {
	    switch(c)
	    { 
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

    strcpy(strHostName,argv[optind]);
    nHostPort=atoi(argv[optind +1]);
    strcpy(path, argv[optind+2]);
	NSOCKETS = atoi(argv[optind + 3]);      


    int *hSocket = (int*)malloc(NSOCKETS * sizeof(int));                 /* handle to socket */
    struct timeval *oldtime = (struct timeval*)malloc(
    	NSOCKETS *sizeof(struct timeval));
    double *deltaTime = (double *)malloc(
    	NSOCKETS * sizeof(double));
    /* make a socket */
	for(int i = 0; i < NSOCKETS; i++) {
	    hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(hSocket[i] == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			free(hSocket);
			free(deltaTime);
			free(oldtime);
			return 0;
		}
	}

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

	int epollFD = epoll_create(1);
	// Send the requests and set up the epoll data
	for(int i = 0; i < NSOCKETS; i++) {
		/* connect to host */
		if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address)) 
		   == SOCKET_ERROR)
		{
			printf("\nCould not connect to host\n");
			free(hSocket);
			free(deltaTime);
			free(oldtime);
			return 0;
		}
		char request[] = "GET /test.html HTTP/1.0\r\n\r\n";

	    write(hSocket[i],request,strlen(request));
		// Keep track of the time when we sent the request
		gettimeofday(&oldtime[i],NULL);
		// Tell epoll that we want to know when this socket has data
		struct epoll_event event;
		event.data.fd = i;
		event.events = EPOLLIN;
		int ret = epoll_ctl(epollFD,EPOLL_CTL_ADD,hSocket[i],&event);
		if(ret)
			perror ("epoll_ctl");
	}
	for(int i = 0; i < NSOCKETS; i++) {
		struct epoll_event event;
		int rval = epoll_wait(epollFD,&event,1,-1);
		if(rval < 0)
			perror("epoll_wait");
		int fd = hSocket[event.data.fd];
		rval = read(fd,pBuffer,BUFFER_SIZE);
		struct timeval newtime;
		// Get the current time and subtract the starting time for this request.
		gettimeofday(&newtime,NULL);
		deltaTime[event.data.fd] = (newtime.tv_sec - oldtime[event.data.fd].tv_sec)*(double)1000000+(newtime.tv_usec-oldtime[event.data.fd].tv_usec);
		// Take this one out of epoll
		epoll_ctl(epollFD,EPOLL_CTL_DEL,fd,&event);

	}
	// Now close the sockets
	for(int i = 0; i < NSOCKETS; i++) {

		/* close socket */                       
		if(close(hSocket[i]) == SOCKET_ERROR)
		{
			printf("\nCould not close socket\n");
			free(hSocket);
			free(deltaTime);
			free(oldtime);
			return 0;
		}
	}
	if(debug){
		for(int i = 0; i < NSOCKETS; i++) {
			printf("\nConnection %d's time: \t %f", i, deltaTime[i]);
		}
	}
	printf("\nMean: %f\nSd: %f\n", mean(deltaTime, NSOCKETS), sd(deltaTime,NSOCKETS));
	free(hSocket);
    free(deltaTime);
	free(oldtime);
}


double mean(double data[], int count){
	int i = 0;
	double sum = 0.0;
	for(;i < count;i++)
		sum += data [i];
	return sum / count;
}

double sd(double data[], int count){
	double meanDat = mean(data, count);
	double sum = 0.0;
	for(int i = 0; i < count; i++){
		double diff = data[i] - meanDat;
		sum += (diff * diff);
	}
	double result = sum / count;
	return sqrt(result);

}