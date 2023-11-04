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

// Function to create a new Person with the given details
Person* createPerson(char name[], char surname[], char pnr[], char address[]) {
    // Allocate memory for the new Person
    Person* newPerson = (Person*)malloc(sizeof(Person));

    // Copy the provided details into the new Person
    strcpy(newPerson->sName, name);
    strcpy(newPerson->sSurname, surname);
    strcpy(newPerson->sPnr, pnr);
    strcpy(newPerson->sAddress, address);

    // Set the 'next' pointer to NULL
    newPerson->next = NULL;

    // Return the newly created Person
    return newPerson;
}

// Function to create a list of Persons
Person* createPersonList() {
    // Create the head of the list with initial person details
    Person* head = createPerson("John", "Doe", "1234567890", "New York");

    // Add more Persons to the list by linking them using the 'next' pointer
    head->next = createPerson("Jane", "Smith", "0987654321", "Los Angeles");
    head->next->next = createPerson("Alice", "Johnson", "5678901234", "Chicago");
    // Add more persons to the list if needed

    // Return the head of the list
    return head;
}

// Function to print the list of Persons
void printPersonList(Person* head) {
    // Start from the head of the list
    Person* current = head;

    // Traverse the list and print each Person's details
    while (current != NULL) {
        printf("Name: %s\n", current->sName);
        printf("Surname: %s\n", current->sSurname);
        printf("Pnr: %s\n", current->sPnr);
        printf("Address: %s\n", current->sAddress);
        printf("\n");

        // Move to the next Person in the list
        current = current->next;
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressLength = sizeof(clientAddress);

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Bind the socket to a specific IP and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(12345); // Change the port number if needed

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Listening failed");
        exit(1);
    }

    printf("Server is running...\n");

    // Create the person list
    Person* personList = createPersonList();

    // Print the person list
    printf("Person List:\n");
    printPersonList(personList);

    while (1) {
        // Accept client connection
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            perror("Accepting failed");
            exit(1);
        }

        // Send the number of persons in the register
        int numPersons = 0;
        Person* current = personList;
        while (current != NULL) {
            numPersons++;
            current = current->next;
        }
        send(clientSocket, &numPersons, sizeof(int), 0);

        // Send each person
        current = personList;
        while (current != NULL) {
            send(clientSocket, current, sizeof(Person), 0);
            current = current->next;
        }
    }

    return 0;
}
