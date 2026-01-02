# with valgrind

valA: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./Maester A.dat stocka.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

valB: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester 
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./Maester B.dat stockb.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

valC: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./Maester C.dat stockc.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

valD: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester 
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./Maester D.dat stockd.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

valE: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	valgrind --dsymutil=yes --track-origins=yes --leak-check=full --track-fds=yes --show-reachable=yes -s ./Maester E.dat stocke.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

#without valgrind

a: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	./Maester A.dat stocka.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

b: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	./Maester B.dat stockb.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

c: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	./Maester C.dat stockc.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

d: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	./Maester D.dat stockd.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o

e: maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o
	gcc -g maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o -o Maester
	./Maester E.dat stocke.db
	rm maester.o terminal.o server.o producte.o regne.o client.o config.o envoy_process.o funcions_basiques.o


maester.o: maester.c terminal.o server.o client.o envoy_process.o funcions_basiques.o
	gcc -g -Wall -Wextra -lpthread -c maester.c -o maester.o

terminal.o: terminal.c server.o producte.o regne.o client.o config.o
	gcc -g -Wall -Wextra -c terminal.c -o terminal.o

server.o: server.c producte.o client.o config.o funcions_basiques.o
	gcc -g -Wall -Wextra -c server.c -o server.o

producte.o: producte.c regne.o client.o config.o funcions_basiques.o
	gcc -g -Wall -Wextra -c producte.c -o producte.o

regne.o: regne.c client.o config.o funcions_basiques.o
	gcc -g -Wall -Wextra -c regne.c -o regne.o

client.o: client.c config.o funcions_basiques.o
	gcc -g -Wall -Wextra -c client.c -o client.o

config.o: config.c funcions_basiques.o
	gcc -g -Wall -Wextra -c config.c -o config.o

envoy_process.o: envoy_process.c regne.o
	gcc -g -Wall -Wextra -c envoy_process.c -o envoy_process.o

funcions_basiques.o: funcions_basiques.c
	gcc -g -Wall -Wextra -c funcions_basiques.c -o funcions_basiques.o
