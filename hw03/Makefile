CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

TEST = tests/test_ptr

# make builds the library object and the test program
all: ptr.o $(TEST)

# Each .o depends on its .c and on every header that .c includes.
# If you add or remove an #include, update the matching line here.
ptr.o: ptr.c ptr.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): $(TEST).o ptr.o
	$(CC) $(CFLAGS) -o $@ $^

# -I. lets tests/test_ptr.c find ptr.h one folder up
$(TEST).o: $(TEST).c ptr.h
	$(CC) $(CFLAGS) -I. -c $< -o $@

test: $(TEST)
	./$(TEST)

clean:
	rm -f ptr.o $(TEST) $(TEST).o

.PHONY: all test clean
