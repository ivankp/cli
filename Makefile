all: test

test: test.c cli.c cli.h
	gcc -Wall -Wextra -pedantic -Werror -fmax-errors=3 -O3 $(filter %.c, $^) -o $@
