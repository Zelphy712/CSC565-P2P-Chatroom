#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 12000
#define MAXLINE 2048

struct room_creation_args {
    char room_name[MAXLINE];
    char password[MAXLINE];
};

struct message {
    string message;

};

using namespace std;
const char pi_server[] = "192.168.1.4";
struct sockaddr_in room_addr;

void *createRoom(void* arguments){
    struct room_creation_args *room = (struct room_creation_args *)arguments;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char creation_message[MAXLINE];
    strcpy(creation_message, "@");
    strcat(creation_message, room -> room_name);
    cout << creation_message << endl;
    cout << room -> password << endl;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, pi_server, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return NULL;
    }
    send(sock , creation_message , strlen(creation_message) , 0 );
    cout << "Creation message sent" << endl;
    valread = read( sock , buffer, 1024);
        struct sockaddr_in servaddr, cliaddr;

    int sockfd;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n;
    socklen_t len;
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        printf("test\n");
    pthread_exit(NULL);
    return NULL;
}

void joinRoom(string room_name, string password) {
    //***********************************************
    //Connect to pi_server
    //***********************************************
    int sock = 0, valread;
    socklen_t len;
    struct sockaddr_in serv_addr;
    const char *hello = (char*) ("?" + room_name).c_str();
    char buffer[22];
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    // Change hardcoded IP to the static server
    if(inet_pton(AF_INET, "10.13.43.44", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    send(sock , hello , strlen(hello) , 0 );
    valread = recvfrom( sock , &buffer, 22, MSG_WAITALL, (struct sockaddr *)&serv_addr,&len);
    cout<<"buf: "<<buffer<<endl;

    //***********************************************
    // Request access to floating chat server
    //***********************************************

    string ip = toString(buffer).substr(0,(toString(buffer).length()-toString(buffer).find(":")));
    int port = stoi(toString(buffer).substr((toString(buffer).length()-toString(buffer).find(":"))+1));





    /*
    -Ping the raspberryPi server to find the ip address of the room it is looking for
    -Once the ip address is known, send a message that contains the peer's ip address, the port number and the room password
    -Confirmation message upon successful join
    */




    return;
}

void exitRoom() {
    /*
    -If peer is the current room server, send an exit message to the raspberryPi server
        +Send the list of peers to another peer, making them the new chat room server; if there are no other peers, toss the room
        +Display message to all peers notifying the server change
    -Send goodbye message to all other peers
    -Close connection
    */
    return;
}

void sendMessage(string message) {
    /*
    -Send message to room server, which will be sent to other peers
    */
    return;
}

void broadcastMessage(string message){
    /*
    -Broadcast message to all peers
    */
    return;
}

int main(){
    pthread_t server_thread;
    struct room_creation_args args;
    while(true){
        cout << "Please input a room name: ";
        cin >> args.room_name;
        cout << "Please input a password: ";
        cin >> args.password;
        cout << endl;
        pthread_create(&server_thread, NULL, &createRoom, (void*) &args);
    }
    pthread_join(server_thread, NULL);

    return 0;

}




