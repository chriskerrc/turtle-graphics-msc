#include "interp.h"
#include "../neillsimplescreen.h"

//run sanitizer and valgrind 
//make interpreter fail for invalid colour "ORANGE" etc. 
//make interpreter exit gracefully if stack underflow/overflow! 
//remember to remove my dummy file forward_var_test.ttl from TTls folder

int main(int argc, char *argv[]) //make main function shorter
{
   bool production = true;
   if(production == false){
      test();
   }
   
   Program* prog = calloc(1, sizeof(Program));
   int i=0;
   init_turtle(prog);
   if(argc == 3){
      prog->is_text_output=true;
   }
   else{
      prog->is_text_output=false; 
   }

   if(argc != SCREEN_OUT_ARGS && argc != TXT_OUT_ARGS){
      fprintf(stderr, "Expecting two or three command line arguments\n");
      exit(EXIT_FAILURE);
   }
   
   //move this to open read file wrapper function 
   FILE* fpin = fopen(argv[1], "r");
   if(fpin == NULL){
       fprintf(stderr, "Cannot read file %s ?\n", argv[1]);
       exit(EXIT_FAILURE);
   }
   
   while(fscanf(fpin, "%s", prog->wds[i])==1 && i<MAXNUMTOKENS){ //check that fscanf returns something
      i++;
   }

   if(Prog(prog)){
      fclose(fpin);
      if(prog->is_text_output==true){
         write_file(argv, prog);
      }
      free(prog); 
      return EXIT_SUCCESS;
   }

   printf("Failed to interpret (in main)\n");
   fclose(fpin);
   free(prog); 
   return EXIT_FAILURE;
}

bool Prog(Program *p)
{
   if(p->is_text_output == false){
      neillclrscrn(); 
   }
   if(!word_matches(p, "START")){
      return false;
   }
   next_word(p);
   if (Inslst(p)) {
      return true;   
   }
   return false;
}

bool Inslst(Program *p)
{
   if(word_matches(p, "END")){
      return true;
   }
   if(Ins(p)){
      next_word(p);
      if(Inslst(p)){
         return true;
      }
   }
   return false;
}

bool Ins(Program *p)
{
   if(Fwd(p)){
      return true;
   }
   else if(Rgt(p)){
      return true; 
   }
   else if(Col(p)){
      return true; 
   }
   else if(Loop(p)){
      return true; 
   }
   else if(Set(p)){
      return true; 
   }
   return false;
}

//found a bug where I was calling wrong function get_character, instead of var2letter, which was causing $ char to be passed to char2index
//this was producing index = -29, which was causing out of bounds
//to do: add some bounds checking in char2index
bool Fwd(Program *p) 
{   
   double distance = 0;
   if(!word_matches(p, "FORWARD")){ 
      return false;
   }
   next_word(p);
   if(!Varnum(p)){ 
      return false;
   }
   if(Num(p)){
      if(get_double(p, &distance)){
         draw_forward(p, distance);
         if(p->is_text_output==false){
            run_simple_screen(p);
         }
      return true;
      }
   }
   if(Var(p)){ //need to fail interpreter if it's not set to anything)
      char letter = var2letter(p);
      int var_index = char2index(letter);
      if(var_val_is_num(p, var_index)){
         double distance = get_num_val_var(p, var_index); 
         draw_forward(p, distance);
         if(p->is_text_output==false){
            run_simple_screen(p);
         }
         return true;
         }
      else{ 
         printf("Failed to interpret: variable is not num?");  
         return EXIT_FAILURE;  
      }
   }
   return false;
}       

bool Rgt(Program *p) //too deeply nested
{
   double new_direction = 0; 
   if(word_matches(p, "RIGHT")){
      next_word(p);
      if(Varnum(p)){
         if(Num(p)){ //this code is copied from Fwd: make it a function instead
            if(get_double(p, &new_direction)){
               double valid_direction = validate_degree(new_direction);
               change_direction(p, valid_direction);
               return true;
            }
         }
         return true;
      }       
   }   
   return false;
}

bool Num(Program *p)
{
   double d;
   if(sscanf(p->wds[p->cw], "%lf", &d)==1){  
      return true;
   }
   return false;
}

bool Op(Program *p)
{   
   char c[CHARBUFFLEN];
   if(sscanf(p->wds[p->cw], "%[+*-/]", c)==1){  
      return true;
   }
   return false;
}

bool Word(Program *p) // long lines
{
   int len = strlen(WDS_CW);
   char c[CHARBUFFLEN];
   if(sscanf(WDS_CW,"%s", c)==1 && WDS_CW[1]== '\"'){
      return false; 
   }
   if(len > 1){ 
      if(sscanf(WDS_CW,"%s",c)==1 && WDS_CW[0]=='\"' && WDS_CW[len-1]=='\"'){
         return true;
      }
   }
   return false;
}

bool Ltr(Program *p, int is_var_call)
{
   char c[CHARBUFFLEN];
   //letter on its own
   if(is_var_call == NO_VAR_CALL){
      if(sscanf(WDS_CW,"%[A-Z]",c)==1 && WDS_CW[1]== '\0'){ 
      return true;
      }
   }
   //letter in a variable
   if(is_var_call == VAR_CALL){
      if(sscanf(WDS_CW,"$%[A-Z]",c)==1 && WDS_CW[CHAR_OF_LTR]=='\0'){
         return true;
      }
   }
   return false;
}

bool Var(Program *p)
{
   char c[CHARBUFFLEN];
   if(sscanf(p->wds[p->cw], "%s", c)==1 && p->wds[p->cw][0]== '$'){
      if(Ltr(p, VAR_CALL)){
         return true;
      }
      else{
         return false;
      }
   }
   return false;
}

bool Varnum(Program *p)
{
   if(Var(p)){
      return true;
   }
   else{
      if(Num(p)){
         return true;
      }
   }
   return false;
}

bool Item(Program *p)
{  
   if(Varnum(p)){
      return true;
   }
   else{
      if(Word(p)){ 
         return true;
      }
   }
   return false;
}

bool Col(Program *p)
{  
   if(!word_matches(p, "COLOUR")){
      return false;
   }
   next_word(p);
   if(Var(p)){
      char letter = var2letter(p);
      int index = char2index(letter);
      if(var_val_is_col(p, index)){
         char colour = get_col_val_var(p, index);
         set_colour(p, colour);
      }
      //else, throw an error
      return true;
   }
   else{
      if(!Word(p)){
         return false;
      }
      if(word_is_colour(p)){
         char col = colour2char(p);
         set_colour(p, col);
      }
      return true;
   }
   return false;
}         

bool Pfix(Program *p, stack *s) //make this function shorter
{
   double num = -1;
   char letter = 0;
   int var_index = 0;
   if(word_matches(p, ")")){
      return true;
   }
   if(Op(p)){
      calc_binary_expression(p, s);
      next_word(p);
      if(Pfix(p, s)){
         return true;
      }
   }
   else{
      if(!Varnum(p)){
         return false;
      }
      if(Num(p)){
         if(get_double(p, &num)){
            stack_push(s, num);
         }
      }
      if(Var(p)){ //this assumes var is a number, not a colour - check type here and fail interp if it's not a num (I think...)
         letter = var2letter(p);
         var_index = char2index(letter);
         num = get_num_val_var(p, var_index);
         stack_push(s, num);
      }
      next_word(p);
      if(Pfix(p, s)){
         return true;
      }
   }
   return false;
}

bool Items(Program *p) 
{
   if(word_matches(p, "}")){
      return true;
   }
   if(Item(p)){
      next_word(p);
      if(Items(p)){
         return true;
      }
   }
   return false; 
}

bool Set(Program *p) 
{ 
   double num = -1;
   int var_index; 
   char letter = 0;
   stack *pfix_stack;
   pfix_stack = stack_init();
   if(!word_matches(p, "SET")){
      stack_free(pfix_stack);
      return false;
   }
   next_word(p);
   if(!Ltr(p, NO_VAR_CALL)){
      stack_free(pfix_stack);
      return false;
   }
   letter = get_character(p);
   var_index = char2index(letter);
   next_word(p);
   if(brace_then_pfix(p, pfix_stack)){ //assuming that var can only be number, not colour here: if it's not number, exit fail
      stack_pop(pfix_stack, &num); 
      set_num_val_var(p, num, var_index);
      stack_free(pfix_stack);
      return true;
   }
   stack_free(pfix_stack);
   return false; 
}
   
bool brace_then_pfix(Program *p, stack *s)
{
   if(word_matches(p, "(")){
      next_word(p);
      if(Pfix(p, s)){
         return true;
      }
   }
   return false;
}

bool Lst(Program *p)
{
   if(word_matches(p, "{")){
      next_word(p);
      if(Items(p)){
         return true;
      }
   }
   return false; 
}

bool Loop(Program *p)
{  
   if(!word_matches(p, "LOOP")){
      return false; 
   }
   next_word(p);
   if(!Ltr(p, NO_VAR_CALL)){
      return false; 
   }
   char letter = get_character(p);
   int var_index = char2index(letter);
   int loop_var_index = var_index; 
   next_word(p);
   int first_item_index = get_first_item_index(p);
   int last_item_index = get_last_item_index(p);
   int loop_jump = get_loop_jump(first_item_index, last_item_index);
   if(list_is_empty(p)){
      return true;
   }
   if(over_lst_inslst(p)){
      run_loop(p, first_item_index, last_item_index, loop_var_index, loop_jump);
      return true;
   }
   return false; 
}

//this is an important bug I found
bool over_lst_inslst(Program *p) //need make this function only parse, not interpret: pass flags to the child functions to not do anything
//doing stuff with ins is handled by execute_loop
{ 
   if(word_matches(p, "OVER")){
      next_word(p);
      if(Lst(p)){
         next_word(p);
         return true;
      }
   }
   return false;
}

//INTERPRETER FUNCTIONS 

bool empty_grid(Program *p)
{
   int not_null = 0;
   for(int row = 0; row < ROW_HEIGHT; row++){
      for(int col = 0; col < COL_WIDTH; col++){
         if(p->grid[row][col] != '\0'){
            not_null++;
         }
      }
   }
   if(not_null > 0){
      return false;
   }
   return true; 
}

void init_turtle(Program *p)
{
   if(empty_grid(p)==true){
      p->curr_y = MID_ROW;
      p->curr_x = MID_COL;
      p->curr_direction = ROTATE_CONST; 
      set_colour(p, WHITE);
   }
}

void print_grid_screen(Program *p)
{  
   if(p->exit_fail == true){
      return;
   }
   for(int row = 0; row < ROW_HEIGHT; row++){
      for(int col = 0; col < COL_WIDTH; col++){
         char c = p->grid[row][col];
         int ansi = char2ansi(c);
         neillfgcol(ansi);
         if(isalpha(c)){
            printf("%c", c);
         }
         else{
            printf(" ");
         }
      }
      printf("\n");
   }
   printf("\n");
   neillreset();
}

void grid2str(char str[ROW_HEIGHT*COL_WIDTH+1], Program *p)
{
   int str_index = 0; 
   for(int row = 0; row < ROW_HEIGHT; row++){   
      for(int col = 0; col < COL_WIDTH; col++){ 
         if(isalpha(p->grid[row][col])){
            str[str_index] = p->grid[(str_index-col)/COL_WIDTH][str_index - row * COL_WIDTH]; 
            str_index++;
         }
         else{
            str[str_index] = SPACE;
            str_index++;
         }
      }
   }
}

//Direction

void update_y_position(Program *p, double y_delta)
{
   p->curr_y += y_delta;
}

void update_x_position(Program *p, double x_delta)
{
   p->curr_x += x_delta;
}

double deg2rad(double deg)
{
   double rad = deg * (PI/RAD_CONST);
   return rad; 
}

void change_direction(Program *p, double new_direction)
{
   double curr_direction = p->curr_direction;
   double raw_new_direction = new_direction + curr_direction; 
   p->curr_direction = validate_degree(raw_new_direction); 
}

double validate_degree(double deg)
{
   double new_angle = 0;
   if(deg - 0 < 0.00001){
      new_angle = 0;
   }
   if(deg > 0 && deg < MAX_ANGLE){
      new_angle = deg; 
   }
   if(deg > MAX_ANGLE){
      new_angle = deg-MAX_ANGLE; 
   }
   if(deg < 0){
      new_angle = neg_degree_to_pos(deg);
   }
   return new_angle; 
}

double neg_degree_to_pos(double deg) 
{
   double new_angle = deg;
   if(new_angle < 0){
      new_angle = MAX_ANGLE + deg; 
   }
   return new_angle; 
}

double get_delta_y(double direction, double distance)
{
   double valid_direction = validate_degree(direction);
   double angle_rad = deg2rad(valid_direction);
   double delta_y = distance * sin(angle_rad);
   return delta_y; 
}

double get_delta_x(double direction, double distance)
{
   double valid_direction = validate_degree(direction);
   double angle_rad = deg2rad(valid_direction);
   double delta_x = distance * cos(angle_rad);
   return delta_x; 
}

int get_new_y(Program *p, double delta_y)
{  
   double new_y_double = round(p->curr_y) + delta_y;
   int new_y_int = round(new_y_double);
   return new_y_int; 
}

int get_new_x(Program *p, double delta_x)
{
   double new_x_double = p->curr_x + delta_x; 
   int new_x_int = round(new_x_double);
   return new_x_int; 
}

//Bresenham line drawing algorithm

void draw_line(Program *p, int y_start, int x_start, int y_end, int x_end)
{
   Line *l = calloc(1, sizeof(Line));; 
   l->abs_dx = abs(x_end - x_start); 
   l->abs_dy = abs(y_end - y_start);
   l->dx = x_end - x_start;
   l->dy = y_end - y_start; 
   l->x = x_start; 
   l->y = y_start; 
   plot_pixel(p, l->y, l->x);
   l->err = 0;
   //slope < 1
   if(l->abs_dx > l->abs_dy){ 
      draw_gentle_line(p,l);
   }
   //slope >= 1
   if(l->abs_dx <= l->abs_dy){
      draw_steep_line(p,l);
   }
   free(l);
}

void draw_steep_line(Program *p, Line *l)
{
   l->err = ERR_CONST*l->abs_dx - l->abs_dy;
   for(int i = 0; i < l->abs_dy; i++){
      if(l->dy < 0){
         l->y--;
      }
      else{
         l->y++;
      }
      if(l->err < 0){
         l->err = l->err + ERR_CONST * l->abs_dx;
      }
      else{
         if(l->dx < 0){
            l->x--; 
         }
         else{
            l->x++;
         }
         l->err = l->err+(ERR_CONST*l->abs_dx)-(ERR_CONST*l->abs_dy);
      }
      plot_pixel(p, l->y, l->x);
   }
}

void draw_gentle_line(Program *p, Line *l)
{
   l->err = ERR_CONST*l->abs_dy - l->abs_dx; 
   for(int i = 0; i < l->abs_dx; i++){
      if(l->dx < 0){
         l->x--; 
      }
      else{
         l->x++;
      }
      if(l->err < 0){
         l->err = l->err + ERR_CONST * l->abs_dy; 
      }
      else{
         if(l->dy < 0){
            l->y--;
         }
         else{
            l->y++;
         }
         l->err =l->err+(ERR_CONST*l->abs_dy-ERR_CONST*l->abs_dx);
      }
      plot_pixel(p, l->y, l->x);
   }
}

void plot_pixel(Program *p, int curr_y_plot, int curr_x_plot)
{  
   char curr_col = get_colour_char(p);
   p->grid[curr_y_plot][curr_x_plot] = curr_col;
}

bool is_x_in_bounds(double x)
{
   if(x >= 0 && x < COL_WIDTH){
      return true;
   }
   return false; 
}

bool is_y_in_bounds(double y)
{
   if(y >= 0 && y < ROW_HEIGHT){
      return true;
   }
   return false; 
}

bool x_and_y_in_bounds(double x, double y)
{
   if(is_x_in_bounds(x) && is_y_in_bounds(y)){
      return true;
   }
   return false;
}

void draw_forward(Program *p, double distance)
{
   double raw_direction = p->curr_direction;
   double valid_direction = validate_degree(raw_direction);
   int start_y = round(p->curr_y);
   int start_x = round(p->curr_x);
   double delta_x = get_delta_x(valid_direction, distance);
   double delta_y = get_delta_y(valid_direction, distance);
   int end_y = get_new_y(p, delta_y);
   int end_x = get_new_x(p, delta_x);
   if(x_and_y_in_bounds(start_x, start_y) && x_and_y_in_bounds(end_x, end_y)){ 
      draw_line(p, start_y, start_x, end_y, end_x);
   }
   else{
      //printf("Not in bounds\n");
   }
   update_y_position(p, delta_y);
   update_x_position(p, delta_x);
}

//File output 

void write_file(char *argv[], Program *p)
{
   FILE* fpout = fopen(argv[OUT_FILE], "w"); 
      if(fpout == NULL){
         fprintf(stderr, "Cannot write to file %s ?\n", argv[OUT_FILE]);
         exit(EXIT_FAILURE);
      }
   output_file(fpout, p);
   fclose(fpout);
}

void output_file(FILE* fpout, Program *p)
{
   for(int row = 0; row < ROW_HEIGHT; row++){
      for(int col = 0; col < COL_WIDTH; col++){
         if(isalpha(p->grid[row][col])){
            fputc(p->grid[row][col], fpout);
         }
         else{
            fputc(' ', fpout);
         }
      }
      fputc('\n', fpout);
   }
}

//Colour 

void set_colour(Program *p, char col)
{
   p->colour = col; 
}

bool word_is_colour(Program *p)
{
   if(word_matches(p, "\"RED\"")){
      return true;
   }
   if(word_matches(p, "\"BLACK\"")){
      return true;
   }
   if(word_matches(p, "\"GREEN\"")){
      return true;
   }
   if(word_matches(p, "\"BLUE\"")){
      return true;
   }
   if(word_matches(p, "\"YELLOW\"")){
      return true;
   }
   if(word_matches(p, "\"CYAN\"")){
      return true;
   }
   if(word_matches(p, "\"MAGENTA\"")){
      return true;
   }
   if(word_matches(p, "\"WHITE\"")){
      return true;
   }
   return false; 
}

char colour2char(Program *p)
{
   if(word_matches(p, "\"RED\"")){
      return RED;
   }
   if(word_matches(p, "\"BLACK\"")){
      return BLACK;
   }
   if(word_matches(p, "\"GREEN\"")){
      return GREEN;
   }
   if(word_matches(p, "\"BLUE\"")){
      return BLUE;
   }
   if(word_matches(p, "\"YELLOW\"")){
      return YELLOW;
   }
   if(word_matches(p, "\"CYAN\"")){
      return CYAN;
   }
   if(word_matches(p, "\"MAGENTA\"")){
      return MAGENTA;
   }
   if(word_matches(p, "\"WHITE\"")){
      return WHITE;
   }
   return 'Z'; //not sure what else to do here
}

char get_colour_char(Program *p)
{
   char col = ' ';
   col = p->colour;
   return col; 
}

int char2ansi(char col)
{
   int colour_code = 0;
   switch (col) {
      case 'R':
         colour_code = red;
         break;
      case 'B':
         colour_code = blue;
         break;
      case 'K':
         colour_code = black;
         break;
      case 'C':
         colour_code = cyan;
         break;
      case 'Y':
         colour_code = yellow;
         break;
      case 'G':
         colour_code = green;
         break;
      case 'M':
         colour_code = magenta;
         break;
      case 'W':
         colour_code = white;
         break;
   }
   return colour_code;
}

//set 

int char2index(char letter) //this function needs bounds checking! if char passed in is not between A and Z, this function should do nothing
{

   int index = letter - BASE_LETTER;
   return index; 
}

//Neill's stack https://github.com/csnwc/ADTs/tree/main/Stack

stack* stack_init(void)
{
   stack *s = (stack*) ncalloc(1, sizeof(stack));
   s->a = (stacktype*) ncalloc(FIXEDSIZE, sizeof(stacktype));
   s->size = 0;
   s->capacity= FIXEDSIZE;
   return s;
}

void stack_push(stack* s, stacktype d)
{
   if(s==NULL){
       return;
   }
   if(s->size >= s->capacity){
      s->a = (stacktype*) nremalloc(s->a,
             sizeof(stacktype)*s->capacity*SCALEFACTOR);
      s->capacity = s->capacity*SCALEFACTOR;
   }
   s->a[s->size] = d;
   s->size = s->size + 1;
}

bool stack_pop(stack* s, stacktype* d)
{
   if((s == NULL) || (s->size < 1)){
      return false;
   }
   s->size = s->size - 1;
   *d = s->a[s->size];
   return true;
}

bool stack_peek(stack* s, stacktype* d)
{
   if((s==NULL) || (s->size <= 0)){
      /* Stack is Empty */
      return false;
   }
   *d = s->a[s->size-1];
   return true;
}

bool stack_free(stack* s)
{
   if(s==NULL){
      return true;
   }
   free(s->a);
   free(s);
   return true;
}

//end of Neill's stack functions 

void set_num_val_var(Program *p, double val, int index)
{
   p->vars[index].is_set = true;
   p->vars[index].type = NUMBER;
   p->vars[index].data.num = val; 
}

double get_num_val_var(Program *p, int index)
{
   double val = -1;
   if(p->vars[index].is_set == false){
      p->exit_fail = true;
      //neillclrscrn();
      //printf("Failed to interpret: variable is not set? \n");
      //return EXIT_FAILURE;
   }
   if(p->vars[index].type == NUMBER){
       val = p->vars[index].data.num;
   }
   return val; 
}

void set_col_val_var(Program *p, int index)
{  
   char colour = colour2char(p);
   p->vars[index].is_set = true;
   p->vars[index].type = CHAR;
   p->vars[index].data.col = colour;
}

char get_col_val_var(Program *p, int index)
{
   char colour = 0;
   if(p->vars[index].is_set == false){
      p->exit_fail = true;
      //neillclrscrn();
      //printf("Failed to interpret: variable is not set? \n");
      //return EXIT_FAILURE;
   }
   if(p->vars[index].type == CHAR){
       colour = p->vars[index].data.col;
   }
   return colour;
}

//adapted from Neill's https://github.com/csnwc/ADTs/blob/main/Stack/postfix.c
void calc_binary_expression(Program *p, stack *s)
{
   char op = get_character(p);
   double top, top_minus_1, result; 
   stack_pop(s, &top);
   stack_pop(s, &top_minus_1);
      switch(op){
         case '+' :
            result = top_minus_1 + top;
            break;
         case '-' :
            result = top_minus_1 - top;
            break;
         case '*' :
            result = top_minus_1 * top;
            break;
         case '/' :
            result = top_minus_1 / top;  //handle case where divide by zero: exit gracefully 
            break;
         default:
            printf("Failed to interpret\n");
            exit(EXIT_FAILURE);
         }
   stack_push(s, result);
}

char get_character(Program *p)
{
   char c = p->wds[p->cw][0]; 
   return c; 
}

char var2letter(Program *p)
{
   char letter = p->wds[p->cw][1];
   return letter; 
}

//Loop 

int get_last_item_index(Program *p)
{
   int cnt = 0;
   int base_word_index = p->cw; 
   while(p->wds[p->cw][0] != '}'){
      next_word(p);
      cnt++;
   }
   int last_item_index = base_word_index + cnt - 1; 
   p->cw = base_word_index;
   return last_item_index; 
}

int get_first_item_index(Program *p)
{
   int cnt = 0; 
   int base_word_index = p->cw; 
   while(p->wds[p->cw][0] != '{'){
      next_word(p);
      cnt++;
   }
   int first_item_index = base_word_index + cnt + 1; 
   p->cw = base_word_index;
   return first_item_index; 
}

int get_loop_jump(int first_item_index, int last_item_index)
{
   int index_first_ins_word = last_item_index + LAST_TO_INS;
   int base_loop_jump = index_first_ins_word - first_item_index; 
   return base_loop_jump; 
}

void run_loop(Program *p, int itm_indx_1, int itm_indx_end, int vr_indx, int jmp)
{
   double num = -1;
   for(int cw_indx = itm_indx_1; cw_indx < itm_indx_end + 1; cw_indx++){
      p->cw = cw_indx; 
      if(Item(p)){
         if(get_double(p, &num)){
            set_num_val_var(p, num, vr_indx);
         }
         if(word_is_colour(p)){
            set_col_val_var(p, vr_indx);
         }
      }
      p->cw += jmp;
      if(Inslst(p)){ 
         jmp--;   
      }
   }
}

bool get_double(Program *p, double *result)
{
   if(sscanf(p->wds[p->cw], "%lf", result)== 1){
      return true; 
   }
   return false;
}

void run_simple_screen(Program *p)
{
   neillcursorhome();
   char col = get_colour_char(p);
   int ansi = char2ansi(col);
   neillfgcol(ansi);
   print_grid_screen(p);
   neillbusywait(1.0);
}

bool var_val_is_num(Program *p, int index)
{
   if(p->vars[index].type == NUMBER){
       return true;
   }
   if(p->vars[index].type == CHAR){
       return false;
   }
   return false; //this is a bit unsafe
}

bool var_val_is_col(Program *p, int index)
{
   if(p->vars[index].type == CHAR){
       return true;
   }
   if(p->vars[index].type == NUMBER){
       return false;
   }
   return false; //this is a bit unsafe
}

bool list_is_empty(Program *p)
{
   int baseline = p->cw;
   next_word(p);
   if(!word_matches(p, "{")){
      p->cw = baseline; 
      return false;
   }
   next_word(p);
   if(!word_matches(p, "}")){
      p->cw = baseline; 
      return false;
   }
   while(!word_matches(p, "END")){
      p->cw++;
   }
   return true;
}

//HELPER FUNCTIONS

void clear_buff(Program *p)
{
   for(int i = 0; i < MAXNUMTOKENS; i++){
      strcpy(p->wds[i], "");
   }
}

void rst_ptr(Program *p)
{
   p->cw = 0;
}

void next_word(Program *p)
{
   p->cw += 1;
}

void str2buff(Program *p, char tst[TSTSTRLEN], int num_wds) 
{
   int wrd_indx = 0; 
   int str_indx = 0; 
   while(sscanf(tst+str_indx,"%s",p->wds[wrd_indx])==1 && wrd_indx<num_wds-1){ 
      //move index to start of next word 
      while(isgraph(tst[str_indx])){
         str_indx++;
      }
      str_indx++; //increment once more to get past space
      wrd_indx++;
   }
}

bool word_matches(Program *p, char match[MAXTOKENSIZE])
{
   if(strcmp(p->wds[p->cw], match)==0){
      return true;
   }
   return false; 
}

void test(void)
{
   Program* prog = calloc(1, sizeof(Program));
   
   char tst[ROW_HEIGHT*COL_WIDTH+1];
   // *** INTERPRETING TESTS ***

   //get_character 
   
   clear_buff(prog);
   str2buff(prog, "+", 1);
   assert(get_character(prog)=='+');
   str2buff(prog, "-", 1);
   assert(get_character(prog)=='-');
   str2buff(prog, "*", 1);
   assert(get_character(prog)=='*');
   str2buff(prog, "/", 1);
   assert(get_character(prog)=='/');
   clear_buff(prog);

   //double calc_binary_expression
   stack *s;
   s = stack_init();

   str2buff(prog, "+", 1); // addition
   double x = 10;
   double y = 8;
   stack_push(s, x);
   stack_push(s, y); 
   calc_binary_expression(prog, s);
   double top_stack = -1; 
   stack_pop(s, &top_stack);   //check result is on top of stack
   assert(fabs(top_stack-18)<=0.00001);

   str2buff(prog, "-", 1); // subtraction
   stack_push(s, x);
   stack_push(s, y); 
   calc_binary_expression(prog, s);
   stack_pop(s, &top_stack);   //check result is on top of stack
   //assert(fabs(top_stack-2)<=0.00001);

   str2buff(prog, "*", 1); // multiplication
   stack_push(s, x);
   stack_push(s, y); 
   calc_binary_expression(prog, s);
   stack_pop(s, &top_stack);   //check result is on top of stack
   //assert(fabs(top_stack-80)<=0.00001);

   str2buff(prog, "/", 1); // division
   stack_push(s, x);
   stack_push(s, y); 
   calc_binary_expression(prog, s);
   stack_pop(s, &top_stack);   //check result is on top of stack
   //assert(fabs(top_stack-1.25)<=0.00001);

   stack_free(s); //don't init and free too many times: will make it really slow
   clear_buff(prog);

   //VERY IMPORTANT! - ensure that interpreter exits gracefully for stack overflow and underflow (trying to pop from empty stack)
   //also ensure fails when try to divide by zero

   //var2letter
   str2buff(prog, "$A", 1);
   assert(var2letter(prog)=='A');
   str2buff(prog, "$M", 1);
   assert(var2letter(prog)=='M');
   str2buff(prog, "$Z", 1);
   assert(var2letter(prog)=='Z');

   //word_is_colour
   str2buff(prog, "RED", 1); //red 
   assert(word_is_colour(prog));
   str2buff(prog, "YELLOW", 1); //yellow
   assert(word_is_colour(prog));
   str2buff(prog, "BLUE", 1); //blue
   assert(word_is_colour(prog));
   str2buff(prog, "GREEN", 1); //green
   assert(word_is_colour(prog));
   str2buff(prog, "MAGENTA", 1); //magenta
   assert(word_is_colour(prog));
   str2buff(prog, "BLACK", 1); //black
   assert(word_is_colour(prog));
   str2buff(prog, "CYAN", 1); //cyanexecute
   assert(word_is_colour(prog));
   str2buff(prog, "ORANGE", 1); //orange (not a valid colour)
   assert(!word_is_colour(prog));
  
   //char2index
   assert(char2index('A')==0);
   assert(char2index('B')==1);
   assert(char2index('Y')==24);
   assert(char2index('Z')==25);
   
   //empty grid

   assert(empty_grid(prog)==true);
   init_turtle(prog);
   grid2str(tst, prog);
   //test that there's a 'W' in the middle cell of the grid
   //assert(strcmp(tst, "                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         W                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         ")==0); 
   //test that x = mid, y = mid, direction = 0
      //create function to do this that takes x, y, and degrees as parameters
   //print_grid_screen(prog);

   //draw_forward
   
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORWARD 15 END", 4); 
   Prog(prog);
   //print_grid_screen(prog);
   clear_buff(prog);
   rst_ptr(prog);
 
   //deg2rad

   assert(fabs(deg2rad(90)-1.570796)<= 0.000001); //90 deg
   assert(fabs(deg2rad(45)-0.785398)<= 0.000001); //45 deg
   assert(fabs(deg2rad(0)-0.000000)<= 0.000001); //0 deg
   assert(fabs(deg2rad(100)-1.745329)<= 0.000001); //100 deg
   assert(fabs(deg2rad(270)-4.712389)<= 0.000001); //270 deg

   //TEST UNTESTED INTERPRETER FUNCTIONS, INCUDING VOID FUNCTIONS

   //TO DO ...............................................

   //validate_degree

   assert(fabs(validate_degree(-10)-350)<= 0.000001); //-10 deg (less than 0)
   assert(fabs(validate_degree(-43)-317)<= 0.000001); //-43 deg (less than 0)
   assert(fabs(validate_degree(-365)-355)<= 0.000001); //-365 deg (less than -360)
   assert(fabs(validate_degree(-90)-270)<= 0.000001); //-90 deg
   assert(fabs(validate_degree(-120)-240)<= 0.000001); //-120 deg

   //neg_deg_to_pos
   
   assert(fabs(neg_degree_to_pos(-12)-348)<= 0.000001); //-12 (less than 0)
   assert(fabs(neg_degree_to_pos(-365)-355)<= 0.000001); //-365 (less than -360)
   assert(fabs(neg_degree_to_pos(-735)-345)<= 0.000001); //-735 (less than -720)

   //get_delta_y 
   assert(fabs(get_delta_y(45, 10)-7.07107)<=0.00001); // between 0 and 90
   assert(fabs(get_delta_y(135, 5)-3.53553)<=0.00001); // between 90 and 180
   assert(fabs(get_delta_y(190, 4)+0.69459)<=0.00001); // between 180 and 270
   assert(fabs(get_delta_y(340, 12)+4.10424)<=0.00001); // between 270 and 360 

   //get_delta_x
   assert(fabs(get_delta_x(12, 8)-7.82518)<=0.00001); // between 0 and 90
   assert(fabs(get_delta_x(147, 5)+4.19335)<=0.00001); // between 90 and 180
   assert(fabs(get_delta_x(210, 6)+5.19615)<=0.00001); // between 180 and 270
   assert(fabs(get_delta_x(300, 7)-3.5)<=0.00001); // between 270 and 360 

   //get_new_y 
   prog->curr_y = 10;
   assert(get_new_y(prog, 3)==13);
   prog->curr_y = 13;
   assert(get_new_y(prog, 4)==17);

   //get_new_x 
   prog->curr_x = 6;
   assert(get_new_x(prog, 3)==9);
   prog->curr_x = 9;
   assert(get_new_x(prog, 5)==14);

   //is_y_in_bounds
   assert(is_y_in_bounds(4)); //in bounds
   assert(!is_y_in_bounds(33)); //over bounds
   assert(!is_y_in_bounds(-1)); //below bounds

   //is_x_in_bounds
   assert(is_y_in_bounds(7)); //in bounds
   assert(!is_y_in_bounds(51)); //over bounds
   assert(!is_y_in_bounds(-1)); //below bounds
   
   //set_num_val_var & get_num_val_var

   int index = char2index('A');
   set_num_val_var(prog, 1, index); //set A to 1 
   assert(fabs(get_num_val_var(prog, index)-1)<=0.00001); //get value of A

   set_num_val_var(prog, 5, index); //reset A to 5
   assert(fabs(get_num_val_var(prog, index)-5)<=0.00001); //get new value of A

   index = char2index('Z');
   set_num_val_var(prog, 17.99, index); //set Z to 17.99
   assert(fabs(get_num_val_var(prog, index)-17.99)<=0.00001); //get value of Z

   //set1.ttl 
   str2buff(prog,"START SET A ( 1 ) END", 7); 
   Prog(prog);
   assert(fabs(get_num_val_var(prog, 0)-1)<=0.00001); //check A is set to 3
   rst_ptr(prog);
   clear_buff(prog);

   //set2.ttl
   str2buff(prog,"START SET A ( 0 ) SET B ( $A 1 + ) SET C ( $B 2 * ) END", 21); 
   Prog(prog);
   assert(fabs(get_num_val_var(prog, 0)-0)<=0.00001); //check A is set to 0
   assert(fabs(get_num_val_var(prog, 1)-1)<=0.00001); //check B is set to 1 (= 0 + 1)
   assert(fabs(get_num_val_var(prog, 2)-2)<=0.00001); //check C is set to 2 (= 1 * 2)
   rst_ptr(prog);
   clear_buff(prog);

   // *** PARSING TESTS ***

   //NON-RECURSIVE FUNCTIONS
   
   //Num
   strcpy(prog->wds[0], "10");
   assert(Num(prog)==true);

   strcpy(prog->wds[0], "-17.99");
   assert(Num(prog)==true);
   //To do......

   strcpy(prog->wds[0], "d"); //not a double
   assert(Num(prog)==false);
   
   strcpy(prog->wds[0], "d.13"); //not a double
   assert(Num(prog)==false);

   //don't need to care about case 17.d etc. As long as it scans a number, it's fine

   //Op 
   
   strcpy(prog->wds[0], "+");
   assert(Op(prog)==true);

   strcpy(prog->wds[0], "-");
   assert(Op(prog)==true);

   strcpy(prog->wds[0], "/");
   assert(Op(prog)==true);

   strcpy(prog->wds[0], "*");
   assert(Op(prog)==true);
  
   strcpy(prog->wds[0], "?"); //non-valid punct
   assert(Op(prog)==false);
   
   strcpy(prog->wds[0], "a"); //letter
   assert(Op(prog)==false);

   strcpy(prog->wds[0], "1"); //number
   assert(Op(prog)==false);

   //Ltr
   
   strcpy(prog->wds[0], "A");
   assert(Ltr(prog, NO_VAR_CALL)==true);

   strcpy(prog->wds[0], "M");
   assert(Ltr(prog, NO_VAR_CALL)==true);

   strcpy(prog->wds[0], "Z");
   assert(Ltr(prog, NO_VAR_CALL)==true);

   strcpy(prog->wds[0], "a"); //lowercase
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "1"); //number
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "!"); //punct 
   assert(Ltr(prog, NO_VAR_CALL)==false);
   
   strcpy(prog->wds[0], "AA"); //double capital letter 
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "A!"); //cap letter then punct
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "Ab"); //cap letter then lowercase
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "A1"); //cap letter then number
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "1A"); //num then cap
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "$A"); //valid var: as if called from Var
   assert(Ltr(prog, VAR_CALL)==true);

   strcpy(prog->wds[0], "$A"); //valid var: as if not called from Var
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "$M"); //valid var: as if called from Var
   assert(Ltr(prog, VAR_CALL)==true);

   strcpy(prog->wds[0], "$M"); //valid var: as if not called from Var
   assert(Ltr(prog, NO_VAR_CALL)==false);

   strcpy(prog->wds[0], "$Z"); //valid var: as if called from Var
   assert(Ltr(prog, VAR_CALL)==true);

   strcpy(prog->wds[0], "$Z"); //valid var: as if not called from Var
   assert(Ltr(prog, NO_VAR_CALL)==false);

   //Word

   strcpy(prog->wds[0], "\"RED\""); //colour with ""
   assert(Word(prog)==true);

   strcpy(prog->wds[0], "RED\""); //colour missing first "
   assert(Word(prog)==false);

   strcpy(prog->wds[0], "\"RED"); //colour missing second "
   assert(Word(prog)==false);

   strcpy(prog->wds[0], "RED"); //colour without ""
   assert(Word(prog)==false);

   strcpy(prog->wds[0], "\"BLUE\""); 
   assert(Word(prog)==true);

   strcpy(prog->wds[0], "\"HELLO!\""); //other word (not interpretable)
   assert(Word(prog)==true);

   strcpy(prog->wds[0], "HELLO!"); //other word without ""
   assert(Word(prog)==false);

   strcpy(prog->wds[0], "\"178\""); //word that's a number
   assert(Word(prog)==true);

   strcpy(prog->wds[0], "178"); //word that's a number without ""
   assert(Word(prog)==false);

   strcpy(prog->wds[0], ""); //null string, no ""
   assert(Word(prog)==false);

   strcpy(prog->wds[0], "\"\""); //null string with ""
   assert(Word(prog)==false);
   // this is a bug I found through assert testing: previously input "\"\"" returned true

   //found significant bug where "\"RED" evaluates to true i.e. missing second "

   //account for case e.g. "RED YELLOW": function should fail for this (I think?)

   //what about case "\"RED YELLOW\"" or "\"RED \"YELLOW\"" etc: can use these as examples of bugs found through testing

   //RECURSIVE FUNCTIONS

   //Prog

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORWARD 10 END", 4); 
   assert(Prog(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORWARD 17.99 END", 4); 
   assert(Prog(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START RIGHT 10 END", 4); //RIGHT
   assert(Prog(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORWARD TEN END", 4); // TEN instead of 10
   assert(Prog(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORARD 10 END", 4); //mispelled FORWARD
   assert(Prog(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORWARD 10", 3); //no END statement
   assert(Prog(prog)==false);
 
   //Rgt

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT 10", 2);  //valid Num
   assert(Rgt(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT -17.99", 2); //valid Num
   assert(Rgt(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT $B", 2); //valid Var
   assert(Rgt(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT $Z", 2); //valid Var
   assert(Rgt(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT $a", 2); //invalid Var: lowercase
   assert(Rgt(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT $$", 2); //invalid Var: two $
   assert(Rgt(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGT -17.99", 2); //mispelled RIGHT 
   assert(Rgt(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT d.99", 2); //not a double
   assert(Rgt(prog)==false);

   //Fwd

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD 10", 2); //valid Num
   assert(Fwd(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD -17.99", 2); //valid Num
   assert(Fwd(prog)==true);
  
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD $B", 2); //valid Var
   assert(Fwd(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD $Z", 2); //valid Var
   assert(Fwd(prog)==true);
  
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD $a", 2); //invalid Var: lowercase
   assert(Fwd(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD $$", 2); //invalid Var: two $
   assert(Fwd(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FRWARD -17.99", 2); //mispelled FORWARD 
   assert(Fwd(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD d.99", 2); //not a double
   assert(Fwd(prog)==false);

   //Inslst

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "END", 1); // END
   assert(Inslst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "EN", 1); // mispelled END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START", 1); // not END or Ins 
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD 10 END", 3); // correct Ins (Fwd) + END
   assert(Inslst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT -35 END", 3); // correct Ins (Rgt) + END
   assert(Inslst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT -35", 2); // correct Ins (Rgt) with no END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD 6", 2); // correct Ins (Fwd) with no END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FRWARD 6 END", 3); // mispelled FORWARD with END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RGHT 7 END", 3); // mispelled RIGHT with END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT d END", 3); // Rgt (not a double) with END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT END", 2); // Rgt (missing double) with END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD x END", 3); // Fwd (not a double) with END
   assert(Inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD END", 2); // Fwd (missing double) with END
   assert(Inslst(prog)==false);
 
   //Ins 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD 10", 2); //valid FORWARD instruction
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT -17.99", 2); //valid RIGHT instruction
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FRWARD -17.99", 2); //mispelled FORWARD 
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD d.99", 2); //not a double
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGT -17.99", 2); //mispelled RIGHT 
   assert(Ins(prog)==false);
 
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT d.99", 2); //not a double
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR $A", 2); //valid Col instruction
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR $M", 2); //valid Col instruction: Var
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR $Z", 2); //valid Col instruction: Var
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOR $Z", 2); //mispelled COLOUR :(
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"RED\"", 2); //valid Col instruction: Word
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR RED", 2); //invalid Col instruction: Word missing ""
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"BLUE\"", 2); //valid Col instruction: Word
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"HELLO!\"", 2); //valid Col instruction: Word
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET A ( $A 0.25 - )", 7); // valid expression with sensible Pfix 
   assert(Ins(prog)==true);

   //found bug: "SET Z" or $Z causes sanitizer error. Increasing size of array from 25 to 26 fixed this

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET Z ( $A 0.25 - )", 7); // valid expression with sensible Pfix (different ltr)
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET Z ( $Z 0.25 - )", 7); // valid expression with sensible Pfix (different ltr)
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET Y ( $Z 0.25 - )", 7); // valid expression with sensible Pfix (different ltr)
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET Y ( $Q 0.25 - )", 7); // valid expression with sensible Pfix (different ltr)
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET 9 ( $A 0.25 - )", 7); // num instead of ltr 
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "ST Z ( $A 0.25 - )", 7); // mispelled SET
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET 9 [ $A 0.25 - )", 7); // wrong opening bracket
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET 9 ( $A 0.25 - ", 7); // missing Pfix )
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP M OVER { \"GREEN\" } END", 7); // valid Loop
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP A OVER { \"RED\" \"GREEN\" \"YELLOW\" \"BLUE\" } FORWARD $D RIGHT 90 END", 14); // valid Loop (long)
   assert(Ins(prog)==true);
 
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP U OVER { \"RED\" } END", 7); // valid Loop (one colour)
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOP M OVER { \"GREEN\" } END", 7); // mispelled LOOP
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP 9 OVER { \"GREEN\" } END", 7); // num instead of Ltr
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP $W OVER { \"GREEN\" } END", 7); // var instead of Ltr
   assert(Ins(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP $B OVR { 20 } END", 7); // mispelled OVER
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP $W OVER 20 } END", 6); // missing {
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP $Y OVER { 20 END", 6); // missing }
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP $Q OVER { 20 }", 6); // missing END (no Inslst)
   assert(over_lst_inslst(prog)==false);

   //Var 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A", 1); //first char is $
   assert(Var(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$M", 1); //first char is $
   assert(Var(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$Z", 1); //first char is $
   assert(Var(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$!", 1); //first char is $, 2nd is punct
   assert(Var(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "!A", 1); //first char is not $
   assert(Var(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$$", 1); //first & 2nd chars are $
   assert(Var(prog)==false);
   //found bug where this was valid. I had "$%[$A-Z]" instead of "$%[A-Z]"

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "M", 1); //letter, not var
   assert(Var(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "S", 1); //letter, not var
   assert(Var(prog)==false);

   //Varnum 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A", 1); //valid Var
   assert(Varnum(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$a", 1); //invalid Var
   assert(Varnum(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "-17.99", 1); //valid Num
   assert(Varnum(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10", 1); //valid Num
   assert(Varnum(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "A", 1); //invalid Num
   assert(Varnum(prog)==false);

   //Item 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A", 1); //valid Varnum: Var
   assert(Item(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10", 1); //valid Varnum: Num
   assert(Item(prog)==true);

   //how do I know the above valid num isn't a word that's missing its "" i.e. "10" is a valid word

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "\"RED\"", 1); //valid Word
   assert(Item(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "\"GREEN\"", 1); //valid Word
   assert(Item(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "\"178\"", 1); //valid Word (number)
   assert(Item(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "GREEN", 1); //invalid Word (no "")
   assert(Item(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "", 1); //invalid Word: null string
   assert(Item(prog)==false);

   //Col 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR $A", 2); //valid Col instruction
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR $M", 2); //valid Col instruction: Var
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR $Z", 2); //valid Col instruction: Var
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOR $Z", 2); //mispelled COLOUR :(
   assert(Col(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"RED\"", 2); //valid Col instruction: Word
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR RED", 2); //invalid Col instruction: Word missing ""
   assert(Col(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"BLUE\"", 2); //valid Col instruction: Word
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"HELLO!\"", 2); //valid Col instruction: Word
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR HELLO!", 2); //invalid Col instruction: Word missing ""
   assert(Col(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR \"178\"", 2); //valid Col instruction: Word
   assert(Col(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR 178", 2); //invalid Col instruction: Word missing ""
   assert(Col(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "COLOUR ", 1); //missing instruction after COLOUR
   assert(Col(prog)==false);
 

   //Pfix 

   //Note that the grammar seems to allow all sorts of strange instructions and does not even
   // state that the operator must come after the varnum (i.e. does not ensure postfix)
   // For now I'm assuming these weird instructions will parse OK but not be interpreted 

   stack *pfix_stack;
   pfix_stack = stack_init();
  
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, ")", 1); // ) only 
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "", 1); //null string  
   assert(Pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 - )", 4); //interpretable instruction
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "0.25 $A - )", 4); //swapped var and num
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 ! )", 4); //incorrect operator
   assert(Pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$a 0.25 / )", 4); //incorrect var
   assert(Pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 -", 3); //missing )
   assert(Pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A RED - )", 4); //word instead of varnum
   assert(Pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 - ]", 4); //wrong type of closing bracket
   assert(Pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "* $A )", 3); //operator then var [not interpretable]
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "+ - / * )", 5); //lots of operators followed by ) [not interpretable]
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "+ )", 2); //operator then ) [not interpretable]
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10 )", 2); //number then ) [not interpretable]
   assert(Pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$Q )", 2); //var then ) [not interpretable]
   assert(Pfix(prog, pfix_stack)==true);
 
   //Items

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "}", 1); // } only
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, ")", 1); // } wrong closing bracket only
   assert(Items(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A }", 2); // var then } 
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10 }", 2); // num then } 
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "\"RED\" }", 2); // word then } 
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RED }", 2); // word no "" then } 
   assert(Items(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A", 1); // var no } 
   assert(Items(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "-17.99", 1); // num no } 
   assert(Items(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "\"RED\"", 1); // word no } 
   assert(Items(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 17 }", 3); // multiple varnum then }
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10 $M }", 3); // multiple varnum then }
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10 $M \"RED\" \"BLACK\" }", 5); // multiple varnum and word then }
   assert(Items(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10 $M RED \"BLACK\" }", 5); // multiple varnum and word then } (one word missing "")
   assert(Items(prog)==false);
 
   //Set 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET A ( $A 0.25 - )", 7); // valid expression with sensible Pfix 
   assert(Set(prog)==true);

 
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET Z ( $A 0.25 - )", 7); // valid expression with sensible Pfix (different ltr)
   assert(Set(prog)==true);
  
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET 9 ( $A 0.25 - )", 7); // num instead of ltr 
   assert(Set(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "ST Z ( $A 0.25 - )", 7); // mispelled SET
   assert(Set(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET 9 [ $A 0.25 - )", 7); // wrong opening bracket
   assert(Set(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET 9 ( $A 0.25 - ", 7); // missing Pfix )
   assert(Set(prog)==false);

   //brace_then_pfix

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "( $A 0.25 - )", 5); // ( then Pfix
   assert(brace_then_pfix(prog, pfix_stack)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 - )", 4); // no ( then Pfix
   assert(brace_then_pfix(prog, pfix_stack)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "( $A 0.25 - ", 4); // ( then Pfix with missing )
   assert(brace_then_pfix(prog, pfix_stack)==false);
 
   //Lst 

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ $A }", 3); // valid Lst: one var 
   assert(Lst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ $A $M }", 4); // valid Lst: two vars 
   assert(Lst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ $A -17.99 }", 4); // valid Lst: one var one num
   assert(Lst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ \"RED\" }", 3); // valid Lst: one word
   assert(Lst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ RED }", 3); // invalid Lst: word missing ""
   assert(Lst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ \"BLACK\" \"RED\" }", 4); // valid Lst: two words
   assert(Lst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ \"BLACK\" $Q }", 4); // valid Lst: one word one var
   assert(Lst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ 10 \"GREEN\" }", 4); // valid Lst: one num one word

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A }", 2); // invalid: missing { 
   assert(Lst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "( $A }", 3); // invalid: wrong opening bracket
   assert(Lst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "{ 10 \"GREEN\"", 3); // invalid Items: missing }
   assert(Lst(prog)==false);
   
   //Loop
   
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP M OVER { \"GREEN\" } END", 7); // valid Loop
   assert(Loop(prog)==true);

//found bug from this next code block: in Fwd, was reading in $ from var instead of the char, and this was being converted to index -29

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP A OVER { \"RED\" \"GREEN\" \"YELLOW\" \"BLUE\" } FORWARD $D RIGHT 90 END", 14); // valid Loop (long)
   assert(Loop(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP U OVER { \"RED\" } END", 7); // valid Loop (one colour)
   assert(Loop(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOP M OVER { \"GREEN\" } END", 7); // mispelled LOOP
   assert(Loop(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP 9 OVER { \"GREEN\" } END", 7); // num instead of Ltr
   assert(Loop(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "LOOP $W OVER { \"GREEN\" } END", 7); // var instead of Ltr
   assert(Loop(prog)==false);

   //found a bug where Ltr function thinks $W is a valid letter, because of the way I wrote it to be called recursively as part of Var


   //over_lst_inslst

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { 20 } END", 5); // num then END
   assert(over_lst_inslst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVR { 20 } END", 5); // mispelled OVER
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER 20 } END", 4); // missing {
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { 20 END", 4); // missing }
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { 20 }", 4); // missing END (no Inslst)
   assert(over_lst_inslst(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { $M } END", 5); // var then END
   assert(over_lst_inslst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { \"RED\" } END", 5); // word then END
   assert(over_lst_inslst(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { 20 $M \"RED\" } END", 7); // num then var then word then END
   assert(over_lst_inslst(prog)==true);


   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "OVER { \"RED\" \"GREEN\" \"YELLOW\" \"BLUE\" } FORWARD $D RIGHT 90 END", 12); // long expression: colours, forward, right
   assert(over_lst_inslst(prog)==true);
 
   //when Ins is expanded to include Col etc, retest this function with this string 
   //str2buff(prog, "OVER { \"RED\" \"GREEN\" \"YELLOW\" \"BLUE\" } COLOUR $C FORWARD $D RIGHT 90 END", 14); // long expression
  
  
   // HELPER FUNCTIONS 

   //word_matches 
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "GREEN", 1);
   assert(word_matches(prog, "GREEN")==true);

   str2buff(prog, "}", 1); 
   assert(word_matches(prog, "}")==true);

   str2buff(prog, "}", 1); 
   assert(word_matches(prog, ")")==false); //different bracket

   str2buff(prog, "BLUE", 1); 
   assert(word_matches(prog, "RED")==false); //different word

   //clear_buff

   str2buff(prog, "START", 1); // set one word
   clear_buff(prog); //clear one word
   assert(word_matches(prog, "")==true);
   rst_ptr(prog);

   str2buff(prog, "START END", 2); // set two words
   clear_buff(prog); //clear two words
   assert(word_matches(prog, "")==true);
   rst_ptr(prog);

   str2buff(prog, "START OVER END", 3); // set two words
   clear_buff(prog); //clear two words
   assert(word_matches(prog, "")==true);
   rst_ptr(prog);

   str2buff(prog, "ONE TWO THREE FOUR FIVE SIX SEVEN EIGHT NINE TEN", 10); // set 10 words
   clear_buff(prog); //clear 10 words
   assert(word_matches(prog, "")==true);
   rst_ptr(prog);

   //rst_ptr

   str2buff(prog, "FORWARD 10", 2); 
   assert(word_matches(prog, "FORWARD")==true); //current word is 1st
   Fwd(prog); //Fwd moves ptr on one
   assert(word_matches(prog, "10")==true); //current word is 2nd
   rst_ptr(prog);
   assert(word_matches(prog, "FORWARD")==true); //current word is 1st again

   str2buff(prog, "START RIGHT 11", 3); 
   Prog(prog); //Prog moves ptr on 
   rst_ptr(prog);
   assert(word_matches(prog, "START")==true); //current word is 1st again

   str2buff(prog, "SET A ( $A 0.25 - )", 7);
   Set(prog); //Set moves ptr on
   rst_ptr(prog);
   assert(word_matches(prog, "SET")==true); //current word is 1st again

   //next_word

   str2buff(prog, "SET A ( $A 0.25 - )", 7);
   assert(word_matches(prog, "SET")==true); //first word is "SET"
   next_word(prog);
   assert(word_matches(prog, "A")==true); //2nd word is "A"
   next_word(prog);
   assert(word_matches(prog, "(")==true); //3rd word is "("
   next_word(prog);
   assert(word_matches(prog, "$A")==true); //4th word is "$A"
   next_word(prog);
   assert(word_matches(prog, "0.25")==true); //5th word is "0.25"
   next_word(prog);
   assert(word_matches(prog, "-")==true); //6th word is "-"
   next_word(prog);
   assert(word_matches(prog, ")")==true); //7th word is ")"
   rst_ptr(prog);

   //str2buff

   str2buff(prog, "SET B ( $W 10 * )", 7);
   assert(strcmp(prog->wds[prog->cw], "SET")==0); //1st word is "SET"
   next_word(prog);
   assert(strcmp(prog->wds[prog->cw], "B")==0); //2nd word is "B"
   next_word(prog);
   assert(strcmp(prog->wds[prog->cw], "(")==0); //3rd word is "("
   next_word(prog);
   assert(strcmp(prog->wds[prog->cw], "$W")==0); //4th word is "$W"
   next_word(prog);
   assert(strcmp(prog->wds[prog->cw], "10")==0); //5th word is "10"
   next_word(prog);
   assert(strcmp(prog->wds[prog->cw], "*")==0); //6th word is "*"
   next_word(prog);
   assert(strcmp(prog->wds[prog->cw], ")")==0); //7th word is ")"

   //testing for str2buff and next_word is a bit circular... 

   //get_arg_filename

   //NEED TO TEST THIS WITH SHELL SCRIPT (black box) .....................................
   
   stack_free(pfix_stack);

   free(prog);
   
}
