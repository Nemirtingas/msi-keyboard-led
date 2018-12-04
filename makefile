FLAGS = -W -Wall -Werror -std=c++14
 
EXEC = msi-keyboard-manager
CXX = g++
CXXFLAGS = $(FLAGS) -O3
#CXXFLAGS = $(FLAGS) -fPIC -O3 -D__EXPORT_SYMBOLS__
CXXCOMPFLAGS = 
#CXXCOMPFLAGS = -shared -fPIC -Wl,-soname,$(EXEC).1
CXXINCLUDES = 
LIBRARIES = -lhidapi-libusb

SERVICE_USER = root

KEYBOARD_SRC     = keyboard.cpp
KEYBOARD_HEADERS = keyboard.h
KEYBOARD_OBJS    = obj/keyboard.o

SOURCES  = main.cpp
SOURCES += $(KEYBOARD_SRC)

HEADERS  = utils.h
HEADERS += $(KEYBOARD_INCLUDES)

OBJS  = obj/main.o
OBJS += $(KEYBOARD_OBJS)
 
debug: CXXFLAGS = $(FLAGS) -ggdb
debug: all

.PHONY: clean, mrproper

all: $(OBJS)
	@[ -e obj ] || mkdir obj
	@[ -e bin ] || mkdir bin
	@[ -e out ] || mkdir out
	@echo Making service file...
	@$(MAKE) out/$(EXEC).service
	@$(MAKE) out/start.conf
	@$(MAKE) out/$(EXEC).conf
	$(CXX) $(CXXCOMPFLAGS) $^ $(LIBRARIES) -o bin/$(EXEC) && strip bin/$(EXEC) -o out/$(EXEC)
 
main.o: $(HEADERS)
 
obj/%.o: %.cpp $(INCLUDES)
	@[ -e obj ] || mkdir obj
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CXXINCLUDES)

clean: 
	find . -name '*.o' -exec rm -f {} \;
 
mrproper:
	rm -rf bin obj out
 
out/$(EXEC).service:
	@[ -e out ] || mkdir out
	@echo \
"[Unit]\\n\
Description=Control SteelSeries keyboard on MSI Laptops\\n\
\\n\
[Service]\\n\
Type=simple\\n\
User=$(SERVICE_USER)\\n\
StandardOutput=syslog\\n\
StandardError=syslog\\n\
SyslogIdentifier=$(EXEC)\\n\
ExecStart=/usr/sbin/$(EXEC)\\n\
\\n\
[Install]\\n\
WantedBy=multi-user.target" >$@

out/start.conf:
	@[ -e out ] || mkdir out
	@echo \
"normal_mode left   #FF0000\\n\
normal_mode middle #00FF00\\n\
normal_mode right  #0000FF" >$@

out/$(EXEC).conf:
	@[ -e out ] || mkdir out
	@echo \
"# See tmpfiles.d(5) for details\\n\
\\n\
# Make sure these are created by default so that nobody else can\\n\
# or empty them at startup\\n\
D! /var/run/$(EXEC) 700 root root" >$@


install: all
	grep -q "^$(SERVICE_USER)" /etc/passwd || useradd -r -M -s /usr/sbin/nologin -d /var/run/$(EXEC) $(SERVICE_USER)
	cp out/$(EXEC) /usr/sbin/$(EXEC)
	cp out/$(EXEC).service /lib/systemd/system/$(EXEC).service
	cp out/$(EXEC).conf /usr/lib/tmpfiles.d/$(EXEC).conf
	@[ -e /var/run/$(EXEC) ] || mkdir /var/run/$(EXEC)
	chown $(SERVICE_USER):$(SERVICE_USER) /var/run/$(EXEC) /usr/sbin/$(EXEC)
	chmod 755 /var/run/$(EXEC)
	@[ -e /etc/$(EXEC) ] || mkdir /etc/$(EXEC)
	@[ -e /etc/$(EXEC)/start.conf ] || cp out/start.conf /etc/$(EXEC)/start.conf
	@systemctl daemon-reload
	@echo "\nYou can start the keyboard service with: systemctl start $(EXEC)\n"
