CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

PROG = vent
OBJS = main.o sensor.o fsm.o store.o aux.o report.o iom361_r4.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Each .o depends on its .c and on every header that .c includes.
# If you add or remove an #include, update the matching line here.
main.o:      main.c iom361_r4.h
sensor.o:    sensor.c sensor.h
fsm.o:       fsm.c fsm.h
store.o:     store.c store.h
aux.o:       aux.c aux.h
report.o:    report.c report.h
iom361_r4.o: iom361_r4.c iom361_r4.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG) $(OBJS)

.PHONY: all clean
