# Text Messenger - Project for System and Concurrent Programming
A text messenger operated in the Linux terminal, written in C

********Program Functionality********

The project is a text messenger operating in the Linux terminal, based on IPC mechanisms. The primary role is played by message queues, which are used to transmit all information between the client and the server. The messenger allows a user to operate on their personal account, enabling them to:
 - Register
 - Log in
 - Log out
 - Send messages to another user
 - Send messages to a group of users they are subscribed to
 - View logged-in users
 - View available topic-based groups
 - View members of a group
 - Join a group
 - Leave a group
 - Check messages
 - Block another user (to stop receiving messages from them)



********Compilation instructions********

For both the server and the client (in separate terminal windows):
- gcc serwer.c -Wall -o serwer
- gcc klient.c -Wall -o klient

********Execution instructions********

For both the server and each client who wants to log in (in separate terminal windows):
- ./serwer
- ./klient



********Brief Description of File Contents********

---Client file:

- It is divided into two main processes:
  
   *first process:
  	- Contains two main loops:
  	The first loop, "not logged in," is an interface that appears when the user is not logged in, allowing them to:
	   > Log in 
	   > Register
	   > Close the application
	The second loop, "logged in," grants access to the server’s main functions, but only after logging in during the previous stage of the program. It includes functions that allow the user to:
	   > Send a message to another user or an entire group
	   > Check who is logged in, available groups and their members
	   > Join or leave a group
	   > Log out and exit the program (closing the appropriate queue)

   *second process:
	- Responsible for capturing incoming messages and displaying them on the screen. It is always active, meaning messages appear as soon as they are received.


---Server file:

- Implements the various functions mentioned above (responding to client requests).
- Maintains a "database," which consists of additional structures storing information about clients and groups necessary for directing messages appropriately.


---Configuration file:

- Contains the default user database and user groups.


********Data Structure Used for Message Transition********

struct zapytanie{
    int id_nadawcy, id_adresata, polecenie, error;
    char wiadomosc[DLUGOSC_WIADOMOSCI], nick[10];
};


********Structure fields********

-- "polecenie" (int) - An integer indicating the type of request/respon
Specific numbers correspond to specific commands, e.g.:
'3' - Sending a message,
'7' - Leaving a group.

-- "id_nadawcy", (int) - The ID of the sender's queue when sending a request to the server.
Used to identify the sender when the message reaches them via the server.

-- "id_adresata", (int) - Used to identify the recipient of the message (user, group, or server). In some functions, it is used as a buffer for other required data.

-- "wiadomosc", (char[]) -  A character array storing the actual message content.

-- "nick", (char[]) - A variable storing the user's nickname when logging into the server.

-- "error", (int) - A variable used to convey information about the success or failure of an action.


********Message Transmission Protocol********

a) Client -> Server communication

The server creates its own message queue, from which it reads all incoming requests.
Each client sends their requests to the single server queue.


b) Server -> Client communication

Each client has its own message queue, whose ID is provided to the server during the login process.
The server sends responses to requests and relayed messages to the client via this personal queue.


c) The total number of queues is n+1, where n is the number of active clients (the additional +1 refers to the server queue).


