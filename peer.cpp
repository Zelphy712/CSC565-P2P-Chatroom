#include <pthread.h>
#include <mutex>
#include <map>
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



bool running = true;
using namespace std;
mutex output_lock;
struct room_args {
    string room_name;
};
struct sockaddr_in host_addr;
int sock;
int sockfd;
int is_server = 0;
const char pi_server[] = "10.13.49.208";
string username;
map <string, struct sockaddr_in> room_addr;
struct room_args args;


void *acceptInput(void*);
void *clientListen(void*);
string contactPiServer(string message);
void *startRoomServer(void* arguments);
void createRoom();
void joinRoom();
void exitRoom();
void sendMessage(string message, struct sockaddr_in receiver);

/*
/****************************************************************
/ Send Message function
/
/ Input: message (string)
/ Input: receiver (sockaddr_in)
/
/ Send a message to the address specified by receiver
/
/
******************************************************************
*/

void sendMessage(string message, struct sockaddr_in receiver){
  	socklen_t sock_len = sizeof(receiver);
    char server_message[message.size() +1];
    strcpy(server_message, message.c_str());
    sendto(sockfd, server_message, string(server_message).length() + 1, 0, (const struct sockaddr*)&receiver, sock_len);

}

/*
/****************************************************************
/ Client Listen Thread function
/
/ Input: (void*)
/
/ While running, listen for messages from the host of the room,
/ and act according to the message
/
******************************************************************
*/

void *clientListen(void*){
    char *buffer= (char *) malloc(MAXLINE*sizeof(char));
    socklen_t len_host = sizeof(host_addr);
    while(running){
    int t = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_DONTWAIT, ( struct sockaddr *) &host_addr, &len_host);
    if(t > 0){
        cout << buffer << endl;
        if(string(buffer).find("server|")!=-1){
            cout<<"host left"<<endl;
            host_addr.sin_port = htons(12001);
            inet_pton(AF_INET, string(buffer).substr(0,string(buffer).find(":")).c_str(), &host_addr.sin_addr);
            sendMessage("@"+username,host_addr);
        }else if(buffer[0]=='@'){
            is_server = 1;
            pthread_t server_thread;
            pthread_t message_thread;
            contactPiServer("@"+args.room_name);
            pthread_create(&server_thread, NULL, &startRoomServer, (void*) &args);
            pthread_create(&message_thread, NULL, &acceptInput, NULL);
            pthread_join(server_thread, NULL);
            pthread_join(message_thread, NULL);

        }
    }
    strcpy(buffer, "");
    }


    pthread_exit(NULL);
}

/*
/****************************************************************
/ Accept Input Thread function
/
/ Input: (void*)
/
/ While the application is running, listen for user input and
/ act according to what the input is
/
******************************************************************
*/

void *acceptInput(void*){
    while(true){
        string message;
        output_lock.lock();
        getline(cin, message);
        output_lock.unlock();

        // Once a user is in a chat room, the exit command is "/exit"
        if(message.find("/exit")!=-1){
            exitRoom();
            break;
        }

        if(is_server){
            // Send message to all peers in the room
            message = username + ": " + message;
            for(map<string, struct sockaddr_in>::iterator it = room_addr.begin(); it!=room_addr.end(); ++it){
                sendMessage(message, it->second);
            }
        }
        else{
            // Send message to the room server
            message = "#"  + username + ": " + message;
            sendMessage(message, host_addr);
        }


    }
    pthread_exit(NULL);
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

string contactPiServer(string message) {
    sock = 0;
    socklen_t len_client;
    struct sockaddr_in serv_addr;
    char server_message[message.size() +1];
    strcpy(server_message, message.c_str());
    char *buffer= (char *) malloc(MAXLINE*sizeof(char));
  	bzero(buffer, sizeof(buffer));
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
    bzero(server_message, sizeof(server_message));
    recvfrom(sock, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &serv_addr, &len_server);
  	return string(buffer);
    close(sock);
}

/*
/****************************************************************
/ Start Room Server Thread function
/
/ Input: Room Arguments (void*)
/
/ Start a room server to begin listening for new clients
/
/
******************************************************************
*/

void *startRoomServer(void* arguments){

    struct room_args *room = (struct room_args *)arguments;
    struct sockaddr_in servaddr, cliaddr;

    char buffer[1024] = {0};
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    socklen_t len_client = sizeof(cliaddr);
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12001);


    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    is_server = 1;
    int n;
    while(running){
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_DONTWAIT, ( struct sockaddr *) &cliaddr, &len_client);

      	if(n > 0){
         	if(string(buffer).at(0) == '@'){
                room_addr.insert(pair<string,struct sockaddr_in>(string(buffer).substr(1),cliaddr));
                const char* message = "\nHello from the host!\n";
                if(sendto(sockfd, message, string(message).length()+1, 0, (const struct sockaddr*)&cliaddr, len_client) < 0){
                    perror("Send failed");
                    exit(EXIT_FAILURE);
                }
        	}
        else if(string(buffer).at(0) == '#') {
            for(map<string, struct sockaddr_in>::iterator it = room_addr.begin(); it!=room_addr.end(); ++it){
                sendMessage(string(buffer).substr(1), it->second);
            }
            cout << string(buffer).substr(1) << endl;
        }

      	}
    }



    pthread_exit(NULL);

}

/*
/****************************************************************
/ Create Room function
/
/ Ask the user for a room name, and then send a room creation
/ message to the pi server for that room name
/
******************************************************************
*/
void createRoom(){
    string room_creation; // Variable to determine if room creation was successful
    string room_name; // Create a temporary variable for the name in case the name is already in the pi server
    pthread_t server_thread;
    pthread_t message_thread;

    cout << "Make a username: ";
    getline(cin, username);
    cout << "Input a room name: ";
    getline(cin, room_name);


    room_creation = contactPiServer("@"+room_name);

    if(room_creation == "-1"){
        while(true){
            cout << "Make a username: ";
            getline(cin, username);
            cout << "Input a room name: ";
            getline(cin, room_name);
        }
    }

    args.room_name = room_name; // If the room name is available, set the room name
    pthread_create(&server_thread, NULL, &startRoomServer, (void*) &args);
    pthread_create(&message_thread, NULL, &acceptInput, NULL);
    pthread_join(server_thread, NULL);
    pthread_join(message_thread, NULL);

}


/*
/****************************************************************
/ Join Room function
/
/ Ask the pi server for a room's host information; if the room
/ exists, the host information will be stored and the client will
/ send the host its user name.
/
******************************************************************
*/
void joinRoom() {
    pthread_t listen_thread;
    pthread_t input_thread;

    string name;
    string password;
    cout << "What room do you want to join? ";
    getline(cin, name);
    cout << endl;
  	string ipString;

    ipString = contactPiServer("?"+name); // Ask the pi server if this room exists.
    if(ipString != "-1"){
      	host_addr.sin_port = htons(12001);
      	inet_pton(AF_INET, ipString.substr(0,ipString.find(":")).c_str(), &host_addr.sin_addr);
    }else{
        while(true){
            cout<<"Room does not exist."<<endl;
            cout << "What room do you want to join? ";
            getline(cin, name);
            cout << endl;
            string ipString;
            ipString = contactPiServer("?"+name);
            // If the room exists, get the host information and store it.
            if(ipString != "-1"){
                host_addr.sin_port = htons(12001);
                inet_pton(AF_INET, ipString.substr(0,ipString.find(":")).c_str(), &host_addr.sin_addr);
                break;
            }
        }
    }
    char buffer[1024] = {0};
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
    }
    cout << "Enter a username: ";output_lock.unlock();
    getline(cin, username);

    string message = "@" + username;
    char server_message[message.size() +1];
    strcpy(server_message, message.c_str());
    socklen_t len_host = sizeof(host_addr);
    sendto(sockfd , server_message , strlen(server_message), 0, (struct sockaddr*)&host_addr, len_host);
    pthread_create(&listen_thread, NULL, &clientListen, NULL);
    pthread_create(&input_thread, NULL, &acceptInput, NULL);

    pthread_join(listen_thread, NULL);
    pthread_join(input_thread, NULL);



    return;
}


/*
/****************************************************************
/ Exit Room function
/
/ Send an exit message to either the host or the pi server and
/ end the program.
/
******************************************************************
*/
void exitRoom() {
    if(is_server){
        if(room_addr.size() == 0){
            // If there are no other peers in the room, send a delete message to the pi server.
            contactPiServer("^"+args.room_name);
          	running = false;
        }else{
            //send message to first peer in the map that says "server|<comma separated list of ip:port>"
          	sendMessage("server|" + to_string(room_addr.begin()->second.sin_addr.s_addr) + ":" + to_string(room_addr.begin()->second.sin_port),room_addr[room_addr.begin()->first]);
          	running = false;
            //the new host sends "host|" to all of the received IPs so they know to change their host location.
        }
    }else{
      	sendMessage("exit|" + username,host_addr); //send message to server that says "exit|<username>"
      	running = false;
    }

    return;
}


int main(){
    string choice;
    host_addr.sin_family = AF_INET;

    while(running){
        cout << "Choose an option:" << endl << "1. Create a room" << endl << "2. Join a room" << endl;
        getline(cin, choice);
        cout << endl;
        if(choice == "1"){
            createRoom();
        }
        else if(choice == "2"){
            joinRoom();
        }
        else{
            //do nothing
        }


    }


    return 0;

}




