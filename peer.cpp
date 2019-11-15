void createRoom(string room_name, string password){
    /*
    -Send the room name, and the ip address of the peer that created it
    -Start server function
    */
    return;
}

void joinRoom(string room_name, string password) {
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




