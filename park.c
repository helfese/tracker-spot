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

static int comma_count = NINT;
static int i_parks = NINT;
static int park_size = PARK_MAX;
static int reglen = REGS_MAX;
static int carlen = CARS_MAX;
static size_t tok_len = NINT;
static int had_error = NINT;
static int bill_id = -1;
static int addpark = 0;
static int has_pair_upp = 0;
static int has_pair_dig = 0;
static int park_id = -1;

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

park *parks;
char **cars;

struct parks *qparks;
int live[5];

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
            if (isspace(*ptr))
                if (!prev_blank)
                    str[prev_blank = i++] = BLNK_CHR;

            if (!isspace(*ptr) && (prev_blank = null) == BASE)
                str[i++] = *ptr;
        } else {
            str[i++] = *ptr;
        }
        ptr++;
    }

    str[i] = STR_END;

    if (str[j] == BLNK_CHR && str[j + FRST] == STR_END)
        return NULL_STR;
    else if (str[j++] == BLNK_CHR && str[j++] == BLNK_CHR && str[j] == STR_END)
        return NULL_STR;

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

int entry_invalid(char *plate) {
    for (int i = 0; i < carlen; i++)
        if (cars[i] != NULL && !strcmp(plate, cars[i]))
            return 1;
    return 0;
}  

void carOut(char *plate) {
    for (int i = 0; i < carlen; i++) {
        if (cars[i] != NULL && !strcmp(plate, cars[i])) {
            for (int j = i; j < carlen - 1; j++) {
                cars[j] = cars[j + 1];
            }
            break;
        }
    }
}

int exit_invalid(char *plate, park Park) {
    int cnt = 0;
    for (int j =0; j < reglen; j++)
        if (Park != NULL && Park->entry[j] != NULL)
            if (Park->entry[j][0] != NULL)
                if (!strcmp(Park->entry[j][0], plate)) cnt++;
    return !(cnt % 2);
}

void date_invalid_ckd() {
    had_error = TRUE;
    printf(ERROR_DATE_INVALID);
}

int date_hour_invalid(char *date, char *hour) {
    if (isFormatDH(date, hour))
        return VALID;
    date_invalid_ckd();
    return INVALID;
}

int date_invalid(char *date) {
    if (isFormatD(date))
        return VALID;
    date_invalid_ckd();
    return INVALID;
}

void print_parks() {
    int i = NINT;
    while (i < park_size) {
        if (parks[i] != NULL) {
            printf("%s ", parks[i]->name);
            printf("%d ", parks[i]->spots);
            printf("%d\n", parks[i]->free_spots);
        }
    i++;
    }
}

char* gstr() {
    return strtok(NULL, BLNK_STR);
}

int gstrint() {
    return strtol(gstr(), NULL, NBASE);
}

float gstrfl() {
    return strtod(gstr(), NULL);
}

int invalid_park_name(char* name) {
    for (size_t i = 0; i < strlen(name); i++) {
        if ('0' <= name[i] && name[i] <= '9') {
            printf("invalid park name.\n");
            had_error = TRUE;
            return 1;
        }
    }
    return 0;
}

void check_p_error(char *name, int spot, float v15, float v1h15, float vmax) {
    if (invalid_park_name(name));
    else if (park_exist(name)) {
        had_error = TRUE;
        printf("%s%s", name, ERROR_PARK_EXIST);
    }
    else if (ckspot_out(spot));
    else if (ckcost_out(v15, v1h15, vmax));
    else ckpark_out();
}

void has_park_e_s_error(char* name) {
    had_error = TRUE;
    printf(ERROR_SFORMAT, name, ERROR_NO_PARK);
}

void check_e_error(char *name, char *plate, char *date, char *hour) {
    if (!park_exist(name)) has_park_e_s_error(name);
    else if (ckpark_full(name));
    else if (ckplate_valid(plate));
    else if (entry_invalid(plate)) {
        had_error = TRUE;
        printf(ERROR_SFORMAT, plate, ERROR_YES_ENTRY);
    }
    else date_hour_invalid(date, hour);
}

void check_s_error(char *name, char *plate, char *date, char *hour) {
    if (!park_exist(name)) has_park_e_s_error(name);
    else if (ckplate_valid(plate)); 
    else if (exit_invalid(plate, parks[park_id])) {
        had_error = TRUE;
        printf(ERROR_SFORMAT, plate, ERROR_NO_ENTRY);
    }
    else date_hour_invalid(date, hour);
}

void any_entry(char *plate) {
    int i = -1; int j = -1;
    while (++i < park_size) for (j =0; j < reglen; j++) 
        if (null_entry(parks, i, j)) {
                if (!strcmp(parks[i]->entry[j][0], plate)) return;
            }
    had_error = TRUE;
    printf("%s%s", plate, ERROR_ANY_ENTRY);
}

void check_v_error(char *plate) {
    if (ckplate_valid(plate));
    else any_entry(plate);
}

void check_f_error(char *name, char *date) {
    if (!park_exist(name)) has_park_e_s_error(name);
    else date_invalid(date);
}

void exec_p(park *parks, park Park) {
    int i = NINT;
    while (i < park_size) {
        if (parks[i] == NULL) {
                parks[i] = Park;
                return;
            }
        i++;
    }
}

void set_park_id() {
    park_id = -1;
}

void exec_e_s(park Park, char *plate, char *date, char *hour) {
    while (i_parks >= reglen) return;
        Park->entry[i_parks][0] = plate;
        Park->entry[i_parks][1] = strdup(date);
        Park->entry[i_parks][2] = strdup(hour);
        i_parks++;
    to_live_data(date, hour);
}

int cmp(const void *a, const void *b) {
    const struct parks *parkA = (const struct parks *)a;
    const struct parks *parkB = (const struct parks *)b;
    return strcmp(parkA->name, parkB->name);
}

void swap(struct parks *a, struct parks *b) {
    struct parks temp = *a;
    *a = *b;
    *b = temp;
}

int partition(struct parks *qparks, int low, int high, int (*func)(const void *, const void *)) {
    struct parks pivot = qparks[high];
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        if (func(&qparks[j], &pivot) < 0) {
            i++;
            swap(&qparks[i], &qparks[j]);
        }
    }
    swap(&qparks[i + 1], &qparks[high]);
    return (i + 1);
}

void quickSort(struct parks *qparks, int low, int high, int (*func)(const void *, const void *)) {
    if (low < high) {
        int pi = partition(qparks, low, high, func);
        quickSort(qparks, low, pi - 1, func);
        quickSort(qparks, pi + 1, high, func);
    }
}

void qPark(struct parks *qparks, int size, 
    int (*func)(const void *, const void *)) {
        quickSort(qparks, 0, size - 1, func);
}

struct parks *parksdup(park *parks) {
    for (int i = 0; i < addpark; i++) {
        if (parks[i] != NULL && parks[i]->name != NULL) {
            qparks[i].name = strdup(parks[i]->name);
            qparks[i].entry = parks[i]->entry;
        }
    }
    return qparks;
}

void exec_v(char *plate) {
    int i = -1; 
    int j = 0;
    char *name, *date, *hour;

    int d, mo, y, h, mn;

    while (++i < addpark){
        int cnt = 0;
        for (j =0; j < reglen; j++) {
            if (qparks[i].entry[j] != NULL && qparks[i].entry[j][0] != NULL) {
                if (!strcmp(qparks[i].entry[j][0], plate)){
                    if (cnt == 0) {
                        name = qparks[i].name;
                        date = qparks[i].entry[j][1];
                        hour = qparks[i].entry[j][2];
                        parse_dh(date, hour, &d, &mo, &y, &h, &mn);
                        printf("%s %02d-%02d-%d %02d:%02d",
                        name, d, mo, y, h, mn);
                        cnt++;
                    }
                    else if (cnt == 1) {
                        date = qparks[i].entry[j][1];
                        hour = qparks[i].entry[j][2];
                        parse_dh(date, hour, &d, &mo, &y, &h, &mn);
                        printf(" %02d-%02d-%d %02d:%02d\n", d, mo, y, h, mn);
                        cnt = 0;
                    }
                }

            }
        if (j == reglen - 1 && cnt == 1) printf("\n");
        }
        
    }
}

int daymo(int mo, int yr) {
    (void) yr;
    int day[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return day[mo - 1];
}

long long datemin(int d, int mo, int y, int h, int mn) {
    long long sum = 0;
    for (int yr = 2024; yr < y; yr++) sum += 365 * 24 * 60;
    for (int m = 1; m < mo; ++m) sum += daymo(m, y) * 24 * 60;
    sum += (d - 1) * 24 * 60;
    sum += h * 60 + mn;
    return sum;
}

void neg_then_nul(int *Nv15, int *Nvh15) {
    if (*Nv15 < 0) *Nv15 = 0;
    if (*Nvh15 < 0) *Nvh15 = 0;
}

float val(char *Di, char *Hi, char *Df, char *Hf, FL v15, FL vh15, FL vd) {
    float v = 0;
    int di, moi, yi, hi, mni, df, mof, yf, hf, mnf;
    parse_dh(Di, Hi, &di, &moi, &yi, &hi, &mni);
    parse_dh(Df, Hf, &df, &mof, &yf, &hf, &mnf);
    long long Ni = datemin(di, moi, yi, hi, mni);
    long long Nf = datemin(df, mof, yf, hf, mnf);
    long long diff = Nf - Ni + 15 - 1;
    int Nv15, Nvh15, Nvd;
    Nvd = diff/(24 * 60);
    diff -= Nvd * 24 * 60;
    Nv15 = diff > 60 ? 4 : diff / 15;
    Nvh15 = (diff - 60)/15;
    neg_then_nul(&Nv15, &Nvh15);
    if (Nv15 * v15 + Nvh15 * vh15 > vd) return v = Nvd * vd + vd; 
    return v = (float) Nvd * vd + Nvh15 * vh15 + Nv15 * v15;
}

void exec_f(park Park, char *date) {
    char *pl, *hr; float vl;
    int h, m;
    for (int i = 0; i <= bill_id; i++) {
        if (Park->bills[i].dtt != NULL) {
            if (!strcmp(date, Park->bills[i].dtt)) {
                pl = Park->bills[i].plt;
                hr = strdup(Park->bills[i].hrs);
                vl = Park->bills[i].vlr;
                parse_hour(hr, &h, &m);
                printf("%s %02d:%02d %.2f\n", pl, h, m, vl);
            }
        }
    }
}

void cleanNull(park *parks) {
    int count = 0;
    for (int i = 0; i < park_size; i++) {
        if (parks[i] != NULL) {
            parks[count++] = parks[i];
        }
    }

    for (int i = count; i < park_size; i++) {
        parks[i] = NULL;
    }
}

void exec_r(park Park) {
    for (int i = 0; i < park_size; i++)
        if (parks[i] == Park) {
            parks[i] = NULL;
            break;
        }

    if (addpark + 1 <= park_size) {
        addpark--;
        if (addpark > 1)
            for (int i = 0; i < addpark - 1; i++)
                free(qparks[i].name);
        free(qparks);
        qparks = malloc(sizeof(struct parks) * addpark);
        cleanNull(parks);
        qparks = parksdup(parks);
        qPark(qparks, addpark, cmp);
    }

    for (int i = 0; i < reglen; i++) {
        if (Park->entry[i][0] != NULL) 
            for (int j = 0; j < 3; j++) {
                if (j == 0)
                    carOut(Park->entry[i][0]);
                else
                    free(Park->entry[i][j]);
            }
    }
    free(Park->entry);
    for (int i = 0; i <= bill_id; i++)
        if (Park->bills[i].plt != NULL) {
            free(Park->bills[i].plt);
            free(Park->bills[i].dtt);
            free(Park->bills[i].hrs);
        }
    free(Park->bills);
    free(Park->name);
    free(Park);
    for (int i = 0; i < addpark; i++) 
            printf("%s\n", qparks[i].name);
}

void exec_p_cmd(char *str) {
    char* name = get_tok(str);
    if (name == NULL) print_parks();
    else {
        park Park = malloc(sizeof(struct parks));
        Park->name = malloc(sizeof(char)*(strlen(name) + FRST));
        strcpy(Park->name, name);
        int spot = Park->spots = gstrint();
        float v15 = Park->val_15 = gstrfl();
        float v1h15 = Park->val_15_1h = gstrfl();
        float vmax = Park->val_max_day = gstrfl();
        Park->free_spots = spot;
        Park->entry = malloc(sizeof(char***) * reglen);
        for (int i = 0; i < reglen; i++) 
            Park->entry[i] = malloc(sizeof(char**) *3);
        Park->bills = malloc(sizeof(bill) * reglen);
        check_p_error(name, spot, v15, v1h15, vmax); 
        if (ckreset_error()) return;
        exec_p(parks, Park);
        if (addpark + 1 <= park_size) {
            addpark++;
            if (addpark > 1)
                for (int i = 0; i < addpark - 1; i++)
                    free(qparks[i].name);
            free(qparks);
            qparks = malloc(sizeof(struct parks) * addpark);
            qparks = parksdup(parks);
            qPark(qparks, addpark, cmp);
        }
    }
    set_park_id();
}

void exec_e_cmd(char *str) {
    char *name = get_tok(str);
    char *plate = gstr();
    char *date = gstr();
    char *hour = gstr();
    check_e_error(name, plate, date, hour);
    if (ckreset_error()) return;
    if (!entry_invalid(plate)) 
        carIn(plate);
    exec_e_s(parks[park_id], plate, date, hour);
    printf("%s %d\n", parks[park_id]->name, --parks[park_id]->free_spots);
    set_park_id();
}

void remDup(char **array, int size) {
    int i, j, k;
    for (i = 0; i < size - 1; i++) {
        if (array[i] != NULL) {
            for (j = i + 1; j < size; j++) {
                if (array[j] != NULL && strcmp(array[i], array[j]) == 0) {
                    free(array[j]);
                    for (k = j; k < size - 1; k++) {
                        array[k] = array[k + 1];
                    }
                    array[size - 1] = NULL;
                    size--;
                    j--;
                }
            }
        }
    }
}

void print_bills(park Park) {
    float sum; char* date;
    char *dttarr[bill_id+1]; int len = 0;
    for (int j = 0; j <= bill_id; j++)
        if (Park->bills[j].dtt != NULL){ 
            len++;
            dttarr[j] = strdup(Park->bills[j].dtt);}
        else dttarr[j] = NULL;
    remDup(dttarr, bill_id + 1);
    for (int j = 0; j <= bill_id; j++) {
        date = dttarr[j];
        if (date != NULL) {
            sum = 0;
            for (int i = 0; i <= bill_id; i++){
                if (Park->bills[i].dtt != NULL && date != NULL){
                    if (!strcmp(date, Park->bills[i].dtt)) 
                        sum += Park->bills[i].vlr;
                }
            }
            printf("%s %.2f\n", date, sum);
        }
    }
}
