#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"

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
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		return true;
		}
	return false;
	}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(){
	if(consume(TYPE_INT)){
		return true;
		}
	if(consume(TYPE_DOUBLE)){
		return true;
		}
	if(consume(TYPE_CHAR)){
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID)){
			return true;
			}
		}
	return false;
	}

bool exprPrimary(){
  if(consume(ID)){
    if(consume(LPAR)){
      if(expr()){
	while(if(consume(COMMA)||expr())){}
      }
      if(consume(LPAR)){}
    }
    return true;
    }
  if(consume(INT) || consume(DOUBLE) || consume(CHAR) || consume(STRING) || (if(consume(LPAR)){if(expr()){if(consume(RPAR)){}}})){
    return true;
    }
  return false;
}
    
bool exprPostfixPrim(){
  if(consume(LBRACKET)){
    if(expr()){
      if(consume(RBRACKET)){
	if(exprPostfixPrim()){
	  return true;
	}
      }
    }
  }
  if(consume(DOT)){
    if(consume(ID)){
      if(exprPostfixPrim()){
	return true;
      }
    }
  }
  return true; //epsilon
}

bool exprPostfix(){
  if(exprPrimary()){
    if(exprPostfixPrim()){
      return true;
    }
  }
  return false;
}

bool exprUnary(){
  if(consume(SUB)|| consume(NOT)){
    if(exprUnary()){
      return true;
    }
  }
  if(exprPostfix()){
    return true;
  }
  return false;
}

bool exprAssign(){
  if(exprUnary()){
    if(consume(ASSIGN)){
      if(exprAssign()){
	return true;
      }
    }
  }
  if(exprOr()){
    return true;
  }
  return false;
}

bool expr(){
  if(exprAssign()){
    return true;
  }
  return false;
}

bool stmCompound(){
  if(consume(LACC)){
    while(varDef()||stm()){}
    if(consume(RACC)){
      return true;
    }
  }
  return false;
}

bool stm(){
  if(stmCompound()){
      return true;
    }
    if(consume(IF)){
      if(consume(LPAR)){
	if(expr()){
	  if(consume(RPAR)){
	    if(stm()){
	      if(consume(ELSE)){
		if(stm()){}
	      }
	      return true;
	    }
	  }
	}
      }
    }
    if(consume(WHILE)){
      if(consume(LPAR)){
	if(expr()){
	  if(consume(RPAR)){
	    if(stm()){
	      return true;
	    }
	  }
	}
      }
    }
    if(consume(RETURN)){
      if(expr()){}
      if(consume(SEMICOLON)){
	return true;
      }
    }
    if(expr()){}
    if(consume(SEMICOLON)){
      return true;
    }
    return false;
}
	
bool fnParam(){
  if(typeBase()){
    if(consume(ID)){
      if(arrayDecl()){}
      return true;
      }
    }
  return false;
}

bool fnDef(){
  if(typeBase()||consume(VOID)){
    if(consume(ID)){
      return true;
    }
  }
  if(consume(LPAR)){
    if(fnparam()){
      while(consume(COMMA) || fnParam()){}
    }
    if(consume(RPAR)){
      return true;
    }
  }
  if(stmCompound()){
    return true;
  }
  return false;
}
 
bool arrayDecl(){
  if(consume(LBRACKET)){
    if(consume(INT)){}
    if(consume(RBRACKET)){
      return true;
    }
  }
  return false;
}

bool varDef(){
  if(typeBase()){
    if(consume(ID)){
     if(arrayDecl()){}
     if(consume(SEMICOLON)){
       return true;
     }
    }
  }
  return false;
}
 
bool structDef(){
  if(consume(STRUCT)){
    if(consume(ID)){
	if(consume(LACC)){
	  while(varDef()){}
	  if(consume(RACC)){
	      if(consume(SEMICOLON)){
		return true;
	      }
	  }
	}
      }
  }
  return false;
}


// unit: ( structDef | fnDef | varDef )* END
bool unit(){
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
		}
	return false;
	}

void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
	}


bool exprMulPrim(){
  if(consume(MUL)||consume(DIV)){
    if(exprCast()){
      if(exprMulPrim()){
	return true;
      }
    }
  }
  return true;
}

bool exprMul(){
  if(exprCast()){
    if(exprMulPrim()){
      return true;
    }
  }
  return false;
}

bool exprAddPrim(){
  if(consume(ADD)||consume(SUB)){
    if(exprMul()){
      if(exprAddPrim){
	return true;
      }
    }
  }
  return true; //epsilon
}

bool exprAdd(){
  if(exprMul()){
    if(exprAddPrim()){
      return true;
    }
  }
  return false;
}
bool exprRelPrim(){
  if(consume(LESS)||consume(LESSEQ)||consume(GREATER)||consume(GREATEREQ)){
    if(exprAdd()){
      if(exprRelPrim()){
	return true;
      }
    }
  }
  return true;
}

bool exprRel(){
  if(exprAdd()){
    if(exprRelPrim()){
      return true;
    }
  }
  return false;
}

bool exprEqPrim(){
  if(consume(EQUAL)||consume(NOTEQ)){
    if(exprRel()){
      if(exprEqPrim()){
	return true;
      }
    }
  }
  return true; //epsilon
}

bool exprEq(){
  if(exprRel()){
    if(exprEqPrim()){
      return true;
    }
  }
  return false;
}

bool exprAndPrim(){
  if(consume(AND)){
    if(exprEq()){
      if(exprAndPrim()){
	return true;
      }
    }
  }
  return true;//epsilon
}

bool exprAnd(){
 printf("#exprAnd %s\n",tkName(iTk->code));
 if(exprEq()){
   if(exprAndPrim()){
     return true;
   }
 }
}


bool exprOrPrim(){
  if(consume(OR)){
    if(exprAnd()){
      if(exprOrPrim()){
	return true;
      }
    }
  }
  return true; // epsilon
}

bool exprOr(){
  printf("#exprOr %s\n",tkName(iTk->code));
  if(exprAnd()){
    if(exprOrPrim()){
      return true;
    }
  }
  return false;
}

