/* The server should receive only the name of a room preceded by a single character
*  and will return either a struct sockaddr_in or an integer representing a success
*  or failure of the requested action.
*  An exclamation point (!) should be used to delete a room entry
*  An at sign (@) should be used to insert a room entry
*  A question mark (?) should be used to get the struct from the room name
*
*  ex. @CSC565 will create a room entry and return a 0 for success, or a -1 for failure
*  ex. ?CSC565 will return the structure containing the IP and port information for the floating server
*  ex. !CSC565 will delete the room entry and return a 0 for success, or a -1 for failure
*
*  Any unrecognized first character will return a -1
*/

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


// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    const char* hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;

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

    int returnStatus;
    struct sockaddr_in floatingServer;
    buffer[n] = '\0';
    if(buffer[0] == "@"){
        if((returnStatus = addRoom(buffer.substr(1),cliaddr)) == 0){
            sendto(sockfd, &returnStatus, 1, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }else if(returnStatus == -1){
            sendto(sockfd, &returnStatus, 1, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
    }else if(buffer[0] == "?"){
         if((returnStatus = getRoom(buffer.substr(1),&floatingServer)) == 0){
            sendto(sockfd, &floatingServer, sizeof(floatingServer), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }else if(returnStatus == -1){
            sendto(sockfd, &returnStatus, 1, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
    }else if(buffer[0] == "!")){
        if((returnStatus = removeRoom(buffer.substr(1),cliaddr)) == 0){
            sendto(sockfd, &returnStatus, 1, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }else if(returnStatus == -1){
            sendto(sockfd, &returnStatus, 1, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
    }else{
        returnStatus = -1;
        sendto(sockfd, &returnStatus, 1, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    }



    printf("Client : %s\n", buffer);
    sendto(sockfd, hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    printf("Hello message sent.\n");

    return 0;
}

int addRoom(string roomName, struct sockaddr_in addr){
    if(roomIp.count(roomName) > 0){
        return -1;//the room already exists
    }else{
        roomIp.insert(pair<string,struct sockaddr_in>(roomName,addr));
    }
    return 0;//insert successful
}

int removeRoom(string roomName){
    if(roomIp.count(roomName)<1){
        return -1;//room did not exist
    }else{
        roomIp.erase(roomName);
    }
    return 0;//removal successful
}

int getRoom(string roomName,struct sockaddr_in * outputStruct){
    if(roomIp.count(roomName) > 0){
        outputStruct = roomIp[roomName];
        return 0;
    }else{
        return -1;//room does not exist
    }
}




