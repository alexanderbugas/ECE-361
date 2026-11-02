CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

PROG = lock
TEST = tests/test_lock
BONUS = tests/test_check

# make builds the lock program
all: $(PROG)

# lock is linked from main and the lock table and functions
$(PROG): main.o lock.o
	$(CC) $(CFLAGS) -o $@ $^

# Each .o depends on its .c and on every header that .c includes, so that
# changing lock.h rebuilds everything that uses it.
# If you add or remove an #include, update the matching line here.
main.o: main.c lock.h
lock.o: lock.c lock.h
check.o: check.c check.h lock.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# the test program is linked from its own object and the same lock.o
$(TEST): $(TEST).o lock.o
	$(CC) $(CFLAGS) -o $@ $^

# -I. lets tests/test_lock.c find lock.h one folder up
$(TEST).o: $(TEST).c lock.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

# make test builds the tests and runs them; it fails if any test fails
test: $(TEST)
	./$(TEST)

# the bonus test program is linked from its own object, check.o and lock.o
$(BONUS): $(BONUS).o check.o lock.o
	$(CC) $(CFLAGS) -o $@ $^

# -I. lets tests/test_check.c find check.h and lock.h one folder up
$(BONUS).o: $(BONUS).c check.h lock.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

# make bonus builds the checker tests and runs them; it fails if any test fails
bonus: $(BONUS)
	./$(BONUS)

# make clean removes everything make built
clean:
	rm -f main.o lock.o check.o $(PROG) $(TEST).o $(TEST) $(BONUS).o $(BONUS)

.PHONY: all test bonus clean
