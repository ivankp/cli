.PHONY: all clean

EXAMPLES := 1-cats-dogs

all: $(EXAMPLES)

%: %.c ../src/cli.c ../include/cli.h
	gcc -std=c99 -Wall -Wextra -pedantic -O3 -I ../include $(filter %.c, $^) -o $@

clean:
	rm $(EXAMPLES)
