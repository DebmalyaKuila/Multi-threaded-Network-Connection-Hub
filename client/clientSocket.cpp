#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include <unistd.h>
#include<pthread.h>
#include <sstream>
using namespace std;

void startListeningAndPrintChatOnNewThread(int socketFD);
void *listenAndPrint(void *sockFdAddr);

int main(){

    int socketFD=socket(AF_INET,SOCK_STREAM,0);

    const char* ip="127.0.0.1"; //localhost 

    struct sockaddr_in serverAddr; //server you want to connect with
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(2000);  //default port to listening of google server
    inet_pton(AF_INET,ip,&serverAddr.sin_addr.s_addr);

    int result=connect(socketFD,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if(result==0)cout<<"connection established"<<endl;
    else {
        cout<<"connection failed"<<endl;
        exit(1);
    }

    char* username=nullptr;
    size_t usernameSize=0;
    cout<<"Enter your username : "<<endl;
    ssize_t usernameCount=getline(&username,&usernameSize,stdin);
    username[usernameCount-1]=0;
    
    

    startListeningAndPrintChatOnNewThread(socketFD);

    char* line=nullptr;
    size_t lineSize=0;
    cout<<"Type your message and hit enter to send message..."<<endl<<"(Type exit() and enter to close chat)"<<endl;
    
    char buffer[1024];
    while(true){
        ssize_t charCount=getline(&line,&lineSize,stdin);
        line[charCount-1]=0;
        sprintf(buffer,"%s : %s",username,line);
        if(charCount>0){
            if(!strcmp(line,"exit()"))break;
        }
        int bytesSent=send(socketFD,buffer,strlen(buffer),0);
    }

    close(socketFD);


    return 0;
}


void startListeningAndPrintChatOnNewThread(int socketFD){
    pthread_t id;
    cout << "Starting new thread to listen for messages..." << endl;  // Debug print
    int *sockFDAddr = new int(socketFD);  // Dynamically allocate a copy of the socket FD
    int result=pthread_create(&id,NULL,listenAndPrint,(void*)sockFDAddr);
    if (result != 0) {
        perror("Error creating thread");
        exit(1);
    }
}

void *listenAndPrint(void *sockFDAddr){
    int socketFD = *((int*)sockFDAddr);
    delete (int*)sockFDAddr; 
    char buff[1024];
    while(true){
        int bytesRecv = recv(socketFD, buff, 1024, 0);
        if (bytesRecv > 0) {
            cout<<buff << endl;  // Debug print
        } else if (bytesRecv == 0) {
            cout << "Server closed the connection." << endl;
            break;  // Connection closed, exit loop
        }
    }
    cout << "Closing client socket: " << socketFD << endl;
    close(socketFD);

    return NULL;
}