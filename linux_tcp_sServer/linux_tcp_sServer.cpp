#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <ctime>
#include <fstream>



int main(){
    std::ofstream fout;
    //Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if(listening == -1){
        std::cerr << "Can't create socket\n";
        return -1;
    }

    //Bind socket to ip/port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54001);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr); //internet pointer to string to a number
    
    if(bind(listening,(sockaddr*)&hint, sizeof(hint)) == -1){
        std::cerr << "Can't bind to IP/port\n";
        return -2;
    }

    //Mark the socket for listening in
    if(listen(listening, SOMAXCONN) == -1){
        std::cerr << "Can't listen\n";
        return -3;
    }

    //Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char service[NI_MAXHOST];

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if(clientSocket == -1){
        std::cerr << "Problen with client connecting";
        return -4;
    }

    //Close the listening socket
    close(listening);

    memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXHOST);

    int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXHOST, 0);
    
    if(result){
        std::cout << host << " connected on " << service << std::endl;
    }else{
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        std::cout << host << " connected on " << ntohs(client.sin_port) << std::endl;
    }
    //While receving - display message, echo message, log message
    char buff[4096];
    while(true){

        //Clear the buffer
        memset(buff, 0, 4096);
        //Wait for a message
        int bytesRecv = recv(clientSocket, buff, 4096, 0);
        if(bytesRecv == -1){
            std::cerr << "There was a connection issue\n";
            break;
        }
        if(bytesRecv == 0){
            std::cout << "The client disconnected" << std::endl;
            break;
        }
        //Log message
        fout.open("log.txt", std::ios::app);
        time_t now = time(0);
        fout << ctime(&now) << " " << ntohs(client.sin_port) << " " <<std::string(buff, 0, bytesRecv) << std::endl;
        //std::cout << "Received: " << std::string(buff, 0, bytesRecv) << std::endl;
        fout.close();
        

        //Resend message
        send(clientSocket, buff, bytesRecv +1, 0);
    }
    //Close socket
    close(clientSocket);


    return  0;
}
