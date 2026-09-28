# Multithreaded TCP Server in C

Simple multithreaded TCP server written in C as part of my Computer Science coursework at ENSICAEN.

## Features

* TCP sockets
* Multithreading with `pthread`
* Client/server communication
* Converts received messages to uppercase

## Usage

Compile:

```bash
make
```

Run:

```bash
./serveur 8000
```

Then connect with a TCP client:

```bash
nc localhost 8000
```

