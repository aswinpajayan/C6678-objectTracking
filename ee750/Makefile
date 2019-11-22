CC     = g++
CCC    = gcc	
CFLAGS = `pkg-config --cflags opencv4`
LIBS   = `pkg-config --libs opencv4`
LD_FLAGS = -pthread
DEBUG  = -Wall -g -Wextra

OBJECTS = UI.o

.PHONY: clean

all: ui

ui: $(OBJECTS)
	$(CC) $(DEBUG) $(OBJECTS) $(LIBS) $(LD_FLAGS) -o $@
UI.o: CvLoad.cpp
	$(CC) $(DEBUG) $(CFLAGS) -c $< -o $@
client.o: UDP_client.c
	$(CCC) $(DEBUG) $(CFLAGS) -c $< -o $@
server.o: UDP_server.c
		$(CCC) $(DEBUG) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o ui


run:
	./ui
