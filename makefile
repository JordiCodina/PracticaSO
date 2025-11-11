CC = gcc
CFLAGS = -g
EXTRAFLAGS = -Wall -Wextra
TARGET = Maester
SRCS = maester.c regne.c funcions_basiques.c producte.c config.c terminal.c client.c server.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(EXTRAFLAGS)

clean:
	rm -f $(TARGET)


SERVER = s
SERVER_SRCS = maestserver.c regne.c funcions_basiques.c producte.c config.c terminal.c client.c server.c

$(SERVER): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRCS) $(EXTRAFLAGS)

cleanserver:
	rm -f $(SERVER)