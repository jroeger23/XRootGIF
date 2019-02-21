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


void sprint(char *str, output_level_t l);
void eprint(char *str, output_level_t l);
void sprintln(char *str, output_level_t l);
void eprintln(char *str, output_level_t l);
void sformat(output_level_t l, char *str, ...);
void eformat(output_level_t l, char *str, ...);

#endif // __OUTPUT_H_
