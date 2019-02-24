#ifndef __OUTPUT_H_
#define __OUTPUT_H_

typedef enum {
        normal  = 0,    /* Normal output */
        warn    = 1,    /* Show warnings */
        verbose = 2,    /* Maximum information */
        debug   = 3     /* Debug information */
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
