CC=g++
CPPFLAGS=-I/usr/include/mysql
LDFLAGS=
LDLIBS=-lpthread -lmysqlclient
OBJS=iot_server.o
TARGET=iot_server
 
all: $(TARGET)
 
clean:
	rm -f *.o
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS) $(LDLIBS)

