CC = gcc
CXX = g++ -std=c++0x
INCLUDES = -I/usr/local/include
CFLAGS = -c -Wall -DUSE_WEBSOCKET -DLINUX $(INCLUDES)
LDFLAGS =
LIBS = -lraspicam -lmmal -lmmal_core -lmmal_util
SOURCES = main.cpp 
HEADERS = 

OBJECTS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))
EXECUTABLE = bellcam

all: $(SOURCES) $(EXECUTABLE)
debug: CC += -g
debug: CXX += -g
debug: $(SOURCES) $(EXECUTABLE)
release: CC += -O3
release: CXX += -O3
release: $(SOURCES) $(EXECUTABLE)
prof: CC += -pg
prof: CXX += -pg
prof: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o : %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

ifeq ($(UNAME_P),x86_64)    
%.o : %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $< -o $@
endif

clean:
	rm -f *.o */*.o $(EXECUTABLE)
