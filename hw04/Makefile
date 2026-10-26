CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

PROG = events
TEST = tests/test_tally

# make builds the events program
all: $(PROG)

# events is linked from its own object and the tally library
$(PROG): events.o tally.o
	$(CC) $(CFLAGS) -o $@ $^

# Each .o depends on its .c and on every header that .c includes, so that
# changing tally.h rebuilds everything that uses it.
# If you add or remove an #include, update the matching line here.
events.o: events.c tally.h
tally.o: tally.c tally.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# the test program is linked from its own object and the same tally.o
$(TEST): $(TEST).o tally.o
	$(CC) $(CFLAGS) -o $@ $^

# -I. lets tests/test_tally.c find tally.h one folder up
$(TEST).o: $(TEST).c tally.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

# make test builds the tests and runs them; it fails if any test fails
test: $(TEST)
	./$(TEST)

# make clean removes everything make built
clean:
	rm -f events.o tally.o $(PROG) $(TEST).o $(TEST)

.PHONY: all test clean
