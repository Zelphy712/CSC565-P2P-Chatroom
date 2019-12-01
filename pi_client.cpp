
// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#define PORT 12000

using namespace std;

//use: ./pi_client <@|?|^><room_name>
//                       ^no space
int main(int argc, char const *argv[]){
    int sock = 0, valread;
    socklen_t len;
    struct sockaddr_in serv_addr;
    const char *hello = argv[1];
    char buffer[22];
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "192.168.0.106", &serv_addr.sin_addr)<=0)
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
    printf("Hello message sent\n");
    valread = recvfrom( sock , &buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *)&serv_addr,&len);
    cout<<"buf: "<<buffer<<endl;
    return 0;
}
