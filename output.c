#include "output.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

output_t output = { .level = verbose };

#define PREFIX(FD, L) write(FD, prefixes[L], strlen(prefixes[L]));

static const char* prefixes[] = {
        "",     /* Normal */
        "[W] ",  /* Warn */
        "[V] ",  /* Verbose */
        "[D] "   /* Debug */
};

void sprint(char *str, output_level_t l)
{
        if(output.level < l) return;

        PREFIX(STDOUT_FILENO, l);
        write(STDOUT_FILENO, str, strlen(str));
}

void eprint(char *str, output_level_t l)
{
        if(output.level < l) return;

        PREFIX(STDERR_FILENO, l);
        write(STDERR_FILENO, str, strlen(str));
}

void sprintln(char *str, output_level_t l)
{
        if(output.level < l) return;

        sprint(str, l);
        write(STDOUT_FILENO, "\n", 1);
}

void eprintln(char *str, output_level_t l)
{
        if(output.level < l) return;

        eprint(str, l);
        write(STDERR_FILENO, "\n", 1);
}

void sformat(output_level_t l, char *str, ...)
{
        if(output.level < l) return;

        va_list va;
        va_start(va, str);
        PREFIX(STDOUT_FILENO, l);
        vprintf(str, va);
        va_end(va);
}

void eformat(output_level_t l, char *str, ...)
{
        if(output.level < l) return;

        va_list va;
        va_start(va, str);
        PREFIX(STDERR_FILENO, l);
        vfprintf(stderr, str, va);
        va_end(va);
}
