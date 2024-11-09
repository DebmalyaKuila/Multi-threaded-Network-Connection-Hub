#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include <unistd.h>
#include<pthread.h>
using namespace std;

int port=2000;

struct acceptedSocket{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    int acceptedSuccesfully;
};
void recvAndPrintIncomingChatOnSeprateThread(int clientSocketFD);
struct acceptedSocket* acceptIncomingConnection(int serverSocketFD);
void *acceptNewConnectionAndRecvAndPrintChat(void* serverSockFDAddr);
void recvAndPrintIncomingChat(int clientSocketFD);
void sendRecvMsgToOtherClients(int incomingMsgclientSocketFD,char*buffer);

struct acceptedSocket clientSockets[10];
int clientSocketCount=0;

int main(){

    int serverSocketFD=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(port);
    serverAddr.sin_addr.s_addr=INADDR_ANY; //listen for any incoming address
    int res=bind(serverSocketFD,(const sockaddr*)&serverAddr,sizeof(serverAddr));
    if(res==0)cout<<"Binding successful"<<endl;
    else{
        cout<<"Binding Failed"<<endl;
        exit(1);
    }    int result=listen(serverSocketFD,10);
    if(result==0)cout<<"server listening on port:"<<port<<endl;
    else{
        cout<<"server failed to listen"<<endl;
        exit(1);
    }

    //start accepting incoming connections
    while(true){
    struct acceptedSocket* clientSocket=acceptIncomingConnection(serverSocketFD);
    clientSockets[clientSocketCount++]=*clientSocket;
    recvAndPrintIncomingChatOnSeprateThread(clientSocket->acceptedSocketFD);
    }
    
    shutdown(serverSocketFD,SHUT_RDWR);
    return 0;
}

struct acceptedSocket* acceptIncomingConnection(int serverSocketFD){
    struct sockaddr_in clientAddr;
    int clientAddrSize=sizeof(struct sockaddr_in);
    int clientSocketFD=accept(serverSocketFD,(struct sockaddr*)&clientAddr,(socklen_t *)&clientAddrSize);
     if (clientSocketFD < 0) {
        // Handle error in accepting connection
        return NULL;
    }
    struct acceptedSocket* acceptedSock = (struct acceptedSocket*)malloc(sizeof(struct acceptedSocket));
    acceptedSock->acceptedSocketFD=clientSocketFD;
    acceptedSock->acceptedSuccesfully=(clientSocketFD>0);
    if(!(acceptedSock->acceptedSuccesfully)) acceptedSock->error=clientSocketFD;
    // Add the client to the list of active clients
    cout << "New client connected: " << clientSocketFD << endl;
    return acceptedSock;
}

void *acceptNewConnectionAndRecvAndPrintChat(void* serverSockFDAddr){
     int serverSocketFD = *((int*)serverSockFDAddr);
     while(true){
     struct acceptedSocket* clientSocket=acceptIncomingConnection(serverSocketFD);
     recvAndPrintIncomingChatOnSeprateThread(clientSocket->acceptedSocketFD);
     }

    return NULL;
}



void *recvAndPrintIncomingChat(void* clientSockFDAddr){
    int clientSocketFD = *((int*)clientSockFDAddr);
    char buff[1024];
    while(true){
        int bytesRecv=recv(clientSocketFD,buff,1024,0);
        if(bytesRecv>0){
            buff[bytesRecv]='\0';
            cout<<buff<<endl;
            //broadcasting incming message to all clients 
            sendRecvMsgToOtherClients(clientSocketFD,buff);
        }
        if(bytesRecv==0)break;
    }
    close(clientSocketFD);

    return NULL;
}

void recvAndPrintIncomingChatOnSeprateThread(int clientSocketFD){
    pthread_t id;
    pthread_create(&id,NULL,recvAndPrintIncomingChat,(void*)&clientSocketFD);
}

void sendRecvMsgToOtherClients(int incomingMsgclientSocketFD,char* buffer){
    for(int i=0;i<clientSocketCount;i++){
        if(clientSockets[i].acceptedSocketFD!=incomingMsgclientSocketFD){
            send(clientSockets[i].acceptedSocketFD,buffer,strlen(buffer),0);
        }
    }
}