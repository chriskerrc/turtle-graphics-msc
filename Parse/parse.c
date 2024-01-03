// ask Neill: is it OK to borrow from 0 & 1 code https://github.com/csnwc/Notes/blob/main/Code/ChapX/0and1s.c
// yes, this is fine but this code doesn't conform to style guide, reads stuff in in a hacky way i.e. change it later

// do research on how to test 
// e.g. black box testing: can I break your program, not knowing how it works 

//take out error calling. either return 0 or 1

//don't need to care about case 17.d etc. As long as it scans a number, it's fine

// check I'm using tabs not spaces (VS Code)

// consider substituting "p->wds[p->cw]" with something more readable
// relatedly, watch out for long lines i.e. && separated expressions in functions

// IMPORTANT: run sanitizer and valgrind on lab machines! 

// what happens if there are no spaces between "words" in the ttl file. should parse fail?

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
#define ERROR(PHRASE) { fprintf(stderr, \
          "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
          __FILE__, __LINE__); \
          exit(EXIT_FAILURE); }
#define MAXFILENAME 50
#define VAR_CALL 1
#define NO_VAR_CALL 0

struct prog{
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];
   int cw; // Current Word <-rename to something readable
};
typedef struct prog Program;

bool Prog(Program *p); //move out to header file etc
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

bool get_arg_filename(int argc, char *argv[], char* filename);
void clear_buff(Program *p);
void str2buff(Program *p, char* tst, int numwords);
void rst_pt(Program *p);
void next_word(Program *p); 
bool word_matches(Program *p, char match[MAXTOKENSIZE]);
void test(void);

int main(int argc, char *argv[])
{
   test();
   Program* prog = calloc(1, sizeof(Program));
   int i=0;
   char filename[MAXFILENAME]; 
  
   if(get_arg_filename(argc, argv, filename) != true){
      fprintf(stderr, "Didn't get filename\n");
      exit(EXIT_FAILURE);
   }

   FILE* fp = fopen(filename, "r");
   if(fp == NULL){
       fprintf(stderr, "Cannot read file %s ?\n", filename);
       exit(EXIT_FAILURE);
   }
   
   while(fscanf(fp, "%s", prog->wds[i])==1 && i<MAXNUMTOKENS){ //check that fscanf returns something
      //printf("%s\n", prog->wds[i]); //seems to keep reading in after it gets to end of file?? 
      i++;
   }
   
   if(Prog(prog)){
      printf("Parsed OK\n");
      fclose(fp);
      free(prog); 
      return 0;
   }
     
    printf("Not parsed OK\n");
    fclose(fp);
    free(prog); 
    return 1;
}

bool Prog(Program *p)
{
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
   //printf("%s\n", p->wds[p->cw]);
   if(word_matches(p, "FORWARD")){ 
      next_word(p);
      if(Varnum(p)){
         return true;
      }       
   }   
   //ERROR("Fwd failed");
   return false;
}

bool Rgt(Program *p)
{
   if(word_matches(p, "RIGHT")){
      next_word(p);
      if(Varnum(p)){
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


//HELPER FUNCTIONS

bool get_arg_filename(int argc, char *argv[], char* filename)
{
   if(argc == 2){
      if(sscanf(argv[1], "%s", filename)==1){
         return true;
      }
   }
   return false;
}

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
      string_index++; //increment once more to get past space
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

void test(void)
{
   Program* prog = calloc(1, sizeof(Program));

   // To do: reduce number of times I clear buffer and reset pointer. Much of the time this is redundant and probably slows things down

   //NON-RECURSIVE FUNCTIONS
   
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

   //Add assert testing for helper functions (i.e. not Grammar functions)

   //FREE

   free(prog);
   
}
