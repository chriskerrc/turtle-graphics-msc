// ask Neill: is it OK to borrow from 0 & 1 code https://github.com/csnwc/Notes/blob/main/Code/ChapX/0and1s.c
// yes, this is fine but this code doesn't conform to style guide, reads stuff in in a hacky way i.e. change it later

// do research on how to test 
// e.g. black box testing: can I break your program, not knowing how it works 

//forward.ttl works for cut down grammar 

//take out error calling. either return 0 or 1

//don't need to care about case 17.d etc. As long as it scans a number, it's fine

/*

<PROG>   ::= "START" <INSLST>

<INSLST> ::= "END" | <INS> <INSLST>
<INS>    ::= <FWD> | <RGT>

<FWD>    ::= "FORWARD" <NUM>
<RGT>    ::= "RIGHT" <NUM>

<NUM>    ::= 10 or -17.99 etc.

*/

//start with NUM function and work backwards
 
//don't add one in forward, right or num function 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define MAXNUMTOKENS 100
#define MAXTOKENSIZE 20
#define strsame(A,B) (strcmp(A, B)==0) //change this
#define ERROR(PHRASE) { fprintf(stderr, \
          "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
          __FILE__, __LINE__); \
          exit(EXIT_FAILURE); }
#define FNAME "forward.ttl"

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
void test(void);

int main(void)
{
  
   Program* prog = calloc(1, sizeof(Program));
   int i=0;

   FILE* fp = fopen(FNAME, "r");
   if(fp == NULL){
       fprintf(stderr, "Cannot readfile %s ?\n", FNAME);
       exit(EXIT_FAILURE);
   }
   
   while(fscanf(fp, "%s", prog->wds[i])==1 && i<MAXNUMTOKENS){ //check that fscanf returns something
      //printf("%s\n", prog->wds[i]);      
      i++;
   }
   if(Prog(prog)==true){
      printf("Parsed OK\n");
      return 0;
   }
   else{
      printf("Not parsed OK\n");
      return 1;
   }
   
   free(prog);
   fclose(fp);
   test();

   
   return 1; //return 0 for parsed and 1 for not parsed
}

bool Prog(Program *p)
{
   //printf("%i\n", p->cw);
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
   printf("%lf", d);
   //ERROR("Num failed");
   return false;
}

void test(void)
{
   Program* prog = calloc(1, sizeof(Program));
   
   //Prog
   strcpy(prog->wds[0], "START");
   assert(Prog(prog)==true);

   //Num
   strcpy(prog->wds[0], "10");
   assert(Num(prog)==true);

   strcpy(prog->wds[0], "-17.99");
   assert(Num(prog)==true);

   strcpy(prog->wds[0], "d"); //not a double
   assert(Num(prog)==false);
   
   strcpy(prog->wds[0], "d.13"); //not a double
   assert(Num(prog)==false);

   //Rgt
   strcpy(prog->wds[0], "RIGHT");
   strcpy(prog->wds[1], "10");
   assert(Rgt(prog)==true);

   //printf("str 0 %s\n", prog->wds[0]);
   //printf("str 1 %s\n", prog->wds[1]);
   strcpy(prog->wds[0], "RIGHT");
   strcpy(prog->wds[1], "-17.99");
   
   //printf("str 0 %s\n", prog->wds[0]);
   //printf("str 1 %s\n", prog->wds[1]);
   assert(Rgt(prog)==true);

   strcpy(prog->wds[0], "RIGT"); //mispelled RIGHT
   strcpy(prog->wds[1], "-17.99");
   assert(Rgt(prog)==false);

   strcpy(prog->wds[0], "RIGHT");
   strcpy(prog->wds[1], "d.99"); //not a double
   assert(Rgt(prog)==false);

   //Fwd
  
   //add assert testing
   
   //free

   free(prog);

}
