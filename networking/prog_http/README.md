# HTTP Server in C

Simple HTTP server written in C as part of my Computer Science coursework at ENSICAEN.

## Features
* TCP sockets
* Multithreading with pthread
* HTTP GET requests
* HTML and image file serving
* Basic MIME type detection
* 404 error handling

## Usage

Compile:

```bash
make
```

Run:

```bash
./serveur 8000
```

Then open:

http://localhost:8000/
