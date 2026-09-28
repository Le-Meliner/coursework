CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lpthread

TARGET = serveur
SRC = server.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
