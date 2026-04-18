# Micro HTTP/1.1 Server (C, Berkeley Sockets)

## Project Overview
This project implements a lightweight HTTP/1.1 web server from scratch using the Berkeley Socket API in C.
The server accepts TCP connections from clients (web browsers), manually parses HTTP requests, and sends properly formatted HTTP responses.

## Features
- Uses Berkeley Socket API (socket, bind, listen, accept)
- Manual HTTP request parsing (method, path, version)
- Supports GET requests
- Custom routing:
  / -> index.html
  /about -> about.html
  /contact -> contact.html
  /projects -> projects.html
- Static file serving from www directory
- Supports HTML, CSS, images, text
- Sends HTTP headers (Date, Server, Content-Type, Content-Length, Last-Modified, Connection)
- Handles errors (400, 404, 405)
- Logs requests to server.log
- Prevents directory traversal (..)

## Project Structure
MicroHttpServer/
- server.c
- server.log
- README.md
- www/
  - index.html
  - about.html
  - contact.html
  - projects.html
  - style.css
  - 404.html
  - network.jpg

## How to Run (WSL/Linux)

Compile:
gcc server.c -o server

Run:
./server

Open browser:
http://localhost:8080

## Available Routes
http://localhost:8080/
http://localhost:8080/about
http://localhost:8080/contact
http://localhost:8080/projects
http://localhost:8080/style.css
http://localhost:8080/network.jpg

## Concepts Demonstrated
- TCP socket programming
- Berkeley Socket API
- HTTP protocol structure
- Manual request parsing
- Response construction
- File I/O in C
- Logging
- Security (path validation)

## Author
Ilia Mchedlishvili
CPAN226 – Network Programming
Humber Polytechnic
