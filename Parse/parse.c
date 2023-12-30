// ask Neill: is it OK to borrow from 0 & 1 code https://github.com/csnwc/Notes/blob/main/Code/ChapX/0and1s.c
// yes, this is fine but this code doesn't conform to style guide, reads stuff in in a hacky way i.e. change it later

// do research on how to test 
// e.g. black box testing: can I break your program, not knowing how it works 

//forward.ttl works for cut down grammar 

//take out error calling. either return 0 or 1

//don't need to care about case 17.d etc. As long as it scans a number, it's fine

// is it ok to use Neill's strsame

// check I'm using tabs not spaces (VS Code)

// consider substituting "p->wds[p->cw]" with something more readable

// IMPORTANT: run sanitizer and valgrind on lab machines! 

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
#define strsame(A,B) (strcmp(A, B)==0) //change this
#define ERROR(PHRASE) { fprintf(stderr, \
          "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
          __FILE__, __LINE__); \
          exit(EXIT_FAILURE); }
#define MAXFILENAME 50

struct prog{
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];
   int cw; // Current Word
};
typedef struct prog Program;

bool Prog(Program *p); //move out to header file etc
bool Inslst(Program *p);
bool Num(Program *p);
bool Rgt(Program *p);
bool Fwd(Program *p);
bool Ins(Program *p);
bool Op(Program *p);
bool Ltr(Program *p);
bool get_arg_filename(int argc, char *argv[], char* filename);
void clear_buff(Program *p);
void str2buff(Program *p, char* tst, int numwords);
void rst_pt(Program *p);
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
   
   if(Prog(prog)==true){
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
   if(!strsame(p->wds[p->cw], "START")){
      //ERROR("No START statement ?");
      return false;
   }
   p->cw = p->cw + 1;
   if (Inslst(p)) {
      return true;   
   }
   return false;
   
}

bool Inslst(Program *p)
{
   //if word is "END", return true and don't increment p
   if(strsame(p->wds[p->cw], "END")){
      return true;
   }
   if(Ins(p)==true){
      p->cw = p->cw + 1;
      if(Inslst(p)==true){
         Inslst(p);
         return true;
      }
   }
   //ERROR("Inslst failed");
   return false;
}

bool Ins(Program *p)
{
   if(Fwd(p)==true){
      return true;
   }
   else{
      if(Rgt(p)==true){
         return true;
      }
   }
   //ERROR("Ins failed");
   return false;
}

bool Fwd(Program *p)
{
   //printf("%s\n", p->wds[p->cw]);
   if(strsame(p->wds[p->cw], "FORWARD")){
      p->cw = p->cw + 1;
      if(Num(p)==true){
         return true;
      }       
   }   
   //ERROR("Fwd failed");
   return false;
}

bool Rgt(Program *p)
{
   if(strsame(p->wds[p->cw], "RIGHT")){
      p->cw = p->cw + 1;
      if(Num(p)==true){
         return true;
      }       
   }   
   //ERROR("Rgt failed");
   return false;
}

bool Num(Program *p)
{
   double d;
   if(sscanf(p->wds[p->cw], "%lf", &d)==1){
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

bool Ltr(Program *p)
{
   char c[CHARBUFFLEN];
    if(sscanf(p->wds[p->cw], "%[A-Z]", c)==1 && p->wds[p->cw][1]== '\0'){  
      return true;
   }
   return false;
}

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

void test(void)
{
   Program* prog = calloc(1, sizeof(Program));

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
   assert(Ltr(prog)==true);

   strcpy(prog->wds[0], "M");
   assert(Ltr(prog)==true);

   strcpy(prog->wds[0], "Z");
   assert(Ltr(prog)==true);

   strcpy(prog->wds[0], "a"); //lowercase
   assert(Ltr(prog)==false);

   strcpy(prog->wds[0], "1"); //number
   assert(Ltr(prog)==false);

   strcpy(prog->wds[0], "!"); //punct 
   assert(Ltr(prog)==false);
   
   strcpy(prog->wds[0], "AA"); //double capital letter 
   assert(Ltr(prog)==false);

   strcpy(prog->wds[0], "A!"); //cap letter then punct
   assert(Ltr(prog)==false);

   strcpy(prog->wds[0], "Ab"); //cap letter then lowercase
   assert(Ltr(prog)==false);

   strcpy(prog->wds[0], "A1"); //cap letter then number
   assert(Ltr(prog)==false);

   strcpy(prog->wds[0], "1A"); //num then cap
   assert(Ltr(prog)==false);

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
   str2buff(prog, "RIGHT 10", 2);
   assert(Rgt(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "RIGHT -17.99", 2);
   assert(Rgt(prog)==true);

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
   str2buff(prog, "FORWARD 10", 2);
   assert(Fwd(prog)==true);

   clear_buff(prog);
   rst_ptr(prog);
   str2buff(prog, "FORWARD -17.99", 2);
   assert(Fwd(prog)==true);

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

   //Add assert testing for helper functions (i.e. not Grammar functions)

   //free

   free(prog);
   
}
