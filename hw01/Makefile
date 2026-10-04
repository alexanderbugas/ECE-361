CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

OBJS = bits.o status.o
TEST = tests/test_bits

all: $(OBJS)

%.o: %.c bits.h status.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): $(TEST).o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST).o: $(TEST).c bits.h status.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

test: $(TEST)
	./$(TEST)

clean:
	rm -f *.o $(TEST) $(TEST).o

.PHONY: all test clean
