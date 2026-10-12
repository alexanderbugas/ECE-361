CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

PROG = readings
OBJS = main.o reading.o stats.o histogram.o
TEST = tests/test_stats

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Each .o depends on its .c and on every header that .c includes.
# If you add or remove an #include, update the matching line here.
main.o:      main.c reading.h stats.h histogram.h
reading.o:   reading.c reading.h
stats.o:     stats.c stats.h
histogram.o: histogram.c histogram.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): $(TEST).o stats.o
	$(CC) $(CFLAGS) -o $@ $^

$(TEST).o: $(TEST).c stats.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

test: $(PROG) $(TEST)
	./$(TEST)
	sh regress.sh

clean:
	rm -f $(PROG) $(OBJS) $(TEST) $(TEST).o

.PHONY: all test clean
