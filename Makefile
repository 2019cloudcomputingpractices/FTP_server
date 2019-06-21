CXXFLAGS += -std=c++11 -I./include -pthread
DEPS = include/utils.h include/server.h include/connection.h

ftp_server: utils.o server.o connection.o main.o
	g++ -o $@ $^ $(CXXFLAGS)
	rm $^

utils.o: src/utils.cpp $(DEPS)
	g++ -c -o $@ $< $(CXXFLAGS)

server.o: src/server.cpp $(DEPS)
	g++ -c -o $@ $< $(CXXFLAGS)

connection.o: src/connection.cpp $(DEPS)
	g++ -c -o $@ $< $(CXXFLAGS)

main.o: src/main.cpp $(DEPS)
	g++ -c -o $@ $< $(CXXFLAGS)