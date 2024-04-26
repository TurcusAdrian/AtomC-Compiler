#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "lexer.h"
//#include "parser.h"
//#include "parser.c"

int main(){
  char *inbuf=loadFile("tests/testlex.c");
  puts(inbuf);
  Token *tokens=tokenize(inbuf);
  showTokens(tokens);
  free(inbuf);
  //  parser(tokens);
  return 0;
}
