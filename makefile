
.PHONY: clean, mrproper
 
FLAGS = -W -Wall -Werror -std=c++14
 
EXEC = bin/msi-keyboard-manager
CXX = g++
CXXFLAGS = $(FLAGS) -O3
#CXXFLAGS = $(FLAGS) -fPIC -O3 -D__EXPORT_SYMBOLS__
CXXCOMPFLAGS = 
#CXXCOMPFLAGS = -shared -fPIC -Wl,-soname,$(EXEC).1
CXXINCLUDES = 
LIBRARIES = -lhidapi-libusb
 
KEYBOARD_SRC     = keyboard.cpp
KEYBOARD_HEADERS = keyboard.h
KEYBOARD_OBJS    = obj/keyboard.o

SOURCES = main.cpp $(KEYBOARD_SRC)
#SOURCES += 

HEADERS = utils.h $(KEYBOARD_INCLUDES)
#HEADERS += 

OBJS    = obj/main.o $(KEYBOARD_OBJS)
 
all: $(OBJS)
	[ -e bin ] || mkdir bin
	$(CXX) $(CXXCOMPFLAGS) $^ $(LIBRARIES) -o $(EXEC)  && strip $(EXEC)
 
main.o: $(HEADERS)
 
obj/%.o: %.cpp $(INCLUDES)
	[ -e obj ] || mkdir obj
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CXXINCLUDES)

clean: 
	find . -name '*.o' -exec rm -f {} \;
 
mrproper: clean
	rm -rf $(EXEC)
 
debug: CXXFLAGS = $(FLAGS) -ggdb
debug: all
