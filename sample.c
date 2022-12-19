#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cMIN -1
#define cLOAD 0
#define cSHOW 1
#define cUNLOAD 2
#define cEXIT 3
#define cRUN 4
#define cMAX 5

#define oMIN -1
#define oADD 0
#define oAND 1
#define oCOMP 2
#define oDIV 3
#define oJ 4
#define oJEQ 5
#define oJGT 6
#define oJLT 7
#define oJSUB 8
#define oLDA 9
#define oLDCH 10
#define oLDL 11
#define oLDX 12
#define oMUL 13
#define oOR 14
#define oRD 15
#define oRSUB 16
#define oSTA 17
#define oSTCH 18
#define oSTL 19
#define oSTSW 20
#define oSTX 21
#define oSUB 22
#define oTD 23
#define oTIX 24
#define oWD 25
#define oMAX 26

FILE *f;
char fname[20];
char tname[20];
char c_line[30];
char o_line[80];
char cmd[8];
int prog_len = 0;
int start_add = 0;
int first_add = 0;
int curr_add = 0;
char *memory;
int mem_size = 0;
int loaded = 0;
int op = 0;
int indexed = 0;
int operand = 0;
int running = 0;

int reg_A, reg_X, reg_L, reg_PC, reg_SW;

const char s_command[5][7] = {"load", "show", "unload", "exit", "run"};
const int MAXADD = 0x7FFF;
const int IKEY = 0x8000;
const char optab[26][3] = {"18", "40", "28", "24", "3C", "30", "34", "38", "48", "00", "50", "08", "04", "20", "44", "D8", "4C", "0C", "54", "14", "E8", "10", "1C", "E0", "2C", "DC"};

int lookup ( char *s ) {
    int i;
    for (i=0; i<oMAX; i++)
	if (strcmp(optab[i], s) == 0) return i;
    return -1;
}

int readline()
{
    int i = 0 ;
    int tmp = cMIN ;

    printf("SIC Simulator> ") ;
    do
    {
        fgets( c_line, 30, stdin ) ;
        i = strlen( c_line ) ;
        c_line[i-1] = '\0' ;
    }
    while (i <= 1) ;

    sscanf( c_line, "%s", cmd ) ;

    if (strcmp( cmd, s_command[0]) == 0)
    {
        sscanf( c_line, "%s %s", cmd , fname ) ;
        tmp = cLOAD ;
    } 
    else if (strcmp( cmd, s_command[1]) == 0) tmp = cSHOW ; 
    else if (strcmp( cmd, s_command[2]) == 0) tmp = cUNLOAD ; 
    else if (strcmp( cmd, s_command[3]) == 0) tmp = cEXIT ; 
    else if (strcmp( cmd, s_command[4]) == 0) tmp = cRUN ;

    c_line[0] = '\0' ;
    cmd[0] = '\0' ;
    return tmp ;
}

void rd_header () {
    char tmp[7];
    int i, j, s;

    for (i=7, j=0; i<13; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf( tmp, "%x", &start_add );

    for (i=13, j=0; i<19; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf( tmp, "%x", &prog_len );

    s = prog_len * 2 + 1;
    memory = (char *)malloc(sizeof(char) * s);
    if (memory) {
    for (i=0; i<s; i++)  memory[i] = 'X';
    memory[s-1] = '\0';
    loaded = 1;
    mem_size = s;
    } else {
	printf("Loading Failed! (Memory allocation error)\n");
    }
}

void rd_text () {
    char tmp[7];
    int i, j, l, s;

    for (i=1, j=0; i<7; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf( tmp, "%x", &s );

    for (i=7, j=0; i<9; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf( tmp, "%x", &l );

    l = 9 + l * 2;
    for (i=9, j=(s - start_add) * 2; i < l; i++, j++)
	memory[j] = o_line[i];
}

void rd_end () {
    char tmp[7];
    int i, j;

    for (i=1, j=0; i<7; i++, j++)  tmp[j] = o_line[i];
    tmp[j] = '\0';
    sscanf( tmp, "%x", &first_add );
}

void s_load()
{
    if (loaded)
    {
        printf("Load failed: Please unload first.\n") ;
        return ;
    }
    f = fopen(fname, "r") ;
    if (!f)
    {
        printf("Load Failed : The [%s] does not exist.\n", fname) ;
        return ;
    }
    fgets(o_line, 80, f) ;
    rd_header() ;
    fgets(o_line, 80, f) ;
    while (o_line[0] != 'E')
    {
        rd_text() ;
        fgets(o_line, 80, f) ;
    }
    rd_end() ;
    o_line[0] = '\0' ;
    fclose(f) ;
}

void s_show()
{
    if (!loaded)
    {
        printf("Show failed: Memory is not loaded.\n") ;
        return ;
    }
    int s = first_add ;
    f = fopen("Memory_address.txt", "w") ;
    for (int i = 0 ; i < mem_size - 1 ; i++)
    {
        if (!(i % 8) && i != 0)
        {
            s += 4 ;
            fprintf(f, " ") ;
        }
        if (!(i % 32))
        {
            if (i != 0) fprintf(f, "\n") ;
            fprintf(f, "%x ", s) ;
        }
        fprintf(f, "%c", memory[i]) ;
    }
    fclose(f) ;
}

void s_unload()
{
    free(memory) ;
    fname[0] = '\0' ;
    first_add = 0 ;
    start_add = 0 ;
    loaded = 0 ;
    mem_size = 0 ;
}

void init_run () {
    reg_A = 0;
    reg_X = 0;
    reg_L = 0;
    reg_PC = first_add;
    reg_SW = 0;
    curr_add = (first_add - start_add) * 2;
    running  = 1;
}

void get_op()
{
    char s[3] ;
    char t[5] ;
    s[0] = memory[curr_add] ;
    s[1] = memory[curr_add + 1] ;
    s[2] = '\0' ;
    op = lookup(s) ;
    t[0] = memory[curr_add + 2] ;
    t[1] = memory[curr_add + 3] ;
    t[2] = memory[curr_add + 4] ;
    t[3] = memory[curr_add + 5] ;
    t[4] = '\0' ;
    sscanf(t, "%X", &operand) ;
    if (operand >= IKEY)
    {
	    indexed = 1 ;
	    operand -= IKEY ;
    }
    else indexed = 0 ;
    
    if (op == -1)
    {
        curr_add += 2 ;
        reg_PC++ ;
        return ;
    }
    if (op != oRSUB && operand <= start_add)
    {
        curr_add += 6 ;
        reg_PC += 3 ;
        op = -1 ;
        return ;
    }

    curr_add += 6 ;
    reg_PC += 3 ;
}

int get_value (int r, int x) {
    int tmp = 0;
    char s[7];
    int i, j;
    if (x) r += reg_X;
    i = (r - start_add) * 2;
    for (j=0; j<6; j++) s[j] = memory[i++];
    s[6] = '\0';
    sscanf(s, "%X", &tmp);
    return tmp;
}

int get_byte (int r, int x) {
    int tmp = 0;
    char s[3];
    int i, j;
    if (x) r += reg_X;
    i = (r - start_add) * 2;
    j=0;
    s[j++] = memory[i++];
    s[j++] = memory[i++];
    s[2] = '\0';
    sscanf(s, "%X", &tmp);
    return tmp;
}

void put_byte (int k, int r, int x) {
    int tmp = 0;
    char s[3];
    int i, j;
    if (x) r += reg_X;
    sprintf( s, "%02X", k );
    s[2] = '\0';
    i = (r - start_add) * 2;
    j=0;
    memory[i++] = s[j++];
    memory[i++] = s[j++];
}

void put_value (int k, int r, int x) {
    int tmp = 0;
    char s[7];
    int i, j;
    if (x) r += reg_X;
    sprintf( s, "%06X", k );
    i = (r - start_add) * 2;
    for (j=0; j<6; j++) memory[i++] = s[j];
}

void show_reg() {
    printf("Register A  = [%06X];\n", reg_A);
    printf("Register X  = [%06X];\n", reg_X);
    printf("Register L  = [%06X];\n", reg_L);
    printf("Register SW = [%06X];\n", reg_SW);
    printf("Register PC = [%06X];\n", reg_PC);
}

void s_run () {
    if (!loaded)
    {
        printf("Run faild: Memory is empty.\n") ;
        return ;
    }
    FILE *in = fopen("in.txt", "r") ;
    FILE *out = fopen("out.txt", "w") ;
    init_run() ;
    get_op() ;
    int count = 0 ;
    while (reg_PC < prog_len + start_add && count <= 4096)
    {
        switch(op) {
            case oADD :
            {
                reg_A += get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oAND :
            {
                reg_A &= get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oCOMP :
            {
                int tmp = get_value(operand, indexed) ;
                if (reg_A == tmp) reg_SW = oJEQ ;
                else if (reg_A < tmp) reg_SW = oJGT ;
                else reg_SW = oJLT ;
                get_op() ;
                break ;
            }
            case oDIV :
            {
                reg_A /= get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oJ :
            {
                reg_PC = operand ;
                curr_add = (reg_PC - start_add) * 2 ;
                get_op() ;
                break ;
            }
            case oJEQ :
            {
                if (reg_SW == oJEQ)
                {
                    reg_PC = operand ;
                    curr_add = (reg_PC - start_add) * 2 ;
                }
                get_op() ;
                break ;
            }
            case oJGT :
            {
                if (reg_SW == oJGT)
                {
                    reg_PC = operand ;
                    curr_add = (reg_PC - start_add) * 2 ;
                }
                get_op() ;
                break ;
            }
            case oJLT :
            {
                if (reg_SW == oJLT)
                {
                    reg_PC = operand ;
                    curr_add = (reg_PC - start_add) * 2 ;
                }
                get_op() ;
                break ;
            }
            case oJSUB :
            {
                reg_L = reg_PC ;
                reg_PC = operand ;
                curr_add = (reg_PC - start_add) * 2 ;
                get_op() ;
                break ;
            }
            case oLDA :
            {
                reg_A = get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oLDCH :
            {
                int tmp = get_byte(operand, indexed) ;
                reg_A &= 0x111100 ;
                reg_A += tmp ;
                get_op() ;
                break ;
            }
            case oLDL :
            {
                reg_L = get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oLDX :
            {
                reg_X = get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oMUL:
            {
                reg_A *= get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oOR :
            {
                reg_A |= get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oRD :
            {
                char d ;
                int data ;
                reg_A &= 0x111100 ;
                d = fgetc(in) ;
                if (d == EOF) d &= 0x000000 ;
                data = (int)d ;
                reg_A += data ;
                get_op() ;
                break ;
            }
            case oRSUB :
            {
                reg_PC = reg_L ;
                if (reg_PC == 0x000000)
                {
                    reg_PC = prog_len + start_add ;
                    break ;
                }
                else curr_add = (reg_PC - start_add) * 2 ;
                get_op() ;
                break ;
            }
            case oSTA:
            {
                put_value(reg_A, operand, indexed) ;
                get_op() ;
                break ;
            }
            case oSTCH :
            {
                put_byte(reg_A, operand, indexed) ;
                get_op() ;
                break ;
            }
            case oSTL :
            {
                put_value(reg_L, operand, indexed) ;
                get_op() ;
                break ;
            }
            case oSTSW :
            {
                put_value(reg_SW, operand, indexed) ;
                get_op() ;
                break ;
            }
            case oSTX :
            {
                put_value(reg_X, operand, indexed) ;
                get_op() ;
                break ;
            }
            case oSUB :
            {
                reg_A -= get_value(operand, indexed) ;
                get_op() ;
                break ;
            }
            case oTD :
            {
                reg_SW = (get_byte(operand, indexed)) ? oJLT : oJEQ ;
                get_op() ;
                break ;
            }
            case oTIX :
            {
                int tmp = get_value(operand, indexed) ;
                reg_X++ ;
                if (reg_X == tmp) reg_SW = oJEQ ;
                else if (reg_X > tmp) reg_SW = oJGT ;
                else reg_SW = oJLT ;
                get_op() ;
                break ;
            }
            case oWD :
            {
                char data[3] ;
                int d ;
                data[2] = '\0' ;
                sprintf(data, "%02x", reg_A) ;
                sscanf(data, "%x", &d) ;
                fprintf(out, "%c", d) ;
                get_op() ;
                break ;
            }
            default : get_op() ;
        }
        count++ ;
    }
    show_reg() ;
    fclose(f) ;
    fclose(in) ;
    fclose(out) ;
}

int main()
{
    int comm = 0 ;
    comm = readline() ;
    while (comm != cEXIT)
    {
        switch (comm)
        {
            case cLOAD : s_load() ; break ;
            case cSHOW : s_show() ; break ;
            case cUNLOAD : s_unload() ; break ;
            case cRUN : s_run() ; break ;
            default : printf("Unknown Command!\n") ; break ;
        }
        comm = cMIN ;
        comm = readline() ;
    }
    if (loaded) s_unload() ;
    return 0 ;
}
