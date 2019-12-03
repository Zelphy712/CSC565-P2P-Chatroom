/* The server should receive only the name of a room preceded by a single character
*  and will return either a struct sockaddr_in or an integer representing a success
*  or failure of the requested action.
*  An caret (^) should be used to delete a room entry
*  An at sign (@) should be used to insert a room entry
*  A question mark (?) should be used to get the struct from the room name
*
*  ex. @CSC565 will create a room entry and return a 0 for success, or a -1 for failure
*  ex. ?CSC565 will return the structure containing the IP and port information for the floating server
*  ex. ^CSC565 will delete the room entry and return a 0 for success, or a -1 for failure
*
*  Any unrecognized first character will return a -1
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <map>

#define PORT 12000
#define MAXLINE 2048

using namespace std;

map<string,struct sockaddr_in> roomIp;

int addRoom(string roomName, struct sockaddr_in addr);
int removeRoom(string roomName);
string getRoom(string roomName);

int main(){


    int returnStatus;
    char* returnStr = (char*)malloc(100000);
    struct sockaddr_in floatingServer;

    int sockfd;
    char *buffer;
    const char* hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;

    buffer = (char*) malloc(sizeof(char)*MAXLINE);

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
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
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int n;
    socklen_t serv_len = sizeof(servaddr);
    socklen_t cli_len = sizeof(cliaddr);
    while(true){
        cout<<"waiting"<<endl;
        if(recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &cli_len) < 0){
                cout<<"Receive failed"<<endl;
                break;
        }
        cout<<"buf:"<<buffer<<": end"<<endl;

        //buffer[n] = '\0';


        if(string(buffer).at(0) == '@'){
            returnStatus = addRoom(string(buffer).substr(1),cliaddr);
            returnStr = (char*) to_string(returnStatus).c_str();
            if(returnStatus == 0){
                if(sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len) < 0){
                  perror("send failed");
                  exit(EXIT_FAILURE);
                };
            }
            else if(returnStatus == -1){
                sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len);
            }
            //cout<<"return status: "<<returnStatus<<endl;
            //cout<<"Map size: "<<roomIp.size()<<endl;
            //cout<<"IP address: "<<roomIp[string(buffer).substr(1)].sin_addr.s_addr<<endl;
        }
        else if(string(buffer).at(0) == '?'){
            string returned = getRoom(string(buffer).substr(1));
            cout<<"Returned: "<<returned<<endl;
            returnStr = (char*) returned.c_str();
            cout << "return status: " << returnStr << endl;
            if( returnStr != "-1"){

                sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len);
            }
            else{

                sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len);
            }
        }
        else if(string(buffer).at(0) == '^'){
            returnStatus = removeRoom(string(buffer).substr(1));
            returnStr = (char*) to_string(returnStatus).c_str();
            if(returnStatus == 0){
                sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len);
            }
            else if(returnStatus == -1){
                sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len);
            }
        }
        else if(string(buffer).at(0) == '%'){
            break;
        }else{
            returnStatus = -1;
            returnStr = (char*) to_string(returnStatus).c_str();
            sendto(sockfd, returnStr, sizeof(returnStr), 0, (const struct sockaddr *) &cliaddr, cli_len);
        }


        //cout<<"Client: "<<buffer<<endl;
        bzero(buffer, sizeof(buffer));
        sendto(sockfd, hello, strlen(hello), 0, (const struct sockaddr *) &cliaddr, cli_len);
    }
    free(buffer);
    return 0;
}

int addRoom(string roomName, struct sockaddr_in addr)
{
    if(roomIp.count(roomName) > 0){
        return -1;//the room already exists
    }
    else{
        roomIp.insert(pair<string,struct sockaddr_in>(roomName,addr));
    }
    return 0;//insert successful
}

int removeRoom(string roomName)
{
    if(roomIp.count(roomName)<1){
        return -1;//room did not exist
    }
    else{
        roomIp.erase(roomName);
    }
    return 0;//removal successful
}

string getRoom(string roomName)
{
    string output;
    cout<<roomName<<endl;
    if(roomIp.count(roomName) > 0){
        output = to_string(roomIp[roomName].sin_addr.s_addr) + ":" + to_string(roomIp[roomName].sin_port);
        cout<<"Output: "<<output<<endl;
        return output;
    }
    else{
        return "-1";//room does not exist
    }
}



