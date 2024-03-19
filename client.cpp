#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>       // class to represent individual threads of execution / concurrently while having the same address space
#include <unistd.h>
#define MAX_LEN_TEXT 100

using namespace std;

void receiveMessages(int clientSocket)
{
    char buffer[MAX_LEN_TEXT];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            cout << "Server disconnected" << endl;
            close(clientSocket);
            exit(EXIT_FAILURE);
        }
        cout << buffer << endl;
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
        cin.ignore(); // clear the input buffer
    }
    while (PORT < 256 || PORT > 65536);

    // create the client socket
    int clientSocket;
    struct sockaddr_in server;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family       = AF_INET;
    server.sin_port         = htons(PORT);
    server.sin_addr.s_addr  = INADDR_ANY;

    // Connect to sevrer
    int returnNumber = connect(clientSocket, (sockaddr*)&server, sizeof(server));
    if (returnNumber != 0)
    {
        cout << "Failed to connect with the server." << endl;
        exit(EXIT_FAILURE);
    }
    else if (returnNumber == 0)
    {
        cout << "Established connection with the server." << endl;
    }
    thread t(receiveMessages, clientSocket);
    t.detach();

    // Send messages
    char buffer[1024];
    while (true) 
    {
        cin.getline(buffer, sizeof(buffer));
        send(clientSocket, buffer, strlen(buffer), 0);
    }
}