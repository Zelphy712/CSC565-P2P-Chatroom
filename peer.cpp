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
    string password;
};
struct sockaddr_in host_addr;
int sock;
int i = 0;
int is_server = 0;
const char pi_server[] = "10.13.49.205";
string username;
map <string, struct sockaddr_in> room_addr;
struct room_args args;


void *acceptInput(void*);
string contactPiServer(string message);
void *startRoomServer(void* arguments);
void createRoom();
void joinRoom();
void exitRoom();
void sendMessage(string message, struct sockaddr_in receiver);

void sendMessage(string message, struct sockaddr_in receiver){
  	cout<<"Message: "<<message<<endl;
  	socklen_t sock_len = sizeof(receiver);
  	sock = socket(AF_INET, SOCK_DGRAM, 0);
    char server_message[message.size() +1];
    strcpy(server_message, message.c_str());
    sendto(sock, server_message, sizeof(server_message), 0, (const struct sockaddr*)&receiver, sock_len);

}
void *acceptInput(void*){

    sock = 0;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("\n Socket creation error \n");
    }
    while(true){
        string message;
        output_lock.lock();
        cout << "Please input your message: ";
        cin >> message;
        if(message.find("/exit")!=-1){
            exitRoom();
            break;
        }
        output_lock.unlock();
        if(is_server){

            for(map<string, struct sockaddr_in>::iterator it = room_addr.begin(); it!=room_addr.end(); ++it){
                sendMessage(message, it->second);
            }
        }
        else{
            // Send message to the room server
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
        cout<<"Return length: "<<string(buffer).length() << endl;
        cout<<"Return string: "<<buffer<<endl;

    cout<<"---------------------------------"<<endl;
  	return string(buffer);
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
    // Filling server information        int sock = 0, valread;

    socklen_t len_client;
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
            char already_in_use[4];
            if(room_addr.count(string(buffer).substr(1)) > 0){
                strcpy(already_in_use, "1");
                if(sendto(sockfd, already_in_use, sizeof(already_in_use), 0, (const struct sockaddr*)&cliaddr, len_client) < 0){
                    perror("Send failed");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                room_addr.insert(pair<string,struct sockaddr_in>(string(buffer).substr(1),cliaddr));
                const char* message = to_string(room_addr[string(buffer).substr(1)].sin_port).c_str();
                if(sendto(sockfd, message, sizeof(message), 0, (const struct sockaddr*)&cliaddr, len_client) < 0){
                    perror("Send failed");
                    exit(EXIT_FAILURE);
                }
            }
            strcpy(already_in_use, "");
        	}
      	}
    }



    pthread_exit(NULL);

}
void createRoom(){
    pthread_t server_thread;
    pthread_t message_thread;
    cout << "Input a room name: ";
    cin >> args.room_name;
    cout << "Input a password: ";
    cin >> args.password;
    cout << endl;

    contactPiServer("@"+args.room_name);

    pthread_create(&server_thread, NULL, &startRoomServer, (void*) &args);
    pthread_create(&message_thread, NULL, &acceptInput, NULL);
    pthread_join(server_thread, NULL);
    pthread_join(message_thread, NULL);
  	cout<<"STUFF"<<endl;
}

void joinRoom() {
    /*
    -Ping the raspberryPi server to find the ip address of the room it is looking for
    -Once the ip address is known, send a message that contains the peer's ip address, the port number and the room password
    -Confirmation message upon successful join
    */

    string name;
    string password;
    cout << "What room do you want to join? ";
    cin >> name;
    cout << endl;
  	string ipString;
    ipString = contactPiServer("?"+name);
    if(ipString != "-1"){
      	host_addr.sin_port = (unsigned short) stoi(ipString.substr(ipString.find(":")+1));
      	host_addr.sin_addr.s_addr = (unsigned long) stoul(ipString.substr(0,ipString.find(":")),nullptr,10);
    }
  	cout << host_addr.sin_port << endl << host_addr.sin_addr.s_addr << endl;
  	int sock = 0, valread;

    socklen_t len_client;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
    }

    if (connect(sock, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
    {
        printf("\nConnection Failed \n");
    }

    cout << "Enter a username: ";
    cin >> username;

    string message = "@" + username;
    char server_message[message.size() +1];
    strcpy(server_message, message.c_str());
    socklen_t len_host = sizeof(host_addr);
    send(sock , server_message , strlen(server_message) , 0 );
    bzero(server_message, sizeof(server_message));
    recvfrom(sock, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &host_addr, &len_host);
    cout << buffer << endl;


    return;
}

void exitRoom() {
    if(is_server){
        if(room_addr.size() == 0){
            contactPiServer("^"+args.room_name);
          	running = false;
        }else{
          	sendMessage("server|" + to_string(room_addr.begin()->second.sin_addr.s_addr) + ":" + to_string(room_addr.begin()->second.sin_port),room_addr[room_addr.begin()->first]);
          	running = false;
						//send message to first peer in the map that says "server|<comma separated list of ip:port>"
            //the new host sends "host|" to all of the received IPs so they know to change their host location.
        }
    }else{
        //send message to server that says "exit|<username>"
      	sendMessage("exit|" + username,host_addr);
      	exit(0);
    }
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
    int choice;output_lock.unlock();
		host_addr.sin_family = AF_INET;

    while(running){
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




