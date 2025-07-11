#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void show_menu();
void handle_add_product(int sock);
void handle_view_cart(int sock);
void handle_place_order(int sock);
void handle_update_account(int sock);
void send_request(int sock, const char *request, char *response);
void receive_product_list(int server_socket);

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    // Receive the product list from the server
    receive_product_list(sockfd);

    // Main interaction menu
    while (1) {
        int choice;
        printf("\n========== Menu ========== \n");
        printf("1. Add Products\n");
        printf("2. Display Items in Cart\n");
        printf("3. To Place Order\n");
        printf("4. Account Options\n");
        printf("5. EXIT\n");
        printf("==========================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline character left by scanf

        char command[BUFFER_SIZE];
        switch (choice) {
            case 1:
                // Handle adding products
                handle_add_product(sockfd);
                break;

            case 2:
                // Handle viewing cart
                handle_view_cart(sockfd);
                break;

            case 3:
                // Handle placing order
                handle_place_order(sockfd);
                break;

            case 4:
                // Handle account options
                handle_update_account(sockfd);
                break;

            case 5:
                // Exit the application
                sprintf(command, "quit");
                send(sockfd, command, strlen(command), 0);
                close(sockfd);
                exit(0);

            default:
                printf("Invalid choice, try again.\n");
        }
    }

    return 0;
}

// Display menu options
void show_menu() {
    printf("\n========== Menu ==========\n");
    printf("1. Add Products\n");
    printf("2. Display Items in Cart\n");
    printf("3. To Place Order\n");
    printf("4. Account Options\n");
    printf("5. EXIT\n");
    printf("==========================\n");
}

// Handle adding products to the cart
void handle_add_product(int sock) {
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int product_index, quantity;

    printf("\nEnter product number (1-4): ");
    scanf("%d", &product_index);
    printf("Enter quantity: ");
    scanf("%d", &quantity);
    getchar(); // Consume newline character left by scanf

    // Send add product request to server
    snprintf(request, sizeof(request), "add product %d %d", product_index, quantity);
    send_request(sock, request, response);

    printf("%s\n", response);
}

// Handle viewing the cart
void handle_view_cart(int sock) {
    char response[BUFFER_SIZE];

    // Send view cart request to server
    send_request(sock, "view cart", response);

    printf("\n%s\n", response);
}

// Handle placing an order
void handle_place_order(int sock) {
    char response[BUFFER_SIZE];

    // Send place order request to server
    send_request(sock, "place order", response);

    printf("\n%s\n", response);
}

// Handle updating account details
void handle_update_account(int sock) {
    char response[BUFFER_SIZE];

    // Send update account request to server
    send_request(sock, "update account", response);

    printf("\nServer: %s\n", response);

    // Collect and send account details
    printf("Enter your name: ");
    char name[30];
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; // Remove newline character

    printf("Enter your address: ");
    char address[40];
    fgets(address, sizeof(address), stdin);
    address[strcspn(address, "\n")] = '\0';

    printf("Enter your PIN: ");
    char pin[8];
    fgets(pin, sizeof(pin), stdin);
    pin[strcspn(pin, "\n")] = '\0';

    printf("Enter your mobile number: ");
    char mobno[10];
    fgets(mobno, sizeof(mobno), stdin);
    mobno[strcspn(mobno, "\n")] = '\0';

    // Combine account details into a request
    snprintf(response, sizeof(response), "name:%s,address:%s,pin:%s,mobno:%s", name, address, pin, mobno);
    send(sock, response, strlen(response), 0);

    printf("Account updated successfully.\n");
}

// Send request to server and receive response
void send_request(int sock, const char *request, char *response) {
    send(sock, request, strlen(request), 0);
    int read_size = recv(sock, response, BUFFER_SIZE - 1, 0);
    if (read_size > 0) {
        response[read_size] = '\0';
    } else {
        strcpy(response, "No response from server.");
    }
}

// Receive product list from server
void receive_product_list(int server_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Receive the product list from the server
    bytes_received = recv(server_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the string
        printf("%s", buffer);           // Display the product list
    } else {
        printf("Failed to receive product list from the server.\n");
    }
}
