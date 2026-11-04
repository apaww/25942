CC = gcc

CFLAGS = -Wall

TARGET = test

all: $(TARGET)

$(TARGET): proc.c
	$(CC) $(CFLAGS) -o $(TARGET) proc.c

clean:
	rm -f $(TARGET)
