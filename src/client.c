/******************************************************************************

Welcome to GDB Online.
  GDB online is an online compiler and debugger tool for C, C++, Python, PHP, Ruby, 
  C#, OCaml, VB, Perl, Swift, Prolog, Javascript, Pascal, COBOL, HTML, CSS, JS
  Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX 10

typedef struct Person {
    char sName[MAX];
    char sSurname[MAX];
    char sPnr[MAX];
    char sAddress[MAX];
    struct Person* next;
} Person;

void printPersonList(Person* head) {
    Person* current = head;
    while (current != NULL) {
        printf("Name: %s\n", current->sName);
        printf("Surname: %s\n", current->sSurname);
        printf("Pnr: %s\n", current->sPnr);
        printf("Address: %s\n", current->sAddress);
        printf("\n");
        current = current->next;
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Connect to the server
    serverAddress.sin_family = AF_INET; /*A socket is created using the socket function. 
    The function takes parameters for the address family (AF_INET for IPv4), socket type (SOCK_STREAM for TCP), and protocol (0 for default protocol).
    If the socket creation fails, an error message is printed, and the program exits.*/
    serverAddress.sin_addr.s_addr = INADDR_ANY; /*The server address structure is
    initialized with the server's IP address (INADDR_ANY for any available address) and the port number to connect to (12345 in this example). */
    serverAddress.sin_port = htons(12345); // Change the port number if needed

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Receive the number of persons in the register
    int numPersons;
    recv(clientSocket, &numPersons, sizeof(int), 0);

    // Receive each person and print the list
    printf("Person List:\n");
    for (int i = 0; i < numPersons; i++) {
        Person person;
        recv(clientSocket, &person, sizeof(Person), 0);
        // Calculate the age based on the current date and the person's social security number
        // Add your code here to calculate the age

        // Print the person information
        printf("Name: %s\n", person.sName);
        printf("Surname: %s\n", person.sSurname);
        printf("Pnr: %s\n", person.sPnr);
        printf("Address: %s\n", person.sAddress);
        printf("\n");
    }

    return 0;
}
