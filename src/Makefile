CC=g++
SOURCES=mailersmtp.cpp \
	mailererrors.cpp \
	sqlmailer.cpp \
	mymailqueue.cpp \
	mailerparams.cpp \
	main.cpp \
	mailertcp.cpp \
	stringtools.cpp \
	argparser.cpp \
	base64.cpp \
	tinyxml/tinyxml.cpp \
	tinyxml/tinystr.cpp \
	tinyxml/tinyxmlparser.cpp \
	tinyxml/tinyxmlerror.cpp 
CFLAGS=-Wall -O2 -ggdb
INCLUDES=-I/usr/local/include -I/usr/local/include/mysql
LIBPATHS=-L/usr/local/lib -L/usr/local/lib/mysql
LIBS=-lmysqlpp -lmysqlclient -lstdc++
LDFLAGS=
OBJECTS=$(SOURCES:.cpp=.o)
OBJDIRS=obj obj/tinyxml
PROGRAM=$(.CURDIR)/../sqlmailer

.SUFFIXES: .o .cpp	 
.cpp.o: 
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LIBPATHS) $(LIBS) $(OBJECTS) -o $(PROGRAM)

$(OBJDIRS):
	mkdir -p $@

clean:
	rm -f $(PROGRAM)
.       for I in $(OBJECTS)
		rm -f $(.OBJDIR)/$I
.       endfor
