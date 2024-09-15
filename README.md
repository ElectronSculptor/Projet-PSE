
# Battleship Game - Client-Server Application

## Introduction

This project implements a network-based version of the classic Battleship game, where two players (clients) can connect to a server and play against each other. Each player places ships on a 10x10 grid and takes turns to target the opponent's ships. The game ends when one player loses all their ships.

## Project Structure

### `serveur.c`
The server code (`serveur.c`) manages the flow of the game, processing inputs from both clients and determining the game state. The server is responsible for:
- Handling client connections.
- Managing turns and game rules.
- Communicating the status of the game to each client.

The main sections of the `serveur.c` code are:
- Client connection management.
- Game state management.
- Communication with clients.

### `client.c`
The client code (`client.c`) handles player interaction with the game. It sends player inputs to the server and receives game updates. The client is responsible for:
- Allowing the player to input their ship coordinates.
- Sending commands to the server.
- Displaying the current game state based on server responses.

The main sections of the `client.c` code are:
- Initial connection setup.
- Input management.
- Communication with the server.

### Overall Architecture
The server and client codes work together to enable real-time gameplay between two clients. The server maintains the game state and synchronizes the actions between both clients. A sequence diagram in the project documentation illustrates the flow of communication between the server and the clients.

## User Guide

### Compilation and Execution

#### To Compile:
1. Open a terminal in the project folder.
2. Run the command: `make`

#### To Launch the Game:
1. Open three terminals in the same project directory.
2. In the first terminal, start the server with the command:
   ```bash
   ./serveur_bataille_navale <port_number>
   ```
   Replace `<port_number>` with a valid port number (e.g., 2024).

3. In the other two terminals, start the clients with the command:
   ```bash
   ./client_bataille_navale localhost <port_number>
   ```
   Ensure the same port number is used for both clients.

Example:
```bash
./serveur_bataille_navale 2024
./client_bataille_navale localhost 2024
```

Note: Players can place ships simultaneously, but ensure the second player finishes last to avoid synchronization issues.

## Challenges Faced

Some challenges encountered during the development include:
- Managing multiple client connections for simultaneous gameplay.
- Debugging issues caused by the blocking `read()` function.
  
## Areas for Improvement

Possible future improvements:
- Add a graphical user interface for better user experience.
- Implement artificial intelligence to allow solo play.
- Encrypt communication between the client and server for enhanced security.
- Prevent players from entering inputs when it's not their turn.
- Ensure ship placement does not result in overlaps.

## Acknowledgments

I would like to thank my professor Xavier SERPAGGI for his guidance and my classmate Bastien GARNIER for his debugging help.
