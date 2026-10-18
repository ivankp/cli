.PHONY: all clean

EXAMPLES := 1-cats-dogs 2-abcd

CFLAGS := -std=c99 -Wall -Wextra -pedantic -Wno-missing-field-initializers -O3

all: $(EXAMPLES)

%: %.c ../src/cli.c ../include/cli.h
	gcc $(CFLAGS) -I ../include $(filter %.c, $^) -o $@

clean:
	rm $(EXAMPLES)
