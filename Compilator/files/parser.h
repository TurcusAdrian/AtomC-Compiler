#pragma once

#include "lexer.h"
#include <stdbool.h>
void parse(Token *tokens);




bool typeBase();

bool exprPrimary();

bool exprPostfixPrim();

bool exprPostfix();

bool exprUnary();

bool exprAssign();

bool expr();

bool stmCompound(bool newDomain);

bool stm();

bool fnParam();

bool fnDef();

bool arrayDecl();

bool varDef();

bool structDef();

bool unit();

bool exprMulPrim();

bool exprMul();

bool exprAddPrim();

bool exprAdd();

bool exprRelPrim();

bool exprRel();

bool exprEqPrim();

bool exprEq();

bool exprAndPrim();

bool exprAnd();

bool exprOrPrim();

bool exprOr();
