all: ringmaster player

ringmaster: ringmaster.cpp socket.cpp potato.h
	g++ -g -o ringmaster ringmaster.cpp socket.cpp 
player: player.cpp socket.cpp potato.h
	g++ -g -o player player.cpp socket.cpp 

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player