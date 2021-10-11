#build executable for telecommand interpreter project
CC = gcc
CFLAGS = -g -Wall

TARGET = telecmdAppl

all: $(TARGET)

$(TARGET): main.c telecmd_interpreter.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c telecmd_interpreter.c

clean:
	rm $(TARGET)
