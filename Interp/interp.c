#include "interp.h"
#include "../neillsimplescreen.c"

//run sanitizer etc 
//is extremely long line in assert test ok?
//think about what happens in empty loop: Neill might test for this. for file fail_parse_ok_interp.ttl, make sure you don't stop looking for ENDs after the first one. Need to make sure there's a closing END for each opening LOOP
//currently my output for donothing.ttl is the turtle in start position, but it should print nothing: only init turtle under certain conditions
//need to check that output .txt file name is same as input .ttl file (with addition of out_)?
//should the interpreter run when parsing fails in this file? or only when it succeeds?
//handle case where direction is more than 360 
//TO DO: do diff on my txt files vs Neill's to check right number of columns etc. 

int main(int argc, char *argv[]) //make main function shorter
{
   test();
   Program* prog = calloc(1, sizeof(Program));
   int i=0;
   init_turtle(prog); //added init so turtle for read in file is initialized (need to only do this if turtle does something)
   if(argc == 3){
      prog->is_text_output=true;
   }
   else{
      prog->is_text_output=false; 
   }

   if(argc != 2 && argc != 3){ //magic numbers
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
      //printf("%s\n", prog->wds[i]); //seems to keep reading in after it gets to end of file?? 
      i++;
   }

   if(Prog(prog)){
      //printf("Parsed OK\n");
      fclose(fpin);
      if(prog->is_text_output==true){
         write_file;
      }
      free(prog); 
      return EXIT_SUCCESS;
   }

   //printf("Not parsed OK\n");
   fclose(fpin);
   free(prog); 
   return EXIT_FAILURE;
}

bool Prog(Program *p)
{
   neillclrscrn(); //not sure if this is right place to call this. was thinking to call this as early as possible...
   //printf("Prog word: %s\n", p->wds[p->cw]);
   if(!word_matches(p, "START")){
      //ERROR("No START statement ?");
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
   //if word is "END", return true and don't increment p
   if(word_matches(p, "END")){
      return true;
   }
   if(Ins(p)){
      next_word(p);
      if(Inslst(p)){
         return true;
      }
   }
   //ERROR("Inslst failed");
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
   //ERROR("Ins failed");
   return false;
}

bool Fwd(Program *p)
{   
   double distance = 0;
   //printf("%s\n", p->wds[p->cw]);
   if(word_matches(p, "FORWARD")){ 
      next_word(p);
      if(Varnum(p)){
         if(Num(p)){
            if(sscanf(p->wds[p->cw], "%lf", &distance)== 1){
               draw_forward(p, distance);
               if(p->is_text_output==false){
                  neillcursorhome();
                  print_grid_screen(p);
                  neillbusywait(1.0);
               }
               //printf("calling draw_forward\n");
               return true;
            }
         }
         //get var or num for interpreter
         //call draw_forward with this num or var 
         return true;
      }       
   }   
   //ERROR("Fwd failed");
   return false;
}

bool Rgt(Program *p)
{
   double new_direction = 0; 
   if(word_matches(p, "RIGHT")){
      next_word(p);
      if(Varnum(p)){
         if(Num(p)){ //this code is copied from Fwd: make it a function instead
            if(sscanf(p->wds[p->cw], "%lf", &new_direction)== 1){
               //double valid_direction = validate_degree(new_direction);
               change_direction(p, new_direction);
               //printf("calling change direction\n");
               return true;
            }
         }
         return true;
      }       
   }   
   //ERROR("Rgt failed");
   return false;
}

bool Num(Program *p)
{
   double d;
   if(sscanf(p->wds[p->cw], "%lf", &d)==1){  //for every sscanf like this, replace p->wds[p->cw] with function that hides this ugly syntax
      return true;
   }
   //printf("%lf", d);
   //ERROR("Num failed");
   return false;
}

bool Op(Program *p)
{   
   char c[CHARBUFFLEN];
   if(sscanf(p->wds[p->cw], "%[+*-/]", c)==1){    //magic num
      return true;
   }
   return false;
}

bool Word(Program *p) // long lines
{
   //should this function only accept capital letters e.g. "RED" not "red"? 
   int len = strlen(p->wds[p->cw]);
   char c[CHARBUFFLEN];
   if(sscanf(p->wds[p->cw], "%s", c)==1 && p->wds[p->cw][1]== '\"'){
      return false; //to ensure input "\"\"" returns false
   }
   if(len > 1){ //to avoid len-1 going out of bounds on null "" string
      if(sscanf(p->wds[p->cw], "%s", c)==1 && p->wds[p->cw][0]== '\"' && p->wds[p->cw][len-1] == '\"'){
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
      if(sscanf(p->wds[p->cw], "%[A-Z]", c)==1 && p->wds[p->cw][1]== '\0'){ 
      return true;
      }
   }
   //letter in a variable
   if(is_var_call == VAR_CALL){
      if(sscanf(p->wds[p->cw], "$%[A-Z]", c)==1 && p->wds[p->cw][2]== '\0'){ //2 = magic num 
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
   //ERROR("Varnum failed");
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
   //ERROR("Item failed");
   return false;
}

bool Col(Program *p)
{
   //Question: should this function enforce that word is valid colour?
   //Currently, nothing in the parser enforces what is valid colour, but this is part of the grammar...
   
   //printf("Col: %s\n", p->wds[p->cw]);
   if(word_matches(p, "COLOUR")){
      next_word(p);
      if(Var(p)){
         return true;
      }
      else{
         if(Word(p)){
            return true;
         }
      }       
   }   
   //ERROR("Col failed");
   return false;
}

bool Pfix(Program *p)
{
   //if word is ")", return true and don't increment p
   if(word_matches(p, ")")){
      return true;
   }
   
   if(Op(p)){
      next_word(p);
      if(Pfix(p)){
         return true;
      }
   }
   else{
      if(Varnum(p)){
         next_word(p);
         if(Pfix(p)){
            return true;
         }
      }
   } 

   //ERROR("Pfix failed");
   return false;
}

bool Items(Program *p)
{
   //if word is "}", return true and don't increment p
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
   if(word_matches(p, "SET")){
      next_word(p);
      if(Ltr(p, NO_VAR_CALL)){
         next_word(p);
         if(brace_then_pfix(p)){
            return true;
         }
      }
   }
   return false; 
}

bool brace_then_pfix(Program *p)
{
   if(word_matches(p, "(")){
      next_word(p);
      if(Pfix(p)){
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
   if(word_matches(p, "LOOP")){
      next_word(p);
      if(Ltr(p, NO_VAR_CALL)){
         next_word(p);
         if(over_lst_inslst(p)){
            return true;
         }
      }
   }
   return false; 
}

bool over_lst_inslst(Program *p)
{
   if(word_matches(p, "OVER")){
      next_word(p);
      if(Lst(p)){
         next_word(p);
         if(Inslst(p)){
            return true;
         }
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
      //p->grid[MID_ROW][MID_COL] = WHITE;
      p->curr_y = MID_ROW;
      p->curr_x = MID_COL;
      p->curr_direction = ROTATE_CONST; 
   }
}

void print_grid_screen(Program *p)
{  
   for(int row = 0; row < ROW_HEIGHT; row++){
      for(int col = 0; col < COL_WIDTH; col++){
         if(isalpha(p->grid[row][col])){
            printf("%c", p->grid[row][col]);
         }
         else{
            printf(" "); //print a space
         }
      }
      printf("\n");
   }
   printf("\n");
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
   p->curr_y += y_delta; //should the coordinates be rounded to ints before printing e.g. now?
   //printf("updated y %lf\n", p->curr_y);
}

void update_x_position(Program *p, double x_delta)
{
   p->curr_x += x_delta; //should the coordinates be rounded to ints before printing e.g. now?
   //printf("updated x %lf\n", p->curr_x);
}

double deg2rad(double deg)
{
   double rad = deg * (PI/RAD_CONST);
   return rad; 
}

void change_direction(Program *p, double new_direction)
{
   //printf("new direction in change dir func %lf\n", new_direction);
   double curr_direction = p->curr_direction;
   p->curr_direction = new_direction + curr_direction; 
   //printf("updated direction in change dir func %lf\n", p->curr_direction);
}

double validate_degree(double deg) //not sure where to use this function. trying to do that seems to break things, but not checking this will probably cause bugs...
{
   double new_angle = 0;
   if(deg > 0 && deg > MAX_ANGLE){
      new_angle = deg; 
   }
   if(deg > MAX_ANGLE){
      new_angle = fmod(deg,MAX_ANGLE); 
   }
   if(deg < 0){
      new_angle = neg_degree_to_pos(deg);
   }
   return new_angle; 
}

double neg_degree_to_pos(double deg)
{
   double new_angle = deg;
   while(new_angle < 0){
      new_angle = new_angle + MAX_ANGLE;
   }
   return new_angle; 
}

double get_delta_y(double direction, double distance)
{
   double angle_rad = deg2rad(direction);
   double delta_y = distance * sin(angle_rad);
   return delta_y; 
}

double get_delta_x(double direction, double distance)
{
   double angle_rad = deg2rad(direction);
   double delta_x = distance * cos(angle_rad);
   return delta_x; 
}

int get_new_y(Program *p, double delta_y)
{
   int new_y = round(p->curr_y + delta_y);
   return new_y; 
}

int get_new_x(Program *p, double delta_x)
{
   int new_x = round(p->curr_x + delta_x);
   return new_x;
}


//this function is way too long: break it up 
//line drawing algorithm adapted from .js here https://github.com/anushaihalapathirana/Bresenham-line-drawing-algorithm/blob/master/src/index.js
//slight bug when line is drawn downwards e.g. for octagon. maybe end points of line are wrong? not just in-between points affected, so need to fix this...
void draw_line(Program *p, int y_start, int x_start, int y_end, int x_end)
{
   //printf("y_start %i\n", y_start);
   //printf("x_start %i\n", x_start);
   //printf("y_end %i\n", y_end);
   //printf("x_end %i\n", x_end);

   //calculate dy and dx
   int abs_dx = abs(x_end - x_start); 
   int abs_dy = abs(y_end - y_start);
   int dx = x_end - x_start;
   int dy = y_end - y_start; 

   int x = x_start; 
   int y = y_start; 
   //plot starting points
   plot_pixel(p, y, x);

   int error_term = 0;
   //slope < 1
   if(abs_dx > abs_dy){
      error_term = ERROR_CONST*abs_dy - abs_dx;
      for(int i = 0; i < abs_dx; i++){
         if(dx < 0){
            x--; 
         }
         else{
            x++;
         }
         if(error_term < 0){
            error_term = error_term + ERROR_CONST*abs_dy; 
         }
         else{
            if(dy < 0){
               y--;
            }
            else{
               y++;
            }
            error_term = error_term + (ERROR_CONST*abs_dy - ERROR_CONST*abs_dx);
         }
         plot_pixel(p, y, x);
      }
   }
   //slope is greater than or equal to 1
   if(abs_dx <= abs_dy){
      error_term = ERROR_CONST*abs_dx - abs_dy;
      for(int i = 0; i < abs_dy; i++){
         if(dy < 0){
            y--;
         }
         else{
            y++;
         }
         if(error_term < 0){
            error_term = error_term + ERROR_CONST*abs_dx;
         }
         else{
            if(dx < 0){
               x--; 
            }
            else{
               x++;
            }
            error_term = error_term + (ERROR_CONST*abs_dx) - (ERROR_CONST*abs_dy);
         }
         plot_pixel(p, y, x);
      }
   }
}

void plot_pixel(Program *p, int curr_y_plot, int curr_x_plot)
{
   p->grid[curr_y_plot][curr_x_plot] = WHITE;
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

void draw_forward(Program *p, double distance)
{
   double raw_direction = p->curr_direction;
   //printf("raw direction %lf\n", raw_direction);
   //double valid_direction = validate_degree(raw_direction);
   //printf("valid direction %lf\n", valid_direction);
   int start_y = round(p->curr_y);
   int start_x = round(p->curr_x);
   //printf("start y %i\n", start_y);
   //printf("start x %i\n", start_x);
   double delta_x = get_delta_x(raw_direction, distance);
   double delta_y = get_delta_y(raw_direction, distance);
   //printf("delta y %lf\n", delta_y);
   //printf("delta x %lf\n", delta_x);
   int end_y = get_new_y(p, delta_y);
   int end_x = get_new_x(p, delta_x);
   //printf("end y %i\n", end_y);
   //printf("end x %i\n", end_x);

   //round x and y values to integers 

   if(is_y_in_bounds(start_y) && is_x_in_bounds(start_x) && is_y_in_bounds(end_y) && is_x_in_bounds(end_x)){ //long line
      draw_line(p, start_y, start_x, end_y, end_x);
   }
   else{
      printf("Not in bounds\n");
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

//HELPER FUNCTIONS

void clear_buff(Program *p)
{
   //zero out first 100 words of buffer 
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

void str2buff(Program *p, char tst[TSTSTRLEN], int numwords) 
{
   int word_index = 0; 
   int string_index = 0; 
   while(sscanf(tst + string_index, "%s", p->wds[word_index])==1 && word_index<numwords-1){ //why -1? this doesn't make sense to me
      //move index to start of next word 
      while(isgraph(tst[string_index])){
         string_index++;
      }
      string_index++; //deleted parsing tests for now. probably need to add them back in (copy from parse.c)
//increment once more to get past space
      word_index++;
   }
}

bool word_matches(Program *p, char match[MAXTOKENSIZE])
{
   if(strcmp(p->wds[p->cw], match)==0){
      return true;
   }
   return false; 
}

//IMPORTANT: make sure testing doesn't print loads of irrelevant grids /stuff to the screen
void test(void)
{
   Program* prog = calloc(1, sizeof(Program));
   /*
   // To do: reduce number of times I clear buffer and reset pointer. Much of the time this is redundant and probably slows things down

   // To do: make assert testing exhaustive

   // *** INTERPRETING TESTS ***
   
   char tst[ROW_HEIGHT*COL_WIDTH+1];
   
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

   //offset_deg

   assert(fabs(validate_degree(-10)-350)<= 0.000001); //-10 deg (less than 0)
   assert(fabs(validate_degree(-43)-317)<= 0.000001); //-43 deg (less than 0)
   assert(fabs(validate_degree(-365)-355)<= 0.000001); //-365 deg (less than -360)

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

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET A ( $A 0.25 - + / 99 $M )", 11); // valid expression with weird but valid Pfix 
   assert(Ins(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "SET Z ( $A 0.25 - )", 7); // valid expression with sensible Pfix (different ltr)
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

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, ")", 1); // ) only 
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "", 1); //null string  
   assert(Pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 - )", 4); //interpretable instruction
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "0.25 $A - )", 4); //swapped var and num
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 ! )", 4); //incorrect operator
   assert(Pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$a 0.25 / )", 4); //incorrect var
   assert(Pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 -", 3); //missing )
   assert(Pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A RED - )", 4); //word instead of varnum
   assert(Pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 - ]", 4); //wrong type of closing bracket
   assert(Pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "* $A )", 3); //operator then var [not interpretable]
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "+ - / * )", 5); //lots of operators followed by ) [not interpretable]
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "+ )", 2); //operator then ) [not interpretable]
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "10 )", 2); //number then ) [not interpretable]
   assert(Pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$Q )", 2); //var then ) [not interpretable]
   assert(Pfix(prog)==true);

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
   str2buff(prog, "SET A ( $A 0.25 - + / 99 $M )", 11); // valid expression with weird but valid Pfix 
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
   assert(brace_then_pfix(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "$A 0.25 - )", 4); // no ( then Pfix
   assert(brace_then_pfix(prog)==false);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "( $A 0.25 - ", 4); // ( then Pfix with missing )
   assert(brace_then_pfix(prog)==false);

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
   */
   free(prog);
   
}