#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <ostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#define MAX_LEN_NAME 30
#define MAX_LEN_TEXT 1000
#define MAX_CLIENTS 3

using namespace std;

mutex mtx; // mutual exclusion: protects data from concurrent access
// blocks one thread from accessing data a different thread is accessing at the same time
vector<int> clientSockets;  // vector, type, name
// an arrays of ints, dinamically changeable
// clientSockets.push_back(value) <- adds a value at the last index place in the vector
// vector elements are placed in contiguous storage so that they can be accessed and traversed using iterators

void broadcastMessage(int senderSocket, const string& message)
{
    lock_guard<mutex> lock(mtx); // lock_guard is a class thats acts as a wrapper for mutexes
    // to prevent two threads vom locking each other
    for (int clientSocket : clientSockets)
    {
        if (clientSocket != senderSocket)
        {
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }
}

void clientHandler(int clientSocket)
{
    char buffer[MAX_LEN_TEXT];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer)); // sets the buffer to all zero
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            lock_guard<mutex> lock(mtx);
            auto i = find(clientSockets.begin(), clientSockets.end(), clientSocket);
            if (i != clientSockets.end())
            {
                clientSockets.erase(i);
            }
            close(clientSocket);
            break;
        }
        string message = "Client " + to_string(clientSocket) + ": " + buffer;
        broadcastMessage(clientSocket, message);
    }
}

int main()
{
    // getting user input for a port number
    int PORT;
    do
    {
        cout << "Port number: " << endl;
        cin >> PORT;
    }
    while (PORT < 256 || PORT > 65536);

    // initialize the server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) 
    {
        cout << "Failed to set up the server socket.\n" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "\n1. Socket opened successfully." << endl;
    }

    // initialize the environment for the sockaddr structure
    struct sockaddr_in server, client;
    int clientSocket;
    server.sin_family           = AF_INET;      // for TCP and UDP connections
    server.sin_port             = htons(PORT);  // htons/hotnl/ntohl/ntohs: host to network byte order / converts an unsigned short integer etc.
    server.sin_addr.s_addr      = INADDR_ANY;   // accepts connections from any internet address
    bzero(&server.sin_zero,0);                  // same as above

    // bind the socket (with it's IP) to the local port (like register a telephone number)
    // -> int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
    //      sockfd = socket filte decriptor
    //      sockaddr = adress to what the socket is bound
    //      addrlen = size of the address
    int returnNumber = bind(serverSocket, (sockaddr*)&server, sizeof(server));
    if (returnNumber != 0)
    {
        cout << "Failed to bind to local port.\n" << endl;
        exit(EXIT_FAILURE);
    }
    else if (returnNumber == 0)
    {
        cout << "2. Socket successfully bound to local port." << endl;
    }

    // listen for incoming data (requests from clients)
    // -> int listen(int sockfd, int backlog);
    returnNumber = listen(serverSocket, MAX_CLIENTS); 
    if (returnNumber != 0)
    {
        cout << "Failed to start listening to local port.\n" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "3. Chat server established..." << endl;

    unsigned int length = sizeof(client);
    while (true) 
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&client, &length);
        cout << "Client connected: " << clientSocket << endl;
        clientSockets.push_back(clientSocket);
        thread t(clientHandler, clientSocket);
        t.detach();
    }

    // accept - establish connection and keep waiting for new requests (in a loop)
    // -> int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    // socket -- opt.: client address information -- opt.: size of the address structure
    // if successful, accept() returns a value of type SOCKET that is 
    // a descriptor for the new socket that is connected to the client.
    // this new socket is the servicing socket
    // the original one keeps listening for more incoming requests

    close(serverSocket);
    return 0;
}