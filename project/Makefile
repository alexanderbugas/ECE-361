CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
SAN    = -fsanitize=address,undefined
# -lm: the math library, for floorf in the store's key
LDLIBS = -lm

PROG = vent
OBJS = main.o sensor.o fsm.o store.o aux.o report.o bits.o iom361_r4.o

TEST      = tests/test_all
TEST_SRCS = tests/test_all.c tests/check.c tests/test_fsm.c tests/test_bits.c \
            tests/test_store.c fsm.c bits.c store.c
TEST_HDRS = tests/check.h fsm.h bits.h store.h sensor.h

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Each .o depends on its .c and on every header that .c includes.
# If you add or remove an #include, update the matching line here.
main.o:      main.c iom361_r4.h
sensor.o:    sensor.c sensor.h
fsm.o:       fsm.c fsm.h bits.h sensor.h
store.o:     store.c store.h sensor.h fsm.h
aux.o:       aux.c aux.h
report.o:    report.c report.h
bits.o:      bits.c bits.h
iom361_r4.o: iom361_r4.c iom361_r4.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# The tests are built from the sources in one step, with the sanitizers the
# sheet requires, so they never share an object with vent. -I. lets the
# files in tests/ find the headers one folder up.
$(TEST): $(TEST_SRCS) $(TEST_HDRS)
	$(CC) $(CFLAGS) $(SAN) -I. -o $@ $(TEST_SRCS) $(LDLIBS)

# make test builds and runs every test; it fails if any test fails
test: $(TEST)
	./$(TEST)

clean:
	rm -f $(PROG) $(OBJS) $(TEST)

.PHONY: all test clean
