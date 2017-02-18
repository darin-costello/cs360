#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <dirent.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <vector>             
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <queue>

using namespace std;

#define MAX_MSG_SZ      1024

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define QUEUE_SIZE          20
#define NQUEUE 20


string path;
char message[] = "HTTP/1.0 404 Not Found\r\nContent-length:%d\r\nContent-Type:text/html\r\n\r\n";
char message404[] ="<!DOCTYPE html><html><body>404 page not found</body></html>";

class myqueue {
	queue <int> stlqueue;
	sem_t empty, full, mutex;
	public:
	myqueue() {
		sem_init(&mutex, PTHREAD_PROCESS_PRIVATE, 1);
		sem_init(&full, PTHREAD_PROCESS_PRIVATE, 0);
		sem_init(&empty, PTHREAD_PROCESS_PRIVATE, NQUEUE);
	}
	~myqueue() {
		sem_close(&empty);
		sem_close(&full);
		sem_close(&mutex);
	}
	void push (int sock) {
		sem_wait(&empty);
		sem_wait(&mutex);
		stlqueue.push(sock);
		sem_post(&mutex);
		sem_post(&full);
	}
	int pop() {
		sem_wait(&full);
		sem_wait(&mutex);
		int result = stlqueue.front();
		stlqueue.pop();
		sem_post(&mutex);
		sem_post(&empty);
		return result;
	}
} sockqueue;

// Determine if the character is whitespace
bool isWhitespace(char c)
{ switch (c)
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

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
	
        if (amtread > 0){
            messagesize += amtread;}
        else if(amtread == 0){
        	break;
	} else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}

void getFile(char* buffer, const char* file, int size){
	
	FILE *fp = fopen(file, "r");
	fread(buffer, size, 1, fp);
	fclose(fp);
	return;
	
}

char* printDir(string file, string path){
	struct stat filestat;
	if(stat(file.c_str(), &filestat)) {
		cout <<"ERROR in stat\n";
		return NULL;
	}

	if(S_ISDIR(filestat.st_mode)) {
  		 int len;
 		 DIR *dirp;
 		 struct dirent *dp;
		 stringstream ss;
 		 dirp = opendir(file.c_str());
		 ss << "<!DOCTYPE html>" << endl
			 << "<html><body><ui>";
		  while ((dp = readdir(dirp)) != NULL){ 
			if(!strcmp("..", dp->d_name) ||
			!strcmp(".", dp->d_name)) {
				continue;
			}
			 ss << "<li><a href=\"" << path << "/"
				 << dp->d_name
				 << "\">" << dp->d_name << "</a></li>";
		  }
		  ss << "</ul></body></html>" << endl;
  		   (void)closedir(dirp);
		   string temp = ss.str();
//		   cout << temp << endl;
		   char* buffer = (char*) malloc(temp.length() + 1);
		   strcpy(buffer, temp.c_str());
		   buffer[temp.length()] = '\0';
		   return buffer;
	} else {
		return NULL;
	}
}

bool hasIndex(string fileName){
	struct stat filestat;
	if(stat(fileName.c_str(), &filestat)) {
		cout <<"ERROR in stat\n";
		return false;
	}
	bool result = false;
	if(S_ISDIR(filestat.st_mode)) {
 		 DIR *dirp;
 		 struct dirent *dp;
 		 dirp = opendir(fileName.c_str());
		 while ((dp = readdir(dirp)) != NULL){
			if (strcmp(dp->d_name, "index.html") == 0){
  				 (void)closedir(dirp);
				return true;
			}
		}
  		   (void)closedir(dirp);
	}
	return result;
}

int closeSocket(int socket){
 /* close socket */
		printf("%s\n", "Clossing Socket");
		shutdown(socket, SHUT_RDWR);
        if(close(socket) == SOCKET_ERROR)
        {
         return -1;
        }
	return 0;
}

string getType(string file){
	if(file.length() < 4)
		return "text/plain";
	string test = file.substr(file.length()-5);
	if(file.rfind(".html", string::npos) != string::npos)
		return "text/html";
	test = test.substr(1, string::npos);
	if(test == ".txt")
		return "text/plain";
	if(test == ".jpg")
		return "image/jpg";
	if(test == ".gif")
		return "image/gif";

	return "test/plain";
}

void *handleRequest(void* arg){
	for(;;) {
		int hSocket = sockqueue.pop();
		char* line = GetLine(hSocket);
		if(!strlen(line)){
			free(line);
			closeSocket(hSocket);
//			printf("%s\n", "empty socket");
			continue;
		}	
		stringstream ss;
		string get;
		string url;
		ss << line;
		ss >> get;
		

		 while(strlen(line) > 0)
   		{
//			 printf("%s\n", line);
	  		 free(line);
	   		 line = GetLine(hSocket);
		}
		free(line);
				
		
		if(get != "GET") {
			string message = "HTTP/1.0 501 Not Implemented\r\n\r\n";
			int nu = write(hSocket,message.c_str(),
				message.size() + 1);
//			printf("%d\n%s\n",nu, message.c_str());
			closeSocket(hSocket);
			continue;
		}
		
		ss >> url;
		string burl = url;
		url = path + url;
//		printf("%s\n", url.c_str());
		if(burl == "/")
			burl = ".";

		struct stat filestat;
		if(stat(url.c_str(), &filestat)) {
			
			char* header = (char *) malloc(500);
			sprintf(header, message, strlen(message404));
			write(hSocket, header,
					strlen(header));
			write(hSocket, message404, strlen(message404));
			free(header);
			closeSocket(hSocket);
			continue;
		}

		char* body;
		int size;
		string contentType;
		if(S_ISREG(filestat.st_mode)) {
			body = (char*) malloc(filestat.st_size);
			size = filestat.st_size;
			getFile(body, url.c_str(), size);
			contentType = getType(url);
		}else if(S_ISDIR(filestat.st_mode)) {
			 contentType = "text/html";
			if(hasIndex(url)){
				url += "/index.html";
				stat(url.c_str(), &filestat);
				body = (char*) malloc(filestat.st_size);
				size = filestat.st_size;
				getFile(body, url.c_str(), size);
				contentType = getType(url);
			} else {
				body = printDir(url, burl);
				size = strlen(body);
			}
					

		}	
		char* header = (char *) malloc(500);
		sprintf(header,
			"HTTP/1.0 200\r\nContent-Length:%d\r\nContent-Type:%s\r\n\r\n", size,
			contentType.c_str());
		write(hSocket, header, strlen(header));
		write(hSocket, body, size);
		free(header);
		free(body);
		
		closeSocket(hSocket);
	}
}
void handler (int status)
{
		printf("received signal %d\n",status);
}
int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;
    int NTHREADS = 10;


    if(argc < 4)
      {
        printf("\nUsage: server port-num num-threads  path\n");
        return 0;
      }
    nHostPort=atoi(argv[1]);
    NTHREADS = atoi(argv[2]);
    string tempst(argv[3]);
    path = tempst;
    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);
// First set up the signal handler
    struct sigaction sigold, signew;
    signew.sa_handler=handler;
    sigemptyset(&signew.sa_mask);
    sigaddset(&signew.sa_mask,SIGPIPE);
    signew.sa_flags = SA_RESTART;
    sigaction(SIGPIPE,&signew,&sigold);
    
    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d",nHostPort);

    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,
		    (socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",
		    hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    linger lin;
    unsigned int y=sizeof(lin);
    lin.l_onoff=1;
    lin.l_linger=10;
    setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
    
    int threadid;
    pthread_t threads[NTHREADS];
    for(threadid = 0; threadid < NTHREADS; threadid++) {	
	    pthread_create(&threads[threadid], NULL, 
			    handleRequest,(void*) threadid);
    }
    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,
			(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
		sockqueue.push(hSocket);
	}
   		
       
    

}

