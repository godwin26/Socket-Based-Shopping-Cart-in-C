#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Struct Definitions
struct account {
    char name[30];
    char address[40];
    char pin[8];
    char mobno[10];
};

struct product {
    char name[30];
    int price;
    int stock;
};

// Global Variables
struct account b1 = {"", "", "", ""};
struct product products[4];
int cart[4] = {0}; // Quantities of products in cart
int total_cost = 0, total_items = 0;
int account_created = 0;
pthread_mutex_t mutex; // Mutex for shared resources (cart and account)

// Function Declarations
void initialize_products();
void handle_client(int client_socket);
void process_command(char *command, char *response, int client_socket);
void add_product_to_cart(int product_index, int quantity, char *response);
void display_cart(char *response);
void place_order(char *response);
void update_account(char *response);
void save_account_data();
void save_cart_data();
void load_account_data();
void load_cart_data();
void clear_cart();
void *client_thread(void *arg);

// Main Function
int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Initialize mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);
    initialize_products();
    load_account_data();
    load_cart_data();

    // Accept client connections
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");

        // Create a new thread to handle the client
        if (pthread_create(&thread_id, NULL, client_thread, (void *)&client_socket) != 0) {
            perror("Thread creation failed");
        }
    }

    // Cleanup
    pthread_mutex_destroy(&mutex);
    return 0;
}

// Client thread function
void *client_thread(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int read_size;

    // Handle client communication
    handle_client(client_socket);
    close(client_socket);
    printf("Client disconnected.\n");
    return NULL;
}

// Initialize product catalog
void initialize_products() {
    strcpy(products[0].name, "TITAN");
    products[0].price = 999;
    products[0].stock = 10;

    strcpy(products[1].name, "FASTTRACK");
    products[1].price = 1299;
    products[1].stock = 9;

    strcpy(products[2].name, "PHILIPS");
    products[2].price = 799;
    products[2].stock = 5;

    strcpy(products[3].name, "SISKO");
    products[3].price = 350;
    products[3].stock = 7;
}

// Handle client communication
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int read_size;

    // Initial product list display to the client
    char product_list_response[BUFFER_SIZE];
    strcpy(product_list_response, "Available products:\n");
    for (int i = 0; i < 4; i++) {
        sprintf(buffer, "%d. %s (Rs.%d) - Stock: %d\n", i + 1, products[i].name, products[i].price, products[i].stock);
        strcat(product_list_response, buffer);
    }
    send(client_socket, product_list_response, strlen(product_list_response), 0);

    // Wait for client input (choosing product or other commands)
    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("Client: %s\n", buffer);

        // Process command and generate response
        process_command(buffer, response, client_socket);

        // Send response to client
        send(client_socket, response, strlen(response), 0);
    }
}

// Process client commands
void process_command(char *command, char *response, int client_socket) {
    if (strncmp(command, "add product", 11) == 0) {
        int product_index, quantity;
        sscanf(command + 12, "%d %d", &product_index, &quantity);
        add_product_to_cart(product_index - 1, quantity, response);
    } else if (strcmp(command, "view cart") == 0) {
        display_cart(response);
    } else if (strcmp(command, "place order") == 0) {
        place_order(response);
    } else if (strncmp(command, "update account ", 15) == 0) {
        // Parse account details
        sscanf(command + 15, "%29[^,],%39[^,],%7[^,],%9s", b1.name, b1.address, b1.pin, b1.mobno);
        account_created = 1;
        sprintf(response, "Account details updated successfully: %s, %s, %s, %s", b1.name, b1.address, b1.pin, b1.mobno);
    } else if (strcmp(command, "save account and cart") == 0) {
        save_account_data();
        save_cart_data();
        sprintf(response, "Account and cart data saved successfully.");
    } else if (strcmp(command, "load account and cart") == 0) {
        load_account_data();
        load_cart_data();
        sprintf(response, "Account and cart data loaded successfully.");
    } else if (strcmp(command, "quit") == 0) {
        sprintf(response, "Goodbye!");
    } else {
        sprintf(response, "Invalid command.");
    }
}

// Add product to cart
void add_product_to_cart(int product_index, int quantity, char *response) {
    pthread_mutex_lock(&mutex); // Lock the mutex before modifying shared resources
    if (product_index < 0 || product_index >= 4) {
        sprintf(response, "Invalid product selection.");
        pthread_mutex_unlock(&mutex);
        return;
    }

    if (quantity > products[product_index].stock) {
        sprintf(response, "Insufficient stock for %s.", products[product_index].name);
        pthread_mutex_unlock(&mutex);
        return;
    }

    products[product_index].stock -= quantity;
    cart[product_index] += quantity;
    total_items += quantity;
    total_cost += products[product_index].price * quantity;

    sprintf(response, "%d x %s added to cart.", quantity, products[product_index].name);
    pthread_mutex_unlock(&mutex); // Unlock the mutex after modifying shared resources
}

// Display items in cart
void display_cart(char *response) {
    pthread_mutex_lock(&mutex);
    char buffer[BUFFER_SIZE];
    strcpy(response, "Items in your cart:\n");
    for (int i = 0; i < 4; i++) {
        if (cart[i] > 0) {
            sprintf(buffer, "%d x %s (Rs.%d each)\n", cart[i], products[i].name, products[i].price);
            strcat(response, buffer);
        }
    }
    sprintf(buffer, "\nTotal Items: %d\nTotal Cost: Rs.%d\n", total_items, total_cost);
    strcat(response, buffer);
    pthread_mutex_unlock(&mutex);
}

// Place order
void place_order(char *response) {
    pthread_mutex_lock(&mutex);
    if (total_items == 0) {
        sprintf(response, "Your cart is empty. Add products before placing an order.");
        pthread_mutex_unlock(&mutex);
        return;
    }

    strcat(response, "Order placed successfully. Thank you for shopping with us!\n");
    clear_cart();
    pthread_mutex_unlock(&mutex);
}

// Update account details
void update_account(char *response) {
    sprintf(response, "Enter your account details: Name, Address, PIN, Mobile No.");
    account_created = 1;
}

// Save account data to a file
void save_account_data() {
    pthread_mutex_lock(&mutex);
    FILE *file = fopen("account_data.txt", "w");
    if (!file) {
        printf("Error saving account data.\n");
        pthread_mutex_unlock(&mutex);
        return;
    }

    // Save account details
    fprintf(file, "%s\n%s\n%s\n%s\n", b1.name, b1.address, b1.pin, b1.mobno);
    fclose(file);
    printf("Account data saved successfully.\n");
    pthread_mutex_unlock(&mutex);
}

// Save cart data to a file
void save_cart_data() {
    pthread_mutex_lock(&mutex);
    FILE *file = fopen("cart_data.txt", "w");
    if (!file) {
        printf("Error saving cart data.\n");
        pthread_mutex_unlock(&mutex);
        return;
    }

    // Save cart details
    for (int i = 0; i < 4; i++) {
        fprintf(file, "%d ", cart[i]);
    }
    fprintf(file, "\n%d %d\n", total_cost, total_items);  // Save total cost and total items
    fclose(file);
    printf("Cart data saved successfully.\n");
    pthread_mutex_unlock(&mutex);
}

// Load account data from a file
void load_account_data() {
    FILE *file = fopen("account_data.txt", "r");
    if (!file) {
        printf("No saved account data found.\n");
        return;
    }

    // Load account details
    fscanf(file, "%s\n%s\n%s\n%s\n", b1.name, b1.address, b1.pin, b1.mobno);
    fclose(file);
    printf("Account data loaded successfully.\n");
}

// Load cart data from a file
void load_cart_data() {
    FILE *file = fopen("cart_data.txt", "r");
    if (!file) {
        printf("No saved cart data found.\n");
        return;
    }

    // Load cart details
    for (int i = 0; i < 4; i++) {
        fscanf(file, "%d ", &cart[i]);
    }

    // Load total cost and total items
    fscanf(file, "%d %d\n", &total_cost, &total_items);
    fclose(file);
    printf("Cart data loaded successfully.\n");
}

// Clear the cart
void clear_cart() {
    for (int i = 0; i < 4; i++) {
        products[i].stock += cart[i];
        cart[i] = 0;
    }
    total_cost = 0;
    total_items = 0;
}
