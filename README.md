# Secure-Multithreaded-File-Transfer-System-C---Linux-
This project implements a secure, multithreaded client-server file transfer system written in C for Linux environments. 
It was developed as a coursework assignment for a Systems Software module, simulating a real-world scenario where departments
 in a company need to submit daily reports securely.

----------------------------------------------------------------------------------------------------
🚀 Features
🔐 User-based file ownership: Files are attributed to the sender using their Linux UID.

🗂️ Group-restricted directories: Files are routed to department-specific folders (Manufacturing / Distribution).

🔁 Multithreaded server: Handles multiple client connections simultaneously using pthread.

📤 One-way file transfer: Client sends files to the server (upload only).

🧾 Secure metadata handling: Client sends UID, department, filename, and size before transfer.

🔒 Thread synchronization: File writes are protected with mutex locks to avoid race conditions.

-----------------------------------------------------------------------------------------------------
⚙️ How It Works
The server listens on port 8080 and waits for clients to connect.
The client prompts the user for a file and department name.
The client sends:
Department name
Filename
UID of the sender (automatically retrieved)
File size
File contents
The server:
Receives all metadata
Writes the file to the appropriate department folder
Uses fchown() to assign ownership to the client UID
Confirms success or failure

-----------------------------------------------------------------------------------------------------
🖥️ Compilation & Execution

gcc server_program.c -o server -lpthread
gcc client_program.c -o client 
🧪 Run the server (as root or admin user)

sudo ./server
📤 Run the client (normal user)
./client
📸 Demo
You can view the full walkthrough in the demo video ((https://youtu.be/HFUgMxwnvY0)).

🛡️ Security & Assumptions
This project assumes local or LAN use; not designed for public internet.

No file overwrites allowed; all uploads are validated.

Runs on modern Linux distributions (Ubuntu tested).

The server must have server_data/ with proper permissions and group ownership set.

📚 Technologies Used
Language: C

Libraries: pthread, socket, unistd, fcntl, sys/stat

Platform: Linux (Ubuntu 20.04+)

