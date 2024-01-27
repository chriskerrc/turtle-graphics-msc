#include "parse.h"

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
      i++;
   }
   
   if(Prog(prog)){
      fclose(fp);
      free(prog); 
      return EXIT_SUCCESS;
   }
     
   printf("Failed to parse\n");
   fclose(fp);
   free(prog); 
   return EXIT_FAILURE;
}

bool Prog(Program *p)
{
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
   if(word_matches(p, "FORWARD")){ 
      next_word(p);
      if(Varnum(p)){
         return true;
      }       
   }   
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
   //should this function only accept capital letters e.g. "RED" not "red"? - comment to say what I did 
   int len = strlen(WDS_CW);
   char c[CHARBUFFLEN];
   if(sscanf(WDS_CW, "%s", c)==1 && WDS_CW[1]== '\"'){
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
      if(sscanf(WDS_CW, "%[A-Z]", c)==1 && WDS_CW[1]== '\0'){ 
      return true;
      }
   }
   //letter in a variable
   if(is_var_call == VAR_CALL){
      if(sscanf(WDS_CW, "$%[A-Z]", c)==1 && WDS_CW[CHAR_OF_LTR]== '\0'){ 
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
   //Question: should this function enforce that word is valid colour?
   //Currently, nothing in the parser enforces what is valid colour, but this is part of the grammar...
   //comment about what I did here...
   
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
   return false;
}

bool Pfix(Program *p)
{
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
   if(!word_matches(p, "SET")){
      return false; 
   }
   next_word(p);
   if(!Ltr(p, NO_VAR_CALL)){
      return false; 
   }
   next_word(p);
   if(brace_then_pfix(p)){
      return true;
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
   if(!word_matches(p, "LOOP")){
      return false; 
   }
   next_word(p);
   if(!Ltr(p, NO_VAR_CALL)){
      return false; 
   }
   next_word(p);
   if(over_lst_inslst(p)){
      return true;
   }
   return false; 
}

bool over_lst_inslst(Program *p)
{
   if(!word_matches(p, "OVER")){
      return false;
   }
   next_word(p);
   if(!Lst(p)){
      return false;
   }
   next_word(p);
   if(Inslst(p)){
      return true;
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

   // To do: reduce number of times I clear buffer and reset pointer. Much of the time this is redundant and probably slows things down

   // To do: make assert testing exhaustive

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

   //FREE

   free(prog);
   
}
