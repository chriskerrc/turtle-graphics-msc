#pragma once

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
#define RED 'R'
#define BLUE 'B'
#define BLACK 'K'
#define YELLOW 'Y'
#define MAGENTA 'M'
#define CYAN 'C'
#define GREEN 'G'
#define SPACE ' '
#define PI 3.14159265358979323846
#define RAD_CONST 180
#define MAX_ANGLE 360
#define ERROR_CONST 2
#define ROTATE_CONST -90
#define OUT_FILE 2
#define A_TO_Z 26
#define BASE_LETTER 'A'
#define LAST_TO_INS 2

//TO DO: put Neill's stuff in different header file if possible 
//Neill's stack (specific.h)
#define MAXINPUT 1000
#define FORMATSTR "%i"
#define ELEMSIZE 20
#define STACKTYPE "Realloc"
#define FIXEDSIZE 16
#define SCALEFACTOR 2

//Neill's stack (stack.h)

typedef struct stack stack;
typedef double stacktype;

/* Create an empty stack */
stack* stack_init(void);
/* Add element to top */
void stack_push(stack* s, stacktype i);
/* Take element from top */
bool stack_pop(stack* s, stacktype* d);
/* Clears all space used */
bool stack_free(stack* s);

/* Optional? */

/* Copy top element into d (but don't pop it) */
bool stack_peek(stack*s,  stacktype* d);
/* Make a string version - keep .dot in mind */
void stack_tostring(stack*, char* str);

//Neill's stack (general.h)
void on_error(const char* s);
void* ncalloc(int n, size_t size);
void** n2dcalloc(int h, int w, size_t size);
void** n2drecalloc(void** p, int oh, int nh, int ow, int nw, size_t szelem);
void n2dfree(void**p, int h);
void* nrecalloc(void* p, int oldsz, int newsz);
void* nremalloc(void* p, int bytes);
void* nfopen(char* fname, char* mode);

//Neill's 

struct stack {
   /* Underlying array */
   stacktype* a;
   int size;
   int capacity;
};

//my stuff again 

enum var_data_type {NUMBER, STRING};
typedef enum var_data_type var_data_type; 

struct var{                 
   var_data_type type; 
   union { 
      double num; 
      char* colour; 
   } data; 
};
typedef struct var variable;

struct prog{
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];
   int cw; // Current Word <-rename to something readable
   char grid[ROW_HEIGHT][COL_WIDTH];
   double curr_y;
   double curr_x; 
   double curr_direction;
   bool is_text_output; 
   char colour; 
   variable vars[A_TO_Z];
   //int curr_item_index; 
   int loop_var_index;
   int loop_jump; 
   int first_item_index;
   int last_item_index; 
   int item_count; //don't think I need this...
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
bool Pfix(Program *p, stack *s);
bool Items(Program *p);
bool Set(Program *p);
bool brace_then_pfix(Program *p, stack *s);
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
void print_grid_screen(Program *p);
void write_file(char *argv[], Program *p);
void output_file(FILE* fpout, Program *p);
void update_y_position(Program *p, double y_delta);
void update_x_position(Program *p, double x_delta);
double deg2rad(double deg);
void change_direction(Program *p, double new_direction);
double validate_degree(double deg);
double neg_degree_to_pos(double deg);
double get_delta_y(double direction, double distance);
double get_delta_x(double direction, double distance);
void draw_line(Program *p, int y_start, int x_start, int y_end, int x_end);//Bresenham line algorithm
void draw_forward(Program *p, double distance);
int get_new_y(Program *p, double delta_y);
int get_new_x(Program *p, double delta_x);
bool is_y_in_bounds(double y);
bool is_x_in_bounds(double x);
void plot_pixel(Program *p, int curr_y_plot, int curr_x_plot);
bool word_is_colour(Program *p);
void set_colour(Program *p, char col);
char get_colour(Program *p);
int char2col(char col);
int char2index(char letter);
void calc_binary_expression(Program *p, stack *s);
char get_character(Program *p);
char var2letter(Program *p);
void set_val_var(Program *p, double val, int index);
double get_val_var(Program *p, int index);
char get_letter(Program *p);
int get_last_item_index(Program *p);
int get_first_item_index(Program *p);
int get_loop_jump(int first_item_index, int last_item_index); //get initial loop jump i.e. number to add to index first list item to get to index first ins 
void execute_loop(Program *p);
bool get_double(Program *p, double *result);

//Test function
void test(void);
