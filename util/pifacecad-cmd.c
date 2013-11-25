/**
 * @file  pifacecad-cmd.c
 * @brief Command line tool for PiFace Control and Display.
 *
 * Takes one to four positional arguments:
 * pifacedigital [cmd] {cmdarg0} {cmdarg1} {cmdarg2}
 * pifacedigital open blinkoff cursoroff
 * pifacedigital write "Hello, World"
 * pifacedigital backlight on
 * pifacedigital read switch
 *
 * Can also take optional flags:
 * pifacedigital --bit 1 read switch
 *
 * Or shorthand:
 * pifacedigital -b 1 read switch
 */
#include <stdlib.h>
#include <argp.h>
#include <strings.h>
#include <mcp23s17.h>
#include <pifacecad.h>


/**********************************************************************/
/* argp stuff from:
 * http://www.gnu.org/software/libc/manual/html_node/Argp.html#Argp
 */
const char *argp_program_version = "pifacecad v0.1.0";
const char *argp_program_bug_address = "<thomas.preston@openlx.org.uk>";

/* Program documentation. */
static char doc[] = "Command line tool for PiFace Control and Display.\n\n"
"Commands:\n\n"
"    open                     Set up the board (opt args: displayoff,\n"
"                             cursoroff, blinkoff)\n"
"    read                     Reads from a port (args: switch, switches)\n"
"    write                    Writes to the LCD.\n"
"    backlight on             Backlight 'on' or 'off'.\n"
"    home                     Set the cursor to the home position.\n"
"    clear                    Clear the screen.\n"
"    cursor                   Sets the cursor to the COL and ROW specified.\n\n"
"Example:\n\n"
"    $ pifacecad open blinkoff\n"
"    $ pifacecad write \"Hello, world!\"\n"
"    $ pifacecad backlight on\n"
"    $ pifacecad setcursor 7 1\n";

/* A description of the arguments we accept. */
static char args_doc[] = "CMD CMDARG0 CMDARG1 CMDARG2";

/* The options we understand. */
static struct argp_option options[] = {
    {"bit-num", 'b', "BITNUM", 0, "Bit number to read/write to." },
    { 0 },
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
    char * cmd;
    char * cmdargs[3];
    int bit_num;
};

/* Parse a single option. */
static error_t parse_opt(int key, char * arg, struct argp_state * state)
{
    /* Get the input argument from argp_parse, which we
      know is a pointer to our arguments structure. */
    struct arguments * arguments = state->input;

    switch (key)
    {
    case 'b':
        arguments->bit_num = atoi(arg);
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num >= 4) {
            argp_usage(state); /* Too many arguments. */
        } else if (state->arg_num == 0) {
            arguments->cmd = arg;
        } else if (state->arg_num == 1) {
            arguments->cmdargs[0] = arg;
        } else if (state->arg_num == 2) {
            arguments->cmdargs[1] = arg;
        } else if (state->arg_num == 3) {
            arguments->cmdargs[2] = arg;
        }
        break;

    case ARGP_KEY_END:
        if (state->arg_num < 1)
             /* Not enough arguments. */
             argp_usage (state);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};
/**********************************************************************/

uint8_t str2reg(char * reg_str);
void pfc_read_switch(int bit_num, uint8_t reg);


int main(int argc, char **argv)
{
    struct arguments arguments;

    // Default values
    arguments.cmd = NULL;
    arguments.cmdargs[0] = NULL;
    arguments.cmdargs[1] = NULL;
    arguments.cmdargs[2] = NULL;
    arguments.bit_num = -1;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // printf("cmd = %s\ncmdargs[0] = %s\ncmdargs[1] = %s\n"
    //        "cmdargs[2] = %s\nbit_num = %d\n",
    //        arguments.cmd,
    //        arguments.cmdargs[0],
    //        arguments.cmdargs[1],
    //        arguments.cmdargs[2],
    //        arguments.bit_num);

    if (arguments.bit_num > 7) {
        fprintf(stderr, "pifacecad: bit num must in range 0-7.\n");
        exit(1);
    }

    pifacecad_open_noinit();

    if (strcmp(arguments.cmd, "open") == 0) {
        pifacecad_open();
        int i;
        for (i = 0; i <= 2; i++) {
            if (arguments.cmdargs[i] == NULL) {
                continue;
            }
            if (strcmp(arguments.cmdargs[i], "displayoff") == 0) {
                pifacecad_lcd_display_off();
            }
            if (strcmp(arguments.cmdargs[i], "blinkoff") == 0) {
                pifacecad_lcd_blink_off();
            }
            if (strcmp(arguments.cmdargs[i], "cursoroff") == 0) {
                pifacecad_lcd_cursor_off();
            }
        }

    } else if (strcmp(arguments.cmd, "read") == 0) {
        pfc_read_switch(arguments.bit_num, str2reg(arguments.cmdargs[0]));

    } else if (strcmp(arguments.cmd, "write") == 0) {
        pifacecad_lcd_write(arguments.cmdargs[0]);

    } else if (strcmp(arguments.cmd, "backlight") == 0) {
        if (strcmp(arguments.cmdargs[0], "on") == 0) {
            pifacecad_lcd_backlight_on();
        } else {
            pifacecad_lcd_backlight_off();
        }

    } else if (strcmp(arguments.cmd, "home") == 0) {
        pifacecad_lcd_home();

    } else if (strcmp(arguments.cmd, "clear") == 0) {
        pifacecad_lcd_clear();

    } else if (strcmp(arguments.cmd, "setcursor") == 0) {
        const uint8_t col = atoi(arguments.cmdargs[0]);
        const uint8_t row = atoi(arguments.cmdargs[1]);
        pifacecad_lcd_set_cursor(col, row);
    }

    pifacecad_close();

    exit(0);
}

uint8_t str2reg(char * reg_str)
{
    // convert to lower case
    int i;
    for(i = 0; reg_str[i]; i++) {
        reg_str[i] = tolower(reg_str[i]);
    }

    // get the real value (TODO add some more registers here)
    if (strcmp(reg_str, "switch") == 0 || \
            strcmp(reg_str, "switches") == 0 || \
            strcmp(reg_str, "input") == 0 || \
            strcmp(reg_str, "gpioa") == 0) {
        return GPIOA;
    } else {
        fprintf(stderr, "pifacecad: no such register '%s'\n", reg_str);
        exit(1);
    }
}

void pfc_read_switch(int bit_num, uint8_t reg)
{
    uint8_t value;
    if (bit_num >= 0) {
        value = pifacecad_read_switch(bit_num);
    } else {
        value = pifacecad_read_switches();
    }
    printf("%d\n", value);
}
