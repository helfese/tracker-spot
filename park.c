#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define FL float
#define MLOC malloc(sizeof(char) * BUFSIZ)
#define NQ not_q_cmd(cmd)
#define FALSE 0
#define TRUE 1
#define BASE 0
#define FRST 1
#define SCND 2
#define LST_PAIR 6
#define DAY 0
#define MON 1
#define YRS 2
#define HRS 3
#define MIN 4
#define JAN 1
#define FEB 2
#define APR 4
#define JUN 6
#define SEP 9
#define NOV 11
#define DEC 12
#define STR_CDEF '"'
#define STR_SDEF "\""
#define DT_DIV '-'
#define HR_DIV ':'
#define DND 2
#define Q_CMD "q"
#define P_CMD "p"
#define E_CMD "e"
#define S_CMD "s"
#define V_CMD "v"
#define F_CMD "f"
#define R_CMD "r"
#define CMD_SIZE 2
#define PLTP_SIZE 3
#define LSTP 6
#define BLNK_STR " "
#define BLNK_CHR ' '
#define NULL_STR "0"
#define NINT 0
#define DAYS_FEB 28
#define DAYS_SHORT_MO 30
#define DAYS_LONG_MO 31
#define STR_INIT 1
#define STR_END '\0'
#define PARK_MAX 20
#define NBASE 10
#define VALID 0
#define INVALID 1
#define REGS_MAX 10000
#define CARS_MAX 5000
#define ERROR_SFORMAT "%s: %s"
#define ERROR_DFORMAT "%d: %s"
#define ERROR_PARK_EXIST ": parking already exists.\n"
#define ERROR_SPOTS_OUT "invalid capacity.\n"
#define ERROR_PARK_FULL "parking is full.\n"
#define ERROR_COST_INVALID "invalid cost.\n"
#define ERROR_DATE_INVALID "invalid date.\n"
#define ERROR_PARKS_OUT "too many parks.\n"
#define ERROR_NO_PARK "no such parking.\n"
#define ERROR_YES_ENTRY "invalid vehicle entry.\n"
#define ERROR_NO_ENTRY "invalid vehicle exit.\n"
#define ERROR_PLATE_INVALID "invalid licence plate.\n"
#define ERROR_ANY_ENTRY ": no entries found in any parking.\n"

static int comma_count = NINT; // Number of commas
static int i_parks = NINT; // Number of parks
static int park_size = PARK_MAX; // Maximum park size
static int reglen = REGS_MAX; // Maximum length of registration
static int carlen = CARS_MAX; // Maximum number of cars
static size_t tok_len = NINT; // Length of token
static int had_error = NINT; // Error flag
static int bill_id = -1; // Bill ID
static int addpark = 0; // Flag for adding park
static int has_pair_upp = 0; // Flag for uppercase pair
static int has_pair_dig = 0; // Flag for digit pair
static int park_id = -1; // Park ID

typedef struct Bill {
    char *plt;
    char *dtt;
    char *hrs;
    float vlr;
} bill;

typedef struct parks {
    char *name;
    int spots;
    float val_15;
    float val_15_1h;
    float val_max_day;
    int free_spots;

    char ***entry;
    bill *bills;

} *park;

park *parks; // Define pointers to park structures
char **cars; // Define array of strings for cars
// Define pointer to struct parks for query purposes
struct parks *qparks;
int live[5]; // Define array of integers for the live status

/**
   Truncates the given string by appending a null terminator at the end.
   @param str The string to be truncated
   @return The truncated string
*/
char *str_end(char *str) {
    size_t i;
    for (i = BASE; i < strlen(str); i++);
    str[i] = STR_END;
    return str;
}

/**
   Deletes leading and trailing blanks from a string.
   @param str The input string
   @return The modified string with leading and trailing blanks removed
*/
char *del_blank(char *str) {
    char *ptr = str;
    int prev_blank, i, j, null;
    prev_blank = i = j = null = BASE;

    // Reset comma count
    comma_count = BASE;

    // Iterate through the string
    while (*ptr) {
        // Count commas
        if (*ptr == STR_CDEF)
            comma_count++;

        // Process characters except within quoted parts
        if (comma_count != FRST) {
            // Remove leading blanks
            if (isspace(*ptr))
                if (!prev_blank)
                    str[prev_blank = i++] = BLNK_CHR;

            // Copy non-blank characters
            if (!isspace(*ptr) && (prev_blank = null) == BASE)
                str[i++] = *ptr;
        } else {
            // Copy characters within quoted parts
            str[i++] = *ptr;
        }
        ptr++;
    }

    // Terminate the string
    str[i] = STR_END;

    // Check if the resulting string is empty
    if (str[j] == BLNK_CHR && str[j + FRST] == STR_END)
        return NULL_STR;
    else if (str[j++] == BLNK_CHR && str[j++] == BLNK_CHR && str[j] == STR_END)
        return NULL_STR;

    // Return the modified string
    return str;
}

/**
   Reads a line from standard input into the provided buffer and returns it.
   @param args Buffer to store the input line
   @return The input line read from stdin
*/
char *get(char *args) {
    char *input = str_end(fgets(args, BUFSIZ, stdin));
    return input;
}

/**
   Checks if the command is not equal to the quit command.
   @param cmd The command to check
   @return Returns false if the command is the quit command, otherwise true
*/
int not_q_cmd(char *cmd) {
    if (!strcmp(cmd, NULL_STR))
        return FALSE;
    return strcmp(cmd, Q_CMD);
}

/**
   Checks if the given token is null and updates its length if not.
   @param tok The token to check
*/
void ck_tok_null(char* tok) {
    if (tok == NULL)
        tok_len = BASE;
    else
        tok_len = strlen(tok);
}

/**
   Gets the token from the given string until a closing quotation mark or space.
   @param str The input string
   @return The extracted token
*/
char* get_tok(char *str) {
    size_t i, len = strlen(str);
    tok_len = BASE;
    char* token;

    // Find the position of the closing quotation mark
    for (i = BASE; i < len; i++)
        if (str[i] == STR_CDEF)
            break;
    i++; // Move to the character after the closing quotation mark

    // If comma count is non-zero, extract token using strtok
    if (comma_count) {
        while(i--)
            str++;
        ck_tok_null(token = strtok(str, STR_SDEF));
        return token;
    }

    // Otherwise, extract token using strtok
    ck_tok_null(token = strtok(NULL, BLNK_STR));
    return token;
}

/**
   Checks if an error has occurred and resets the error flag.
   @return Returns true if an error has occurred, otherwise false
*/
int ckreset_error() {
    if (had_error) {
        had_error = FALSE;
        return TRUE;
    }
    return FALSE;
}

/**
   Checks if a parking spot with the given name exists.
   @param name The name of the parking spot to search for
   @return Returns true if the parking spot exists, otherwise false
*/
int park_exist(char *name) {
    int i = BASE;
    while (i < park_size) {
        if (parks[i] != NULL) {
            if (!strcmp(parks[i]->name, name)) {
                park_id = i;
                return TRUE;
            }
        }
        i++;
    }
    return FALSE;
}

/**
   Checks if a parking spot is unavailable and prints an error message.
   @param spot The parking spot to check
   @return Returns true if the spot is unavailable, otherwise false
*/
int ckspot_out(int spot) {
    if (spot > BASE)
        return FALSE;
    
    had_error = TRUE;
    printf(ERROR_DFORMAT, spot, ERROR_SPOTS_OUT);
    return TRUE;
}

/**
   Checks if the given cost values are invalid and prints an error message.
   @param v15 The cost for 15 minutes
   @param v1h15 The cost for 1 hour and 15 minutes
   @param vmax The maximum cost
   @return Returns true if the cost values are invalid, otherwise false
*/
int ckcost_out(float v15, float v1h15, float vmax) {
    if (BASE < v15 && v15 < v1h15 && v1h15 < vmax)
        return FALSE;
    
    had_error = TRUE;
    printf(ERROR_COST_INVALID);
    return TRUE;
}

/**
   Checks if there are available spots and prints an error message if not.
   @return Returns false if there are available spots, otherwise true
*/
int ckpark_out() {
    int len = park_size, count_free = BASE;
    
    // Count the number of free parking spots
    while (len--)
        if (parks[len] == NULL)
            count_free++;

    // If there are free parking spots, return false
    if (count_free)
        return FALSE;

    // If all parking spots are occupied, set error flag and print error message
    had_error = TRUE;
    printf(ERROR_PARKS_OUT);
    return TRUE;
}

/**
   Checks if there are spots in a parking and prints an error message if not.
   @param name The name of the parking
   @return Returns VALID if there are available spots, otherwise INVALID
*/
int ckpark_full(char *name) {
    for (int i = NINT; i < park_size; i++) {
        if (parks[i] != NULL) {
            if (!strcmp(parks[i]->name, name)) {
                if (parks[i]->free_spots)
                    return FALSE;
            }
        }
    }
    
    // If all spots are occupied, set error flag and print error message
    had_error = TRUE;
    printf(ERROR_SFORMAT, name, ERROR_PARK_FULL);
    return TRUE;
}

/**
   Checks if they are uppercases and updates the count of their pairs.
   @param c1 The first character
   @param c2 The second character
   @return Returns true if both characters are uppercase, otherwise false
*/
int areuppers(char c1, char c2) {
    int test = isupper(c1) && isupper(c2);
    has_pair_upp += test;
    return test;
}

/**
   Checks if both characters are digits and updates the count of digit pairs.
   @param c1 The first character
   @param c2 The second character
   @return Returns true if both characters are digits, otherwise false
*/
int aredigits(char c1, char c2) {
    int test = isdigit(c1) && isdigit(c2);
    has_pair_dig += test;
    return test;
}

/**
   Checks if at least both uppercase and digit pairs exist.
   @param i The integer to check
   @return Returns true if the pairs exist until the last, otherwise false
*/
int hasUpDg(int i) {
    return (has_pair_upp * has_pair_dig > FALSE && i == LST_PAIR);;
}

/**
   Checks if both characters are uppercase letters or digits.
   @param c1 The first character
   @param c2 The second character
   @return Returns true if both are uppercases or digits, otherwise false
*/
int uppdig(char c1, char c2) {
    int test = (areuppers(c1, c2) || aredigits(c1, c2));
    return test;
}

/**
   Checks if the given character is the end of a string.
   @param c1 The character to check
   @return Returns true if the character is the end of a string, otherwise false
*/
int end(char c1) {
    return c1 == STR_END;
}

/**
   Checks if the given license plate is invalid and prints an error if so.
   @param pl The license plate to check
   @return Returns false if the license plate is valid, otherwise true
*/
int ckplate_valid(char *pl) {
    int i = BASE;
    has_pair_upp = BASE;
    has_pair_dig = BASE;
    do {
        if (pl[i + 2] == '-' && aredigits(pl[i], pl[i + 1]) &&
            pl[i + 5] == '-' && aredigits(pl[i + 3], pl[i + 4]) &&
            end(pl[i + 8]) && aredigits(pl[i + 6], pl[i + 7]))
             return FALSE;
        else if (pl[i + SCND] == DT_DIV && areuppers(pl[i], pl[i + FRST]));
        else if (pl[i + SCND] == DT_DIV && aredigits(pl[i], pl[i + FRST]));
        else {
            if (uppdig(pl[i], pl[i + FRST]) && end(pl[i + SCND]) && hasUpDg(i));
            else {
                had_error = TRUE;
                printf(ERROR_SFORMAT, pl, ERROR_PLATE_INVALID);
                return TRUE;
            }
        }
        i = i + PLTP_SIZE;
    } while(i <= LST_PAIR);
    return FALSE;
}

// p parque 5 0.30 0.50 15.00
// e parque 00-00-00 01-01-2024 8:02

/**
   Checks if the given day is valid for the specified month.
   @param day The day
   @param mon The month
   @return Returns true if the day is valid for the month, otherwise false
*/
int day_valid(int day, int mon) {
    if (BASE < day && JAN <= mon && mon <= DEC) {
        if (mon == APR || mon == JUN || mon == SEP || mon == NOV)
            return day <= DAYS_SHORT_MO;
        else if (mon == FEB)
            return day <= DAYS_FEB;
        else
            return day <= DAYS_LONG_MO;
    }
    return FALSE;
}

/**
   Parses the hour string and extracts the hour and minute components.
   @param hour The hour string in "hh:mm" format
   @param hrs Pointer to store the hour component
   @param min Pointer to store the minute component
*/
void parse_hour(char *hour, int *hrs, int *min) {
    sscanf(hour, "%d:%d", hrs, min);
}

/**
   Parses the date string and extracts the day, month, and year components.
   @param date The date string in "dd-mm-yyyy" format
   @param day Pointer to store the day component
   @param mon Pointer to store the month component
   @param yrs Pointer to store the year component
*/
void parse_date(char *date, int *day, int *mon, int *yrs) {
    sscanf(date, "%d-%d-%d", day, mon, yrs);
}
