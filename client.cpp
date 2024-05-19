#include<iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <errno.h> 
#include<thread>
using namespace std;

bool Intialize(){
    return true;
}

void sendMessage(int s){
    cout<<"enter your chat name : "<<endl;
    string name;
    getline(cin,name);
    string message;

    while(1){
        getline(cin , message);
        string msg = name + " : " + message ;
        int bytesent = send(s , msg.c_str() , msg.size() , 0);
        if(bytesent == -1){
            cout<<"error sending message"<<endl;
            break;
        }
        if(message == "quit"){
            cout<<"stopping the application"<<endl;
            break;
        }
    }
    close(s);
}

void recieveMessage(int s){
    char buffer[4096];
    int recvlength ;
    string msg = "";
    while(1){
        recvlength = recv(s,buffer,sizeof(buffer),0);
        if(recvlength <= 0){
            cout<<"disconnected from server"<<endl;
            break;
        }
        else {
            msg = string(buffer , recvlength);
            cout<<msg<<endl;
        }
    }
    close(s);
}

int main(){
    if(!Intialize()){
        cout<<"intialization failed "<<endl;
    }
    cout<<"Client Program"<<endl;

    int s = socket(AF_INET , SOCK_STREAM , 0);
    
    if(s == -1){
        cout<<"socket creation failed "<<endl;
        return 1;
    }
    int port = 12345;
    string serveraddress = "127.0.0.1";

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET , serveraddress.c_str() , &(serveraddr.sin_addr));

    if(connect(s,(struct sockaddr *)&serveraddr , sizeof(serveraddr)) == -1){
        cout<<"not able to connect to server "<<endl;
        close(s);
        return 1;
    }

    cout<<"Successfully connected to server "<<endl;

    thread senderThread(sendMessage , s);
    thread reciever(recieveMessage , s);

    senderThread.join();
    reciever.join();

    return 0;
}