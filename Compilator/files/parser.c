#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"


Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token

void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

bool consume(int code){
  printf("consume(%s)",tokenName(code));
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		printf(" => consumed\n");
		return true;
	}
	printf(" => found %s\n",tokenName(iTk->code));
	return false;
	}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(){
   printf("#typeBase\n");
   Token *start=iTk;
	if(consume(TYPE_INT)){
		return true;
		}
	else if(consume(TYPE_DOUBLE)){
		return true;
		}
	else if(consume(TYPE_CHAR)){
		return true;
		}
	else if(consume(STRUCT)){
		if(consume(ID)){
			return true;
		}else tkerr("Missing name for the struct");
		}
	iTk=start;
	return false;
	}

bool arrayDecl(){
  printf("#arrayDecl\n");
  Token *start=iTk;
  if(consume(LBRACKET)){
    if(consume(INT)){}
    if(consume(RBRACKET)){
      return true;
    }else tkerr("Missing ] from array declaration");
  }
  iTk=start;
  return false;
}

bool varDef(){
   printf("#varDef\n");
   Token *start=iTk;
  if(typeBase()){
    if(consume(ID)){
     if(arrayDecl()){}
     if(consume(SEMICOLON)){
       return true;
     } else tkerr("Missing ; from variable definition");
    } else tkerr("Missing name(id) from variable declaration");
  }
  iTk=start;
  return false;
}

bool expr(){
  printf("#expr %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprAssign()){
    return true;
  }
  iTk=start;
  return false;
}

bool exprAssign(){
  printf("#exprAssign %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprUnary()){
    if(consume(ASSIGN)){
      if(exprAssign()){
        return true;
      } else tkerr("Missing expression after = sign");
    }
    iTk=start;
  }
  
  if(exprOr()){
    return true;
  }
  iTk=start;
  return false;
}


bool exprOr(){
  printf("#exprOr %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprAnd()){
    if(exprOrPrim()){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprOrPrim(){
  if(consume(OR)){
    if(exprAnd()){
      if(exprOrPrim()){
	return true;
      }
    }else tkerr("Missing expression after ||");
  }
  return true; // epsilon
}

bool exprAnd(){
 printf("#exprAnd %s\n",tokenName(iTk->code));
 Token *start=iTk;
 if(exprEq()){
   if(exprAndPrim()){
     return true;
   }
 }
 iTk=start;
 return false;
}

bool exprAndPrim(){
  if(consume(AND)){
    if(exprEq()){
      if(exprAndPrim()){
	return true;
      }
    }else tkerr("Missing expression after &&");
  }
  return true;//epsilon
}

bool exprEq(){
  printf("#exprEq %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprRel()){
    if(exprEqPrim()){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprEqPrim(){
  if(consume(EQUAL)||consume(NOTEQ)){
    if(exprRel()){
      if(exprEqPrim()){
	return true;
      }
    }else tkerr("Missing expression after = or != sign");
  }
  return true; //epsilon
}

bool exprRel(){
  printf("#exprRel %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprAdd()){
    if(exprRelPrim()){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprRelPrim(){
  if(consume(LESS)||consume(LESSEQ)||consume(GREATER)||consume(GREATEREQ)){
    if(exprAdd()){
      if(exprRelPrim()){
	return true;
      }
    }else tkerr("Missing expression after comparison operator");
  }
  return true;
}

bool exprAdd(){
  printf("#exprAdd %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprMul()){
    if(exprAddPrim()){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprAddPrim(){
  if(consume(ADD)||consume(SUB)){
    if(exprMul()){
      if(exprAddPrim()){
	return true;
      }
    }else tkerr("Missing expression after + or - operator");
  }
  return true; //epsilon
}

bool exprCast(){
  printf("#exprCast %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(consume(LPAR)){
    if(typeBase()){
      if(arrayDecl()){}
      if(consume(RPAR)){
	if(exprCast()){
	  return true;
	}
      }else tkerr("Missing ) from expression");
    }
  }
  if(exprUnary()){
    return true;
  }
  iTk=start;
  return false;
}


bool exprMulPrim(){
  if(consume(MUL)||consume(DIV)){
    if(exprCast()){
      if(exprMulPrim()){
	return true;
      }
    }else tkerr("Missing expression after * or / operator");
  }
  return true;
}

bool exprMul(){
  printf("#exprMul %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprCast()){
    if(exprMulPrim()){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprUnary(){
  printf("#exprUnary %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(consume(SUB)|| consume(NOT)){
    if(exprUnary()){
      return true;
    }else tkerr("Missing operator - or ! in expression");
  }
  if(exprPostfix()){
    return true;
  }
  iTk=start;
  return false;
}

bool exprPostfixPrim(){
  Token *start=iTk;
  if(consume(LBRACKET)){
    if(expr()){
      if(consume(RBRACKET)){
	if(exprPostfixPrim()){
	  return true;
	}
      }else tkerr("Missing ] from expression");
    }
    iTk=start;
  }
  if(consume(DOT)){
    if(consume(ID)){
      if(exprPostfixPrim()){
	return true;
      }
    }else tkerr("Missing name after . ");
    iTk=start;
  }
  return true; //epsilon
}

bool exprPostfix(){
  printf("#exprPostfix %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprPrimary()){
    if(exprPostfixPrim()){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprPrimary(){
  printf("#exprPrimary\n");
  Token* start = iTk;
  if(consume(ID)){
   if(consume(LPAR)){
    if(expr()){
     while(consume(COMMA)){
      if(expr()){}
      else tkerr("Missing expression after ,");
     }
    }else tkerr("Missing expression after (");
     if(consume(RPAR)){
      return true;
     }else tkerr("Missing )");
   }
    return true;
  }
  else if(consume(DOUBLE)){
    return true;
  }
  else if(consume(CHAR)){
    return true;
  }
  else if(consume(STRING)){
    return true;
  }
  else if(consume(INT)){
    return true;
  }
  else if(consume(LPAR)){
    if(expr()){
     if(consume(RPAR)){
      return true;
     }else tkerr("Missing )");
    }else tkerr("Missing expression after (");
  }
 iTk = start;
 return false;
}

    
bool stm(){
	printf("#stm\n");
	Token* start= iTk;
	if(stmCompound()){
		return true;
	}
	if(consume(IF)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						if(consume(ELSE)){
							if(stm()) {
                                return true;
                            } else {
                                tkerr("Else statement missing");
                            }
						}
						return true;
					}else tkerr("If statement missing");
				}else tkerr("Missing ) for if");
			}else tkerr("Missing if expression");
		}else tkerr("Missing ( after if");
		iTk=start;
	}
	else if(consume(WHILE)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						return true;
					}else tkerr("Missing while body");
				}else tkerr("Missing ) for while");
			}else tkerr("Missing while condition");
		}else tkerr("Missing ( after while keyword");
		iTk=start;
	}
	else if(consume(RETURN)){
		if(expr()){}
		if(consume(SEMICOLON)){
			return true;
		}else tkerr("Missing ; at return statement");
		iTk=start;
	}
	else if(expr()) {
        if(consume(SEMICOLON)){
            return true;
        } 
	}
	iTk = start;
	return false;
}


bool stmCompound(){
	printf("StmCompound\n");
	Token* start = iTk;
	if(consume(LACC)){
			for(;;){
			if(varDef()){}
			else if(stm()){}
			else break;
		}
		if(consume(RACC)){
			return true;
		}else tkerr("Missing } for a statement");
	}
	iTk = start;
	return false;
}

	
bool fnParam(){
  printf("#fnParam\n");
  Token *start=iTk;
  if(typeBase()){
    if(consume(ID)){
      if(arrayDecl()){}
      return true;
    }else tkerr("Missing function parameter name");
  }
  iTk=start;
  return false;
}

bool fnDef(){
  printf("#fnDef\n");
  Token *start= iTk;
  if(typeBase() || consume(VOID)){
   if(consume(ID)){
    if(consume(LPAR)){
     if(fnParam()){
       while(consume(COMMA)){
        if(fnParam()){}
        else tkerr("Expected type specifier in function after ,");
       }
      }
     if(consume(RPAR)){
      if(stmCompound()){
       return true;
      }else tkerr("Missing body function");
     }else tkerr("Missing ) in function");
    }
   }else tkerr("Missing name(id) for function");
  }
  iTk=start;
  return false;
}
 
bool structDef(){
  printf("#structDef\n");
  Token *start=iTk;
  if(consume(STRUCT)){
    if(consume(ID)){
	if(consume(LACC)){
	  while(varDef()){}
	  if(consume(RACC)){
	      if(consume(SEMICOLON)){
		return true;
	      } else tkerr("Missing ; from struct definition");
	  } else tkerr("Missing } from struct initialisation");
	}
    } else tkerr("Missing struct name");
  }
  iTk=start;
  return false;
}


// unit: ( structDef | fnDef | varDef )* END
bool unit(){
  printf("#unit\n");
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
	} else tkerr("End of file not reached!");
  return false;
}

void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
}
