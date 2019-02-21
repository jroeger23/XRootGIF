#ifndef __OUTPUT_H_
#define __OUTPUT_H_

typedef enum {
        normal,     /* Normal output */
        warn,       /* Show warnings */
        verbose,    /* Maximum information */
        debug       /* Debug information */
} output_level_t;

typedef struct {
    output_level_t level;
} output_t;
extern output_t output;


/**
 * Print to stdout with level
 */
void sprint(char *str, output_level_t l);

/**
 * Print to stderr with level
 */
void eprint(char *str, output_level_t l);

/**
 * Print to stdout with level and append \n
 */
void sprintln(char *str, output_level_t l);

/**
 * Print to stderr with level and append \n
 */
void eprintln(char *str, output_level_t l);

/**
 * Printf to stdout with level
 */
void sformat(output_level_t l, char *str, ...);

/**
 * Printf to stderr with level
 */
void eformat(output_level_t l, char *str, ...);

#endif // __OUTPUT_H_
