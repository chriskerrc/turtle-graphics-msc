#include "interp.h"
#include "../neillsimplescreen.h"

//move Neill's stack to a separate file (treat it as ADT): feedback

int main(int argc, char *argv[])
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
   
   FILE* fpin = fopen(argv[1], "r");
   if(fpin == NULL){
       fprintf(stderr, "Cannot read file %s ?\n", argv[1]);
       exit(EXIT_FAILURE);
   }
   
   while(fscanf(fpin, "%s", prog->wds[i])==1 && i<MAXNUMTOKENS){ 
      i++;
   }

   if(Prog(prog)){
      fclose(fpin);
      if(prog->is_text_output==true){
         write_file(argv, prog);
      }
      free(prog); 
      exit(EXIT_SUCCESS);
   }
   fclose(fpin);
   free(prog); 
   fprintf(stderr, "Failed to interpret\n");
   exit(EXIT_FAILURE);
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
   if(Var(p)){ 
      char letter = var2letter(p);
      int var_index = char2index(letter);
      if(!var_is_set(p, var_index)){
         return false; 
      }
      if(var_val_is_num(p, var_index)){
         double distance = get_num_val_var(p, var_index); 
         draw_forward(p, distance);
         if(p->is_text_output==false){
            run_simple_screen(p);
         }
         return true;
         }
      else{ 
         return false; 
      }
   }
   return false;
}       

bool Rgt(Program *p) 
{
   double new_direction = 0; 
   if(word_matches(p, "RIGHT")){
      next_word(p);
      if(Varnum(p)){
         if(Num(p)){
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

bool Word(Program *p)
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
         return true;
      }
      else{
         return false; 
      }
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

bool Pfix(Program *p, stack *s) 
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
      if(Var(p)){ 
         letter = var2letter(p);
         var_index = char2index(letter);
         if(!var_val_is_num(p, var_index)){
            return false; 
         }
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
   if(brace_then_pfix(p, pfix_stack)){
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

bool over_lst_inslst(Program *p) 
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
   return 'Z';
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

int char2index(char letter) 
{
   if(letter >= 'A' && letter <= 'Z'){
      int index = letter - BASE_LETTER;
      return index; 
   }
   fprintf(stderr, "Failed to interpret: invalid letter\n");
   exit(EXIT_FAILURE);
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
      neillclrscrn();
      fprintf(stderr, "Failed to interpret: variable is not set?\n");
      exit(EXIT_FAILURE);
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
      neillclrscrn();
      fprintf(stderr, "Failed to interpret: variable is not set?\n");
      exit(EXIT_FAILURE);
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
   double top = 0;
   double top_minus_1 = 0;
   double result = 0; 
   stacktype peek = 0;
   if(!stack_peek(s, &peek)){
      fprintf(stderr, "Failed to interpret\n");
      exit(EXIT_FAILURE);
   }
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
         result = top_minus_1 / top; 
         break;
      default:
         fprintf(stderr, "Failed to interpret\n");
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
   return false; 
}

bool var_val_is_col(Program *p, int index)
{
   if(p->vars[index].type == CHAR){
       return true;
   }
   if(p->vars[index].type == NUMBER){
       return false;
   }
   return false; 
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

bool var_is_set(Program *p, int index)
{
   if(p->vars[index].is_set != true){
      return false;
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
   assert(fabs(top_stack-2)<=0.00001);

   str2buff(prog, "*", 1); // multiplication
   stack_push(s, x);
   stack_push(s, y); 
   calc_binary_expression(prog, s);
   stack_pop(s, &top_stack);   //check result is on top of stack
   assert(fabs(top_stack-80)<=0.00001);

   str2buff(prog, "/", 1); // division
   stack_push(s, x);
   stack_push(s, y); 
   calc_binary_expression(prog, s);
   stack_pop(s, &top_stack);   //check result is on top of stack
   assert(fabs(top_stack-1.25)<=0.00001);

   stack_free(s); 
   clear_buff(prog);

   //var2letter
   str2buff(prog, "$A", 1);
   assert(var2letter(prog)=='A');
   str2buff(prog, "$M", 1);
   assert(var2letter(prog)=='M');
   str2buff(prog, "$Z", 1);
   assert(var2letter(prog)=='Z');

   //word_is_colour
   str2buff(prog, "\"RED\"", 1); //red 
   assert(word_is_colour(prog));
   str2buff(prog, "\"YELLOW\"", 1); //yellow
   assert(word_is_colour(prog));
   str2buff(prog, "\"BLUE\"", 1); //blue
   assert(word_is_colour(prog));
   str2buff(prog, "\"GREEN\"", 1); //green
   assert(word_is_colour(prog));
   str2buff(prog, "\"MAGENTA\"", 1); //magenta
   assert(word_is_colour(prog));
   str2buff(prog, "\"BLACK\"", 1); //black
   assert(word_is_colour(prog));
   str2buff(prog, "\"CYAN\"", 1); //cyanexecute
   assert(word_is_colour(prog));
   str2buff(prog, "\"ORANGE\"", 1); //orange (not a valid colour)
   assert(!word_is_colour(prog));
  
   //char2index
   assert(char2index('A')==0);
   assert(char2index('B')==1);
   assert(char2index('Y')==24);

   //draw_forward
   
   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "START FORWARD 15 END", 4); 
   Prog(prog);
   clear_buff(prog);
   rst_ptr(prog);

   //deg2rad

   assert(fabs(deg2rad(90)-1.570796)<= 0.000001); //90 deg
   assert(fabs(deg2rad(45)-0.785398)<= 0.000001); //45 deg
   assert(fabs(deg2rad(0)-0.000000)<= 0.000001); //0 deg
   assert(fabs(deg2rad(100)-1.745329)<= 0.000001); //100 deg
   assert(fabs(deg2rad(270)-4.712389)<= 0.000001); //270 deg

   //validate_degree

   assert(fabs(validate_degree(-10)-350)<= 0.000001); //-10 deg (less than 0)
   assert(fabs(validate_degree(-43)-317)<= 0.000001); //-43 deg (less than 0)
   assert(fabs(validate_degree(-90)-270)<= 0.000001); //-90 deg
   assert(fabs(validate_degree(-120)-240)<= 0.000001); //-120 deg

   //neg_deg_to_pos
   
   assert(fabs(neg_degree_to_pos(-12)-348)<= 0.000001); //-12 (less than 0)

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

   //update_x_position
   prog->curr_x = 2;
   update_x_position(prog, 7);
   assert(fabs(prog->curr_x-9)<=0.00001);

   prog->curr_x = 10;
   update_x_position(prog, -3);
   assert(fabs(prog->curr_x-7)<=0.00001);

   //update_y_position
   prog->curr_y = 4;
   update_y_position(prog, 2);
   assert(fabs(prog->curr_y-6)<=0.00001);
   
   prog->curr_y = 8;
   update_y_position(prog, -2);
   assert(fabs(prog->curr_y-6)<=0.00001);

   //is_y_in_bounds
   assert(is_y_in_bounds(4)); //in bounds
   assert(!is_y_in_bounds(33)); //over bounds
   assert(!is_y_in_bounds(-1)); //below bounds

   //is_x_in_bounds
   assert(is_y_in_bounds(7)); //in bounds
   assert(!is_y_in_bounds(51)); //over bounds
   assert(!is_y_in_bounds(-1)); //below bounds

   //x_and_y_in_bounds
   assert(x_and_y_in_bounds(7, 4)); //in bounds
   assert(!x_and_y_in_bounds(51, -1)); //out bounds

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

   //char2ansi
   assert(char2ansi('R')==red);
   assert(char2ansi('B')==blue);
   assert(char2ansi('Y')==yellow);
   assert(char2ansi('M')==magenta);
   assert(char2ansi('K')==black);
   assert(char2ansi('G')==green);
   assert(char2ansi('W')==white);

   //char2index
   assert(char2index('A')==0);
   assert(char2index('C')==2);
   assert(char2index('Z')==25);
   
   //char colour2char, set_col_val_var, get_col_val_var, var_val_is_num, var_val_is_col

   clear_buff(prog);
   str2buff(prog, "\"RED\"", 1); 
   assert(colour2char(prog)=='R');
   assert(!var_is_set(prog, 3)); //check 'D' is not set
   set_col_val_var(prog, 3); //set 'D' val
   assert(get_col_val_var(prog, 3)=='R');
   assert(var_is_set(prog, 3)); //check 'D' is set
   assert(!var_val_is_num(prog, 3)); // D is not a num
   assert(var_val_is_col(prog, 3)); // D is a colour
   clear_buff(prog);

   str2buff(prog, "\"YELLOW\"", 1); 
   assert(colour2char(prog)=='Y');
   assert(!var_is_set(prog, 4)); //check 'E' is not set
   set_col_val_var(prog, 4); //set 'E' val
   assert(get_col_val_var(prog, 4)=='Y');
   assert(var_is_set(prog, 4)); //check 'D' is set
   assert(!var_val_is_num(prog, 4)); // E is not a num
   assert(var_val_is_col(prog, 4)); // E is a colour
   clear_buff(prog);

   str2buff(prog, "\"BLACK\"", 1); 
   assert(colour2char(prog)=='K');
   assert(!var_is_set(prog, 5)); //check 'F' is not set
   set_col_val_var(prog, 5); //set 'E' val
   assert(get_col_val_var(prog, 5)=='K');
   assert(var_is_set(prog, 5)); //check 'F' is set
   assert(!var_val_is_num(prog, 5)); // F is not a num
   assert(var_val_is_col(prog, 5)); // F is a colour
   clear_buff(prog);
   
   //set_colour_char & get_colour_char
 
   set_colour(prog, 'R');
   assert(get_colour_char(prog)=='R');
   set_colour(prog, 'K');
   assert(get_colour_char(prog)=='K');
   set_colour(prog, 'G');
   assert(get_colour_char(prog)=='G');
   set_colour(prog, 'B');
   assert(get_colour_char(prog)=='B');
   set_colour(prog, 'K');
   assert(get_colour_char(prog)=='K');

   //set_num_val_var, get_num_val_var, var_is_set, var_val_is_num
   
   assert(!var_is_set(prog, 16)); //check 'Q' is not set
   set_num_val_var(prog, 20, 16); //set 'Q' val
   assert(fabs(get_num_val_var(prog, 16)-20)<=0.00001); //get 'Q' val
   assert(var_is_set(prog, 16)); //check 'Q' is now set
   assert(var_val_is_num(prog, 16)); //check 'Q' is num

   assert(!var_is_set(prog, 24)); //check 'Y' is not set
   set_num_val_var(prog, 12, 24); //set 'Y' val
   assert(fabs(get_num_val_var(prog, 24)-12)<=0.00001); //get 'Q' val
   assert(var_is_set(prog, 24)); //check 'Y' is now set
   assert(var_val_is_num(prog, 24)); //check 'Y' is num

   //get_last_item_index, get_first_item_index, get_loop_jump
   clear_buff(prog);
   str2buff(prog, "OVER { \"RED\" \"GREEN\" \"YELLOW\" \"BLUE\" } COLOUR $C FORWARD 5 RIGHT 90 END", 14); 
   assert(get_last_item_index(prog)==5);
   assert(get_first_item_index(prog)==2);
   assert(get_loop_jump(2, 5)==5);
   rst_ptr(prog);
   clear_buff(prog);

   str2buff(prog, "OVER { 1 2 3 4 5 6 7 8 9 } COLOUR $C FORWARD 5 RIGHT 90 END", 18); 
   assert(get_last_item_index(prog)==10);
   assert(get_first_item_index(prog)==2);
   assert(get_loop_jump(2, 10)==10);
   rst_ptr(prog);
   clear_buff(prog);

   //list_is_empty
   str2buff(prog, "OVER { } COLOUR $C FORWARD 5 RIGHT 90 END", 10); //empty list
   assert(list_is_empty(prog));
   rst_ptr(prog);
   clear_buff(prog);

   str2buff(prog, "OVER { 1 } COLOUR $C FORWARD 5 RIGHT 90 END", 11); //not empty list
   assert(!list_is_empty(prog));
   rst_ptr(prog);
   clear_buff(prog);

   //get_double 

   double num = 0; 
   str2buff(prog, "4", 1); 
   assert(get_double(prog, &num));
   assert(fabs(num-4)<=0.00001);

   str2buff(prog, "12", 1); 
   assert(get_double(prog, &num));
   assert(fabs(num-12)<=0.00001);

   //init_turtle

   init_turtle(prog);
   assert(fabs(prog->curr_y-16)<=0.00001);
   assert(fabs(prog->curr_x-25)<=0.00001);
   assert(fabs(prog->curr_direction+90)<=0.00001);
   assert(get_colour_char(prog)=='W');

   //change_direction 
   change_direction(prog, 120);
   assert(fabs(prog->curr_direction-30)<=0.00001);
   change_direction(prog, 10);
   assert(fabs(prog->curr_direction-40)<=0.00001);
   change_direction(prog, 20);
   assert(fabs(prog->curr_direction-60)<=0.00001);
   change_direction(prog, -5);
   assert(fabs(prog->curr_direction-55)<=0.00001);

   //empty_grid
   assert(empty_grid(prog)); //grid is empty  
   plot_pixel(prog, 5, 6); 
   assert(!empty_grid(prog)); //grid is not empty  

   //draw line, draw_gentle_line, draw_steep_line
   set_colour(prog, 'W');
   draw_line(prog, 20, 10, 3, 14); //steep line
   assert(prog->grid[20][10] == 'W'); //start steep line
   assert(prog->grid[3][14] == 'W'); //end steep line
  
   draw_line(prog, 20, 20, 15, 35); //gentle line
   assert(prog->grid[20][20] == 'W'); //start steep line
   assert(prog->grid[15][35] == 'W'); //end steep line

   //PARSER FUNCTIONS UNCHANGED IN INTERPRETER 

   //Num
   strcpy(prog->wds[0], "10");
   assert(Num(prog)==true);

   strcpy(prog->wds[0], "-17.99");
   assert(Num(prog)==true);

   strcpy(prog->wds[0], "d"); //not a double
   assert(Num(prog)==false);
   
   strcpy(prog->wds[0], "d.13"); //not a double
   assert(Num(prog)==false);

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

   free(prog);
   
}
