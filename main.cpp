#include<iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <errno.h> 
#include<thread>
#include<vector>
#include "threadPoolTut.cpp"
using namespace std;

// Intialize the socket


bool Intialize(){
    return true;
}

void InteractWithClient(int clientSocket , vector<int>& clients , mutex& clientMutex){
    cout<<"client connected "<<endl;
    char buffer[4096];
    while(1){
        int bytesrecvd = recv(clientSocket,buffer,sizeof(buffer),0);
        if(bytesrecvd <= 0){
            cout<<"client disconnected"<<endl;
            break;
        }
        string message(buffer , bytesrecvd);
        cout<<"message from client : "<<message<<endl;
        unique_lock<mutex> lock(clientMutex);
        for(auto client : clients){
            if(client != clientSocket){
                send(client , message.c_str() , message.size() , 0);
            }
        }
        lock.unlock();
    }
    unique_lock<mutex> lock(clientMutex);
    auto it  = find(clients.begin() , clients.end() , clientSocket);
    if(it != clients.end()){
        clients.erase(it);
    }
    lock.unlock();
    close(clientSocket);
}

int main(){
    if(!Intialize()){
        cout<<"intialization failed "<<endl;
    }
    cout<<"Server Program"<<endl;

    int listenSocket = socket(AF_INET,SOCK_STREAM,0);
    if(listenSocket == -1){
        cout<<"socket creation failed "<<endl;
        return 1;
    }

    // create address structure
    int port = 12345;
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    // convert ip address (0.0.0.0) put it inside sin_family in binary form
    if(inet_pton(AF_INET , "0.0.0.0" , &serveraddr.sin_addr) != 1){
        cout<<"setting address structure failed"<<endl;
        close(listenSocket);
        return 1;
    }

    // bind
    int bindResult = ::bind(listenSocket , (struct sockaddr *)&serveraddr , static_cast<int>(sizeof(serveraddr)));
        if(bindResult == -1){
            cout<<"bind error"<<strerror(errno)<<endl;
            close(listenSocket);
            return 1;
        }  

    // Listen
    if(listen(listenSocket , SOMAXCONN) == -1){
        cout<<"Listen Error"<<endl;
        close(listenSocket);
        return 1;
    }
    cout<<"server has started listening on port : "<<port<<endl;
    vector<int> clients;
    ThreadPool pool(8);
    mutex clientMutex;
    while(1){
        // accept
        int clientSocket = accept(listenSocket,nullptr,nullptr);
        if(clientSocket == -1){
            cout<<"invalid client socket"<<endl;
        }
        unique_lock<mutex> lock(clientMutex);
        clients.push_back(clientSocket);
        // recieve
        // std::thread t1(InteractWithClient , clientSocket , std::ref(clients));
        // t1.detach();
        pool.ExecuteTask([clientSocket, &clients, &clientMutex] {
            InteractWithClient(clientSocket, clients, clientMutex);
        });
        lock.unlock();
    }
    
    close(listenSocket);

    return 0;
}