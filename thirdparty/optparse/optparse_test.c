#define OPTPARSE_IMPLEMENTATION
#include "optparse.h"

#include <shell.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_argv(char **argv)
{
    while (*argv)
        os_kprintf("%s ", *argv++);
    os_kprintf("\r\n");
}

void try_optparse(char **argv)
{
    int opt;
    char *arg;
    struct optparse options;

    print_argv(argv);
    optparse_init(&options, argv);
    while ((opt = optparse(&options, "abc:d::")) != -1) {
        if (opt == '?')
            os_kprintf("%s: %s\r\n", argv[0], options.errmsg);
        os_kprintf("%c (%d) = '%s'\r\n", opt, options.optind, options.optarg);
    }
    os_kprintf("optind = %d\r\n", options.optind);
    while ((arg = optparse_arg(&options)))
        os_kprintf("argument: %s\r\n", arg);
}

void try_optparse_long(char **argv)
{
    char *arg;
    int opt, longindex;
    struct optparse options;
    struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_REQUIRED},
        {"delay", 'd', OPTPARSE_OPTIONAL},
        {"erase", 256, OPTPARSE_REQUIRED},
        {"set",   0,   OPTPARSE_NONE}
    };

    print_argv(argv);
    optparse_init(&options, argv);
    while ((opt = optparse_long(&options, longopts, &longindex)) != -1) {
        char buf[2] = {0, 0};
        if (opt == '?')
            os_kprintf("%s: %s\r\n", argv[0], options.errmsg);
        buf[0] = opt;
        os_kprintf("%-6s(%d, %d) = '%s'\r\n",
               opt < 127 ? buf : longopts[longindex].longname,
               options.optind, longindex, options.optarg);
    }
    os_kprintf("optind = %d\r\n", options.optind);
    while ((arg = optparse_arg(&options)))
        os_kprintf("argument: %s\r\n", arg);
}

int optparse_test(int argc, char **argv)
{
    char *long_argv[] = {
        "./main", "--amend", "-b", "--color", "red", "--delay=22",
        "subcommand", "example.txt", "--amend", "--erase", "all", NULL
    };
    size_t size = (argc + 1) * sizeof(*argv);
    char **argv_copy = malloc(size);

    memcpy(argv_copy, argv, size);
    os_kprintf("\r\nOPTPARSE\r\n");
    try_optparse(argv_copy);

    os_kprintf("\r\nOPTPARSE LONG\r\n");
    try_optparse_long(long_argv);
    return 0;
}

SH_CMD_EXPORT(optparse_test, optparse_test, "start optparse test demo");
