CC = gcc
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags protobuf-c`
LDFLAGS = `pkg-config --libs protobuf-c`
SRC = tracking-solution.c tracked_object.pb-c.c
OUT = tracking-solution

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -f $(OUT) *.o *.pb-c.c *.pb-c.h

.PHONY: all clean
