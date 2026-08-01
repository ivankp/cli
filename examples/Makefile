.PHONY: all clean

EXAMPLES := 1-cats-dogs

all: $(EXAMPLES)

%: %.c ../src/cli.c
	gcc -std=c99 -Wall -Wextra -pedantic -O3 -I ../include $^ -o $@

clean:
	rm $(EXAMPLES)
