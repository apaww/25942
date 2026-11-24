CC = gcc

CFLAGS = -Wall

TARGET = lsld 

all: $(TARGET)

$(TARGET): lsld.c
	$(CC) $(CFLAGS) -o $(TARGET) lsld.c -lacl

clean:
	rm -f $(TARGET)
