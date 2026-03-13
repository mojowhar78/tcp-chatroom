# TTTALK - TCP Chatroom

A lightweight, feature-rich TCP chatroom application built in C with an interactive **ncurses-based user interface** for real-time communication between multiple users over a network.

## Features

- **ncurses UI**: Beautiful terminal-based graphical interface with color support and dynamic window management
- **TCP/IP Protocol**: Reliable connection-oriented communication
- **Multi-client Support**: Handle multiple simultaneous connections
- **Real-time Messaging**: Instant message delivery between connected users
- **Interactive Interface**: Separate input and output windows for seamless messaging experience
- **Lightweight**: Efficient C implementation with minimal overhead
- **Cross-platform**: Runs on Unix-like systems (Linux, macOS, etc.)

## Prerequisites

- GCC or Clang compiler
- C standard library (libc)
- **ncurses library** (`libncurses5-dev` on Ubuntu/Debian, `ncurses` on macOS via Homebrew)
- POSIX-compliant system
- Basic knowledge of TCP/IP networking

## Installation

### Ubuntu/Debian

```bash
sudo apt-get install libncurses5-dev build-essential
```

### macOS

```bash
brew install ncurses
```

## Building

Compile the project using:

```bash
gcc -o tttalk *.c -lncurses
```

Or if a Makefile is available:

```bash
make
```

## Usage

### Starting the Server

```bash
./tttalk -s [port]
```

Starts the chatroom server on the specified port (default: 9090).

### Connecting as a Client

```bash
./tttalk -c [hostname] [port]
```

Connects to a chatroom server with an interactive ncurses UI at the specified hostname and port.

### Example

**Terminal 1 (Server):**
```bash
./tttalk -s 9090
```

**Terminal 2 (Client 1):**
```bash
./tttalk -c localhost 9090
```

**Terminal 3 (Client 2):**
```bash
./tttalk -c localhost 9090
```

## User Interface

The ncurses client provides an intuitive split-window interface:

- **Message Display Area**: Shows all incoming messages from other users
- **Input Area**: Type and send your messages to the chatroom
- **Status Bar**: Displays connection information and user count
- **Color Support**: Easy-to-read colored text for better readability

### Keyboard Controls

- **Type**: Enter your message
- **Enter**: Send message
- **Ctrl+C**: Exit the chatroom

## Architecture

### Project Structure

- **`server.c`** / **`server.h`**: Server implementation handling client connections and message broadcasting
- **`client.c`** / **`client.h`**: Client implementation for connecting to the chatroom
- **`ui.c`** / **`ui.h`**: ncurses-based UI components for the client interface
- **`header.h`**: Shared headers and common definitions
- **`test.c`**: Testing utilities

### Key Components

- **Socket Programming**: Uses BSD sockets for TCP communication
- **ncurses Library**: Terminal-based UI with window management and color support
- **Threading/Multiplexing**: Handles multiple concurrent connections and user input
- **Buffer Management**: Efficient message buffering and transmission

## Limitations & Future Improvements

- [ ] Message history/logging
- [ ] User list display with nicknames
- [ ] Graceful shutdown handlers
- [ ] Enhanced error handling and validation
- [ ] Security features (encryption, SSL/TLS)
- [ ] Emoji and unicode support
- [ ] Auto-reconnect on connection loss


## Contributing

Feel free to fork this project and submit pull requests for any improvements.

## License

This project is open source and available under your choice of license. Consider adding a LICENSE file.

## Author

Created by [mojowhar78](https://github.com/mojowhar78)

## Support

For issues, questions, or suggestions, please open an issue on GitHub.
