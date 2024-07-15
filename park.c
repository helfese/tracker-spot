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

struct parks *qparks;
int live[5]; // Define array of integers for the live status

char *str_end(char *str) {
    size_t i;
    for (i = BASE; i < strlen(str); i++);
    str[i] = STR_END;
    return str;
}

char *del_blank(char *str) {
    char *ptr = str;
    int prev_blank, i, j, null;
    prev_blank = i = j = null = BASE;

    comma_count = BASE;

    while (*ptr) {
        if (*ptr == STR_CDEF)
            comma_count++;

        if (comma_count != FRST) {
            // Remove leading blanks
            if (isspace(*ptr))
                if (!prev_blank)
                    str[prev_blank = i++] = BLNK_CHR;

            if (!isspace(*ptr) && (prev_blank = null) == BASE)
                str[i++] = *ptr;
        } else {
            // Copy characters within quoted parts
            str[i++] = *ptr;
        }
        ptr++;
    }

    str[i] = STR_END;

    if (str[j] == BLNK_CHR && str[j + FRST] == STR_END)
        return NULL_STR;
    else if (str[j++] == BLNK_CHR && str[j++] == BLNK_CHR && str[j] == STR_END)
        return NULL_STR;

    // Return the modified string
    return str;
}

char *get(char *args) {
    char *input = str_end(fgets(args, BUFSIZ, stdin));
    return input;
}

int not_q_cmd(char *cmd) {
    if (!strcmp(cmd, NULL_STR))
        return FALSE;
    return strcmp(cmd, Q_CMD);
}

void ck_tok_null(char* tok) {
    if (tok == NULL)
        tok_len = BASE;
    else
        tok_len = strlen(tok);
}

char* get_tok(char *str) {
    size_t i, len = strlen(str);
    tok_len = BASE;
    char* token;

    for (i = BASE; i < len; i++)
        if (str[i] == STR_CDEF)
            break;
    i++;

    if (comma_count) {
        while(i--)
            str++;
        ck_tok_null(token = strtok(str, STR_SDEF));
        return token;
    }

    ck_tok_null(token = strtok(NULL, BLNK_STR));
    return token;
}

int ckreset_error() {
    if (had_error) {
        had_error = FALSE;
        return TRUE;
    }
    return FALSE;
}

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

int ckspot_out(int spot) {
    if (spot > BASE)
        return FALSE;
    
    had_error = TRUE;
    printf(ERROR_DFORMAT, spot, ERROR_SPOTS_OUT);
    return TRUE;
}

int ckcost_out(float v15, float v1h15, float vmax) {
    if (BASE < v15 && v15 < v1h15 && v1h15 < vmax)
        return FALSE;
    
    had_error = TRUE;
    printf(ERROR_COST_INVALID);
    return TRUE;
}

int ckpark_out() {
    int len = park_size, count_free = BASE;
    
    // Count the number of free parking spots
    while (len--)
        if (parks[len] == NULL)
            count_free++;

    if (count_free)
        return FALSE;

    had_error = TRUE;
    printf(ERROR_PARKS_OUT);
    return TRUE;
}

int ckpark_full(char *name) {
    for (int i = NINT; i < park_size; i++) {
        if (parks[i] != NULL) {
            if (!strcmp(parks[i]->name, name)) {
                if (parks[i]->free_spots)
                    return FALSE;
            }
        }
    }

    had_error = TRUE;
    printf(ERROR_SFORMAT, name, ERROR_PARK_FULL);
    return TRUE;
}

int areuppers(char c1, char c2) {
    int test = isupper(c1) && isupper(c2);
    has_pair_upp += test;
    return test;
}

int aredigits(char c1, char c2) {
    int test = isdigit(c1) && isdigit(c2);
    has_pair_dig += test;
    return test;
}

int hasUpDg(int i) {
    return (has_pair_upp * has_pair_dig > FALSE && i == LST_PAIR);;
}

int uppdig(char c1, char c2) {
    int test = (areuppers(c1, c2) || aredigits(c1, c2));
    return test;
}

int end(char c1) {
    return c1 == STR_END;
}

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

void parse_hour(char *hour, int *hrs, int *min) {
    sscanf(hour, "%d:%d", hrs, min);
}

void parse_date(char *date, int *day, int *mon, int *yrs) {
    sscanf(date, "%d-%d-%d", day, mon, yrs);
}

void parse_dh(char *date, char *hour, int *d, int *m, int *y, int *H, int *M) {
    parse_date(date, d, m, y);
    parse_hour(hour, H, M);
}

void to_live_data(char *date, char *hour) {
    int day, mon, yrs, hrs, min, i = 0;
    parse_dh(date, hour, &day, &mon, &yrs, &hrs, &min);
    live[i++] = day;
    live[i++] = mon;
    live[i++] = yrs;
    live[i++] = hrs;
    live[i] = min;
}

int is_live_data(int day, int mon, int yrs, int hrs, int min) {
    if (yrs > live[YRS])
        return TRUE;
    else if (yrs == live[YRS]) {
        if (mon > live[MON])
            return TRUE;
        else if (mon == live[MON]) {
            if (day > live[DAY])
                return TRUE;
            else if (day == live[DAY]) {
                if (hrs > live[HRS])
                    return TRUE;
                else if (hrs == live[HRS])
                    if (min >= live[MIN])
                        return TRUE;
            }
        }
    }
    return FALSE;
}

int isValidDateFormat(const char *str) {
    int len = strlen(str);
    if (len != 10)
        return FALSE;
    for (int i = BASE; i < len; i++) {
        if (i == 2 || i == 5) {
            if (str[i] != DT_DIV)
                return FALSE;
        }
        else if (!isdigit(str[i]))
                return FALSE;
    }
    return TRUE;
}

int isValidTimeFormat(const char *str) {
    int len = strlen(str);
    if (len != 4 && len != 5)
        return FALSE;
    
    for (int i = BASE; i < len; i++) {
        if ((len == 4 && i == 1) || (len == 5 && i == 2)) {
            if (str[i] != HR_DIV)
                return FALSE;
        } 
        else if (!isdigit(str[i]))
            return FALSE;
    }
    return TRUE;
}

int isFormatDH(char *date, char *hour) {
    int dayf, monf, yrsf, hrsf, minf;
    if (isValidDateFormat(date) && isValidTimeFormat(hour)) {
        parse_dh(date, hour, &dayf, &monf, &yrsf, &hrsf, &minf);
        if (0 <= hrsf && hrsf <= 23 && 0 <= minf && minf <= 59)
            if (1000 <= yrsf && yrsf <= 9999 && day_valid(dayf, monf))
                return is_live_data(dayf, monf, yrsf, hrsf, minf);
    }
    return VALID;
}

int isFormatD(char *date) {
    int dayf, monf, yrsf;
    if (isValidDateFormat(date)) {
        parse_date(date, &dayf, &monf, &yrsf);
            if (1000 <= yrsf && yrsf <= 9999 && day_valid(dayf, monf))
                return !is_live_data(dayf, monf, yrsf, 0, 0);
    }
    return VALID;
}

int null_entry(park *parks, int i, int j) {
    if (parks[i] != NULL && parks[i]->entry[j] != NULL)
        return (parks[i]->entry[j][0] != NULL);
    return 0;
}

void carIn(char *plate) {
    for (int i = 0; i < carlen; i++)
        if (cars[i] == NULL) {
            cars[i] = plate;
            break;
        }
}
