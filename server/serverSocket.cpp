#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include <unistd.h>
#include<pthread.h>
using namespace std;

#define MAX_CLIENTS 10

int port=2000;

struct acceptedSocket{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    int acceptedSuccesfully;
};
void recvAndPrintIncomingChatOnSeprateThread(int i);
struct acceptedSocket* acceptIncomingConnection(int serverSocketFD);
void recvAndPrintIncomingChat(int clientSocketIndex);
void broadcastClients(int incomingMsgclientSocketFD,char*buffer);

struct acceptedSocket* clientSockets[MAX_CLIENTS];
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
        if(clientSocket==NULL)continue;
        for(int i=0;i<MAX_CLIENTS;i++){
            if(clientSockets[i]==NULL){
                clientSockets[i]=clientSocket;
                clientSocketCount++;
                cout<<"Number of live connections : "<<clientSocketCount<<endl;
                recvAndPrintIncomingChatOnSeprateThread(i);
                break;
            }
        }
    }
    
    shutdown(serverSocketFD,SHUT_RDWR);
    return 0;
}


struct acceptedSocket* acceptIncomingConnection(int serverSocketFD){
    struct sockaddr_in clientAddr;
    int clientAddrSize=sizeof(struct sockaddr_in);
    int clientSocketFD=accept(serverSocketFD,(struct sockaddr*)&clientAddr,(socklen_t *)&clientAddrSize);
    if (clientSocketFD < 0) {
        perror("Client accept failed");
        return NULL;
    }

    if (clientSocketCount >= MAX_CLIENTS) {
        // Reject the client if max clients reached
        cout << "Rejecting client: Maximum clients reached." << endl;
        close(clientSocketFD); // Close the socket immediately
        return NULL;
    }

    struct acceptedSocket* acceptedSock = (struct acceptedSocket*)malloc(sizeof(struct acceptedSocket));
    acceptedSock->acceptedSocketFD=clientSocketFD;
    acceptedSock->acceptedSuccesfully=(clientSocketFD>0);
    if(!(acceptedSock->acceptedSuccesfully)) acceptedSock->error=clientSocketFD;
    cout << "New client connected: " << clientSocketFD << endl;
    return acceptedSock;
}



void *recvAndPrintIncomingChat(void* clientSocketIndex){ 
    int pos = *(int*)clientSocketIndex;
    struct acceptedSocket* clientSocket =clientSockets[pos];
    char buff[1024];
    while(true){
        int bytesRecv=recv(clientSocket->acceptedSocketFD,buff,1024,0);
        if(bytesRecv>0){
            buff[bytesRecv]='\0';
            cout<<buff<<endl;
            //broadcasting incming message to all clients 
            broadcastClients(clientSocket->acceptedSocketFD,buff);
        }
        if(bytesRecv==0){
            cout<<"client "<<(clientSocket->acceptedSocketFD)<<" closed connection"<<endl;
            break;
        }
    }
    close(clientSocket->acceptedSocketFD);
    free(clientSocket);
    clientSockets[pos]=NULL;
    clientSocketCount--;
    cout<<"Number of live connections : "<<clientSocketCount<<endl;

    return NULL;
}

void recvAndPrintIncomingChatOnSeprateThread(int i){
    pthread_t id;
    pthread_create(&id,NULL,recvAndPrintIncomingChat,(void*)&i);
}

void broadcastClients(int incomingMsgClientSocketFD,char* buffer){
    for(int i=0;i<MAX_CLIENTS;i++){
        if(clientSockets[i]!=NULL && clientSockets[i]->acceptedSocketFD  != incomingMsgClientSocketFD ){
            send(clientSockets[i]->acceptedSocketFD,buffer,strlen(buffer),0);
        }
    }
}