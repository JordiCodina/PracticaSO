CC = gcc
CFLAGS = -g
EXTRAFLAGS = -Wall -Wextra -lpthread
TARGET = Maester
SRCS = maester.c regne.c funcions_basiques.c producte.c config.c terminal.c client.c server.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(EXTRAFLAGS)

valA: $(TARGET)
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes --show-leak-kinds=all -s ./$(TARGET) A.dat stock.db

valB: $(TARGET)
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./$(TARGET) B.dat stockb.db

valC: $(TARGET)
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./$(TARGET) C.dat stock.db

a: $(TARGET)
	./$(TARGET) A.dat stocka.db

b: $(TARGET)
	./$(TARGET) B.dat stockb.db

c: $(TARGET)
	./$(TARGET) C.dat stockc.db

clean:
	rm -f $(TARGET)
