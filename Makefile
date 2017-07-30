all: cli ser dispatch cli.o ser.o dispatch.o

cli.o: cli.cpp	msg.h
	g++ -c cli.cpp -o cli.o -g
ser.o: ser.cpp msg.h
	g++ -c ser.cpp -o ser.o -g
dispatch.o: threadPool.cpp dispatch.h msg.h SerMessage.h
	g++ -c threadPool.cpp -o dispatch.o -g
cli: cli.o
	g++ cli.o -o cli -g
ser: ser.o
	g++ ser.o -o ser -g
dispatch: dispatch.o 
	g++ dispatch.o -o dispatch -g
	
clean:
	rm -rf *.o dispatch cli ser output output.tar.gz
