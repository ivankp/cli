#include "cli.h"
#include <stdio.h>
#include <string.h>

#define LEN(ARR) (sizeof((ARR)) / sizeof(*(ARR)))

void Count(const char* value, void* cnt) {
    (void) value;
    ++*(unsigned*)cnt;
}

int main(int argc, const char* const* argv) {
    unsigned cats = 0, dogs = 0;

    CliOption optCat = { "c cat", &Count, &cats, "Increment the cat counter" };
    CliOption optDog = { "dog d", &Count, &dogs, "Increment the dog counter" };
    CliOption* opts[] = { &optCat, &optDog };
    CliParser parser = {
        CliPathName(argv[0]), LEN(opts), 0, opts, NULL,
        "Count cats and dogs", "", NULL
    };

    int ec = CliParse(&parser, argv + 1, argc - 1);
    if (ec != 0)
        return ec > 0;

    printf("%5u cats\n%5u dogs\n", cats, dogs);

    return 0;
}
