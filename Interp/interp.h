#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#define MAXNUMTOKENS 100
#define MAXTOKENSIZE 20
#define TSTSTRLEN 100
#define CHARBUFFLEN 100
#define ERROR(PHRASE) { fprintf(stderr, \
          "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
          __FILE__, __LINE__); \
          exit(EXIT_FAILURE); }
//take the above out if not used
#define MAXFILENAME 50
#define VAR_CALL 1
#define NO_VAR_CALL 0
#define COL_WIDTH 51
#define ROW_HEIGHT 33
#define MID_COL 25
#define MID_ROW 16
#define WHITE 'W'
#define SPACE ' '
#define PI 3.14159265358979323846
#define RAD_CONST 180
#define MAX_ANGLE 360

struct prog{
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];
   int cw; // Current Word <-rename to something readable
   char grid[ROW_HEIGHT][COL_WIDTH];
   double curr_y;
   double curr_x; 
   double curr_direction;
};
typedef struct prog Program;

char str[ROW_HEIGHT*COL_WIDTH+1];

//Parser grammar functions
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

//parser helper functions
void clear_buff(Program *p);
void str2buff(Program *p, char* tst, int numwords);
void rst_pt(Program *p);
void next_word(Program *p); 
bool word_matches(Program *p, char match[MAXTOKENSIZE]);

//Interpreter functions 
bool empty_area(Program *p);
void init_turtle(Program *p);
void grid2str(char str[ROW_HEIGHT*COL_WIDTH+1], Program *p);
void print_grid(Program *p);
void write_file(char *argv[], Program *p);
void output_file(FILE* fpout, Program *p);
double update_y_position(Program *p, double y_delta);
double update_x_position(Program *p, double x_delta);
double deg2rad(double deg);
void change_direction(Program *p, double new_direction);
double offset_degree(double deg);
double neg_degree_to_pos(double deg);
double get_delta_y(double direction, double distance);
double get_delta_x(double direction, double distance);


//Test function
void test(void);
