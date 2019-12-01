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

struct room_args {
    char room_name[MAXLINE];
    char password[MAXLINE];
};

using namespace std;
const char pi_server[] = "10.13.49.208";
char message_to_pi_server[MAXLINE];
struct sockaddr_in room_addr;

void *startRoomServer(void* arguments){
    struct room_args *room = (struct room_args *)arguments;

    int sock = 0, valread;

    socklen_t len_client;
    struct sockaddr_in serv_addr;
    strcpy(message_to_pi_server, "@");
    strcat(message_to_pi_server, room -> room_name);
    cout << message_to_pi_server << endl;
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
    socklen_t len_server = sizeof(serv_addr);
    send(sock , message_to_pi_server , strlen(message_to_pi_server) , 0 );
    cout << "Creation message sent" << endl;
    bzero(message_to_pi_server, sizeof(message_to_pi_server));
    recvfrom(sock, (char *)buffer, sizeof(buffer), MSG_WAITALL, ( struct sockaddr *) &serv_addr, &len_server);
    struct sockaddr_in servaddr, cliaddr;
    cout << buffer << endl;

    close(sock);
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
    servaddr.sin_port = htons(12001);


    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n;
    cout << "Room server started" << endl;
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len_server);
        printf("test\n");
    pthread_exit(NULL);

}
void createRoom(){
    pthread_t server_thread;
    struct room_args args;
    cout << "Input a room name: ";
    cin >> args.room_name;
    cout << "Input a password: ";
    cin >> args.password;
    cout << endl;

    pthread_create(&server_thread, NULL, &startRoomServer, (void*) &args);
    pthread_join(server_thread, NULL);
}

void joinRoom(string room_name, string password) {
    /*
    -Ping the raspberryPi server to find the ip address of the room it is looking for
    -Once the ip address is known, send a message that contains the peer's ip address, the port number and the room password
    -Confirmation message upon successful join
    */

    char name[MAXLINE];
    cout << "What room do you want to join? ";
    cin >> name;
    cout << endl;

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
    int choice;

    while(true){
        cout << "Choose an option:" << endl << "1. Create a room" << endl << "2. Join a room" << endl;
        cin >> choice;
        cout << endl;
        if(choice == 1){
            createRoom();
        }


    }


    return 0;

}




