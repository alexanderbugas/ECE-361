CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
SAN    = -fsanitize=address,undefined

PROG = replay
OBJS = main.o sensor.o panel.o scenario.o iom361_r4.o

TEST      = tests/test_hw06
TEST_SRCS = tests/test_hw06.c sensor.c panel.c scenario.c iom361_r4.c
TEST_HDRS = sensor.h panel.h scenario.h iom361_r4.h

# make builds replay from all the modules and iom361
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Each .o depends on its .c and on every header that .c includes, so that
# changing a header rebuilds everything that uses it.
# If you add or remove an #include, update the matching line here.
main.o:      main.c sensor.h panel.h scenario.h iom361_r4.h
sensor.o:    sensor.c sensor.h iom361_r4.h
panel.o:     panel.c panel.h iom361_r4.h
scenario.o:  scenario.c scenario.h
iom361_r4.o: iom361_r4.c iom361_r4.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# The unit tests are built from the sources in one step, with the
# sanitizers, so they never share an object with replay. -I. lets
# tests/test_hw06.c find the headers one folder up.
$(TEST): $(TEST_SRCS) $(TEST_HDRS)
	$(CC) $(CFLAGS) $(SAN) -I. -o $@ $(TEST_SRCS)

# make test runs the unit tests, then the regression test; it fails if
# either fails
test: $(TEST) $(PROG)
	./$(TEST)
	sh regress.sh

# make clean removes everything make built
clean:
	rm -f $(PROG) $(OBJS) $(TEST)

.PHONY: all test clean
