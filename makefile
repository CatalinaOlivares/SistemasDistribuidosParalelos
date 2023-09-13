CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

TARGET = dilation

all: $(TARGET)

$(TARGET): dilation.c
	$(CC) $(CFLAGS) -o $(TARGET) dilation.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
