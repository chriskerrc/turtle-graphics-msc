#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#define MAXNUMTOKENS 100
#define MAXTOKENSIZE 20
#define TSTSTRLEN 100
#define CHARBUFFLEN 100
#define MAXFILENAME 50
#define VAR_CALL 1
#define NO_VAR_CALL 0
#define WDS_CW p->wds[p->cw]
#define CHAR_OF_LTR 2

struct prog{
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];
   int cw;
};
typedef struct prog Program;

bool Prog(Program *p); 
bool Inslst(Program *p);
bool Num(Program *p);
bool Rgt(Program *p);
bool Fwd(Program *p);
bool Ins(Program *p);
bool Op(Program *p);
bool Ltr(Program *p, int is_var_call);
bool Word(Program *p);
bool Var(Program *p);
bool Varnum(Program *p);
bool Item(Program *p);
bool Col(Program *p);
bool Pfix(Program *p);
bool Items(Program *p);
bool Set(Program *p);
bool brace_then_pfix(Program *p);
bool Lst(Program *p);
bool Loop(Program *p);
bool over_lst_inslst(Program *p);
//helper functions
bool get_arg_filename(int argc, char *argv[], char* filename);
void clear_buff(Program *p);
void str2buff(Program *p, char tst[TSTSTRLEN], int num_wds); 
void rst_ptr(Program *p);
void next_word(Program *p); 
bool word_matches(Program *p, char match[MAXTOKENSIZE]);
void test(void);
