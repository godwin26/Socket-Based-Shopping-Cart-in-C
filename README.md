# Socket-Based Shopping Cart in C

A client-server based shopping application developed in **C** using **TCP sockets**. The system allows multiple clients to connect to a centralized server concurrently using **POSIX threads**, view product catalog, add items to cart, update user account, place orders, and persist session data. Thread safety is ensured using **mutex locks**, and all session data is stored using basic file I/O.

---

## 🧠 Features

- Multi-client handling using `pthread` threads
- Product catalog with price and stock management
- Shopping cart system (add/view/place order)
- Account creation and update support
- Data persistence using local text files
- Thread-safe access to shared resources

---

## 🚀 How to Run

### 1. Compile the Server and Client
### 2. Start the Server
       ./server
       This starts the server and waits for client connections on port 8080.
### 3. Run the Client (In another terminal)
       ./client
       You can run multiple clients in different terminals.

## Technologies Used
     C Programming
     POSIX Threads (pthread)
     TCP/IP Socket Programming
     File I/O
     Mutex (for synchronization)



