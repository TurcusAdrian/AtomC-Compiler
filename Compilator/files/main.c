#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "ad.h"

int main()
{
    char *inbuf = loadFile("tests/testad.c");
    // puts(inbuf);
    Token *tokens = tokenize(inbuf);
    free(inbuf);
    // showTokens(tokens);
    pushDomain();
    parse(tokens);
    showDomain(symTable, "global"); // afisare domeniu global
    dropDomain();
    free(tokens);
    return 0;
}
