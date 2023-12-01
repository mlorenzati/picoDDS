#ifndef CMD_PARSER
#define CMD_PARSER

#define TRUE  1 
#define FALSE 0 

typedef struct 
{
    const char *name;      /* the name of the long option */
    char        has_arg;   /* Has a following argument */
    char       *strval;    /* If has argument, return here the string */
    const char  val;	   /* short option */
} option;

void get_argv_argc(char *str, int *argc, char *argv[]);
int  get_cmd (int argc, char *const argv[], option * options, int *option_index, int *arg_index);
void print_cmds(const option *options);
#endif