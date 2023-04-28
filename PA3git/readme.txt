Project II: Tic-Tac-Toe On-line
Due: March 27, 2023
_________________________________________________________________________________________________________________________________
Name and NetID:
Kusum Gandham kg699   *submitting assignment
_________________________________________________________________________________________________________________________________

Project Goal:
This project's goal is to use sockets and threading to make a playable game of tic-tac-toe online. 
_________________________________________________________________________________________________________________________________

To start:
After initiating the server and the client. The client will send a play message and the server will immediately follow it up
with a WAIT|0|. It will then begin the game and assigns each user with the X and O.

Making Moves:
The player can then take turns making moves on the board. They are not allowed to overlap with a spot already filled and After
every move we checked for winners on the board. If there is win or all the spots are filled the game terminates with an over message.

Resign:
At any point the player can take a loss and resign from the game and terminating the socket.

Draw:
A player can request a Draw S and the other user can either accept or reject Draw. If accepted the game will terminate with an
over. Otherwise the game continues.

Error handling:
The server checks for valid format and making sure certain messages arent being sent at certain times. It also checks if 
the bytes stated is corrected.

Technical: 
We created sockets and two threads. The server keeps track of each socket and thread in a struct for each client to keep track of
information like name, role, socket, and certain flags to keep track of draws and resigns.