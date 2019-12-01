#include <pthread.h>
#include <mutex>
#include <cstring>
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



using namespace std;
mutex output_lock;
struct room_args {
    string room_name;
    string password;
};
int is_server = 0;
const char pi_server[] = "10.13.49.208";
struct sockaddr_in room_addr[100];

void *sendMessage(void*){
    while(true){
        string message;
        output_lock.lock();
        cout << "Please input your message: ";
        cin >> message;
        output_lock.unlock();

    }
}
/*
/****************************************************************
/ Contact Pi server function
/
/ Input: message (string)
/
/ Create a socket for communication with the raspberry pi server
/ Send a message to the server
/
******************************************************************
*/

void contactPiServer(string message) {
    int sock = 0, valread;

    socklen_t len_client;
    struct sockaddr_in serv_addr;
    char server_message[message.size() +1];
    strcpy(server_message, message.c_str());
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, pi_server, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
    }
    socklen_t len_server = sizeof(serv_addr);
    send(sock , server_message , strlen(server_message) , 0 );
    cout << "Message sent" << endl;
    bzero(server_message, sizeof(server_message));
    recvfrom(sock, (char *)buffer, sizeof(buffer), MSG_WAITALL, ( struct sockaddr *) &serv_addr, &len_server);
    cout << buffer << endl;

    close(sock);
}
void *startRoomServer(void* arguments){
    struct room_args *room = (struct room_args *)arguments;
    struct sockaddr_in servaddr, cliaddr;

    int sockfd;
    char buffer[1024] = {0};
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    socklen_t cli_len = sizeof(cliaddr);
    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12001);


    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n;
    output_lock.lock();
    cout << "Room: " << room->room_name << endl;
    output_lock.unlock();
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &cli_len);
    is_server = 1;
    pthread_exit(NULL);

}
void createRoom(){
    pthread_t server_thread;
    pthread_t message_thread;
    struct room_args args;
    cout << "Input a room name: ";
    cin >> args.room_name;
    cout << "Input a password: ";
    cin >> args.password;
    cout << endl;

    contactPiServer("@"+args.room_name);

    pthread_create(&server_thread, NULL, &startRoomServer, (void*) &args);
    pthread_create(&message_thread, NULL, &sendMessage, NULL);
    pthread_join(server_thread, NULL);
    pthread_join(message_thread, NULL);
}

void joinRoom() {
    /*
    -Ping the raspberryPi server to find the ip address of the room it is looking for
    -Once the ip address is known, send a message that contains the peer's ip address, the port number and the room password
    -Confirmation message upon successful join
    */

    string name;
    cout << "What room do you want to join? ";
    cin >> name;
    cout << endl;

    contactPiServer("?"+name);

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


int main(){
    int choice;

    while(true){
        cout << "Choose an option:" << endl << "1. Create a room" << endl << "2. Join a room" << endl;
        cin >> choice;
        cout << endl;
        if(choice == 1){
            createRoom();
        }
        else if(choice == 2){
            joinRoom();
        }


    }


    return 0;

}




