#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"
#include "ad.h"
#include "vm.h"
#include "utils.h"
#include "at.h"


Token *iTk;		// the iterator in the tokens list
Token *consumedTk;     // the last consumed token

bool stmCompound(bool newDomain);
bool typeBase(Type *t);
bool arrayDecl(Type *t);

bool consume(int code);
bool structDef();
bool varDef();
bool fnDef();
bool fnParam();
bool stm();
bool expr(Ret *r);
bool exprAssign(Ret *r);
bool exprOr(Ret *r);
bool exprAnd(Ret *r);
bool exprEq(Ret *r);
bool exprRel(Ret *r);
bool exprAdd(Ret *r);
bool exprMul(Ret *r);
bool exprCast(Ret *r);
bool exprUnary(Ret *r);
bool exprPostfix(Ret *r);
bool exprPrimary(Ret *r);


bool exprOrPrim(Ret *r);
bool exprAndPrim(Ret *r);
bool exprEqPrim(Ret *r);
bool exprRelPrim(Ret *r);
bool exprAddPrim(Ret *r);
bool exprMulPrim(Ret *r);
bool exprPostfixPrim(Ret *r);

Symbol *owner=NULL;

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
bool typeBase(Type *t){
   printf("#typeBase\n");
   Token *start=iTk;
   t->n = -1;
	if(consume(TYPE_INT)){
	        t->tb=TB_INT;
		return true;
		}
	else if(consume(TYPE_DOUBLE)){
	        t->tb=TB_DOUBLE;
		return true;
		}
	else if(consume(TYPE_CHAR)){
	        t->tb=TB_CHAR;
		return true;
		}
	if (consume(STRUCT)) {
		if (consume(ID)) {
			Token *tkName = consumedTk;
			t->tb = TB_STRUCT;
			t->s = findSymbol(tkName->text);
			if(!t->s) {tkerr("Undefined structure: %s",tkName->text);}
			return true;
		}else {tkerr("Missing struct name");}
	}
	iTk=start;
	return false;
	}

bool arrayDecl(Type *t){
  printf("#arrayDecl\n");
  Token *start=iTk;
  if(consume(LBRACKET)){
    if(consume(INT)){
      Token *tkSize=consumedTk;
      t->n=tkSize->i;}
    else {t->n=0;} // array fara dimensiune
    if(consume(RBRACKET)){
      return true;
    }else {tkerr("Missing ] from array declaration");}
  }
  iTk=start;
  return false;
}

bool varDef(){
  printf("#vardef\n");
	Token* start = iTk;
	Type t;
	if(typeBase(&t)){
	
		if(consume(ID)){
			Token *tkName = consumedTk;
			if (arrayDecl(&t)) {
			  if (t.n == 0) {tkerr("a vector variable must have a specified dimension");}
			}
			if(consume(SEMICOLON)){
				Symbol *var=findSymbolInDomain(symTable,tkName->text);
				if(var){tkerr("symbol redefinition: %s",tkName->text);}
				var=newSymbol(tkName->text,SK_VAR);
				var->type=t;
				var->owner=owner;
				addSymbolToDomain(symTable,var);
				if (owner) {
					switch(owner->kind) {
						case SK_FN:
							var->varIdx=symbolsLen(owner->fn.locals);
							addSymbolToList(&owner->fn.locals,dupSymbol(var));
							break;
						case SK_STRUCT:
							var->varIdx=typeSize(&owner->type);
							addSymbolToList(&owner->structMembers,dupSymbol(var));
							break;
						default:
							break;
					}
				} else {
					var->varMem=safeAlloc(typeSize(&t));
				}
				return true;
			}else {tkerr("Missing ; from variable definition");}
		}else {tkerr("Missing name(id) from variable declaration");}
	}
	iTk = start;
	return false;
}

bool expr(Ret* r){
  printf("#expr %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprAssign(r)){
    return true;
  }
  iTk=start;
  return false;
}

bool exprAssign(Ret* r){
  printf("#exprAssign %s\n",tokenName(iTk->code));
  Token *start=iTk;
  Ret rDst;
  if(exprUnary(&rDst)){
    if(consume(ASSIGN)){
      if(exprAssign(r)){
	if(!rDst.lval){tkerr("the assign destination must be a left-value");}
	if(rDst.ct) {tkerr("the assign destination cannot be constant");}
        if (!canBeScalar(&rDst)) {tkerr("the assign destination must be scalar");}
        if (!canBeScalar(r)) {tkerr("the assign source must be scalar");}
	if (!convTo(&r->type, &rDst.type)) {tkerr("the assign source cannot be converted to destination");}
        r->lval=false;
	r->ct=true;
        return true;
      } else tkerr("Missing expression after = sign");
    }
    iTk=start;
  }
  
  if(exprOr(r)){
    return true;
  }
  iTk=start;
  return false;
}


bool exprOr(Ret* r){
  printf("#exprOr %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if (exprAnd(r)){
   if (exprOrPrim(r)){
     return true;
   }
  }
  iTk=start;
  return false;
}

bool exprOrPrim(Ret* r) {
    if (consume(OR)) {
        Ret right;
        if (exprAnd(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst)) {tkerr(iTk, "invalid operand type for ||");}
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if(exprOrPrim(r)){
	      return true;
	    }
        } else {tkerr(iTk, "invalid expression after ||");}
    }
    return true;
}


bool exprAnd(Ret* r){
 printf("#exprAnd %s\n",tokenName(iTk->code));
 Token *start=iTk;
 if(exprEq(r)){
   if(exprAndPrim(r)){
     return true;
   }
 }
 iTk=start;
 return false;
}

bool exprAndPrim(Ret* r){
  if(consume(AND)){
    Ret right;
    if(exprEq(&right)){
      Type tDst;
      if (!arithTypeTo(&r->type, &right.type, &tDst)) {tkerr("invalid operand type for &&");}
      *r = (Ret){{TB_INT, NULL, -1}, false, true};
      if(exprAndPrim(r)){
	return true;
      }
    }else {tkerr("Missing expression after &&");}
  }
  return true;//epsilon
}

bool exprEq(Ret* r){
  printf("#exprEq %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprRel(r)){
    if(exprEqPrim(r)){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprEqPrim(Ret* r){
  if(consume(EQUAL)||consume(NOTEQ)){
    Ret right;
    if(exprRel(&right)){
      Type tDst;
      if (!arithTypeTo(&r->type, &right.type, &tDst)) {tkerr("invalid operand type for == or !=");}
      *r = (Ret){{TB_INT,NULL,-1},false,true};
      if(exprEqPrim(r)){
	return true;
      }
    }else {tkerr("Missing expression after = or != sign");}
  }
  return true; //epsilon
}

bool exprRel(Ret* r){
  printf("#exprRel %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprAdd(r)){
    if(exprRelPrim(r)){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprRelPrim(Ret* r){
  if(consume(LESS)||consume(LESSEQ)||consume(GREATER)||consume(GREATEREQ)){
    Ret right;
    if(exprAdd(&right)){
      Type tDst;
      if (!arithTypeTo(&r->type, &right.type, &tDst)) {tkerr("invalid operand type for <, <=, >, >=");}
      *r = (Ret){{TB_INT,NULL,-1},false,true};
      if(exprRelPrim(r)){
	return true;
      }
    }else {tkerr("Missing expression after comparison operator");}
  }
  return true;
}

bool exprAdd(Ret* r){
  printf("#exprAdd %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprMul(r)){
    if(exprAddPrim(r)){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprAddPrim(Ret* r){
  if(consume(ADD)||consume(SUB)){
    Ret right;
    if(exprMul(&right)){
      Type tDst;
      if (!arithTypeTo(&r->type, &right.type, &tDst)){tkerr("invalid operand type for + or -");}
      *r = (Ret){tDst,false,true};
      if(exprAddPrim(r)){
	return true;
      }
    }else {tkerr("Missing expression after + or - operator");}
  }
  return true; //epsilon
}

bool exprCast(Ret* r) {
    printf("#exprCast %s\n", tokenName(iTk->code));
    Token *start = iTk;
    if (consume(LPAR)) {
       Type t;
       Ret op;
        if (typeBase(&t)) {
            if (arrayDecl(&t)) {}
            if (consume(RPAR)) {
                if (exprCast(&op)) {
                    if (t.tb == TB_STRUCT) {
                        tkerr("cannot convert to a struct type");
                    }
                    if (op.type.tb == TB_STRUCT) {
                        tkerr("cannot convert a struct");
                    }
                    if (op.type.n >= 0 && t.n < 0) {
                        tkerr("an array can be converted only to another array");
                    }
                    if (op.type.n < 0 && t.n >= 0) {
                        tkerr("a scalar can be converted only to another scalar");
                    }
                    *r = (Ret){t, false, true};
                    return true;
                }
            } else {tkerr("Missing ) from expression");}
        }
    }
    if (exprUnary(r)) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprMulPrim(Ret* r){
  if(consume(MUL)||consume(DIV)){
    Ret right;
    if(exprCast(&right)){
      Type tDst;
      if (!arithTypeTo(&r->type, &right.type, &tDst)) {tkerr("invalid operand type for * or /");}
      *r = (Ret){tDst,false,true};
      if(exprMulPrim(r)){
	return true;
      }
    }else {tkerr("Missing expression after * or / operator");}
  }
  return true;
}

bool exprMul(Ret* r){
  printf("#exprMul %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprCast(r)){
    if(exprMulPrim(r)){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprUnary(Ret *r) {
    printf("#exprUnary %s\n", tokenName(iTk->code));
    Token *start = iTk;
    if (consume(SUB) || consume(NOT)) {
      if (exprUnary(r)) {
	    if (!canBeScalar(r)) {tkerr("unary - or ! must have a scalar operand");}
            r->lval = false;
            r->ct = true;
            return true;
      } else {tkerr("Missing operator - or ! in expression");}
	iTk = start;
    }
    if (exprPostfix(r)) {
        return true;
    }
    iTk = start;
    return false;
}


bool exprPostfixPrim(Ret* r){
  Token *start=iTk;
  if(consume(LBRACKET)){
    Ret idx;
    if(expr(&idx)){
      if(consume(RBRACKET)){
	if (r->type.n < 0) {tkerr("only an array can be indexed");}
	Type tInt = {TB_INT,NULL,-1};
        if (!convTo(&idx.type, &tInt)) {tkerr("the index is not convertible to int");}			       
        r->type.n = -1;
        r->lval = true;
        r->ct = false;
	if(exprPostfixPrim(r)){
	  return true;
	}
	}else {tkerr("Missing ] from expression");}
    }
    iTk=start;
  }
  if(consume(DOT)){
    if(consume(ID)){
      Token *tkName = consumedTk;
      if (r->type.tb != TB_STRUCT){tkerr("a field can only be selected from a struct");}
      Symbol *s = findSymbolInList(r->type.s->structMembers, tkName->text);
      if (!s){tkerr("the structure %s does not have a field %s",r->type.s->name,tkName->text);}
      *r = (Ret){s->type,true,s->type.n>=0};
      if(exprPostfixPrim(r)){
	return true;
      }
    }else {tkerr("Missing name after . ");}
    iTk=start;
  }
  return true; //epsilon
}

bool exprPostfix(Ret* r){
  printf("#exprPostfix %s\n",tokenName(iTk->code));
  Token *start=iTk;
  if(exprPrimary(r)){
    if(exprPostfixPrim(r)){
      return true;
    }
  }
  iTk=start;
  return false;
}

bool exprPrimary(Ret* r){
  printf("#exprPrimary\n");
  Token* start = iTk;
  if(consume(ID)){
    Token *tkName = consumedTk;
    Symbol *s=findSymbol(tkName->text);
    if(!s) {tkerr("undefined id: %s", tkName->text);}
   if(consume(LPAR)){
     if(s->kind!=SK_FN) {tkerr("only a function can be called");}
     Ret rArg;
     Symbol *param=s->fn.params;
    if(expr(&rArg)){
      if(!param) {tkerr("too many arguments in function call");}
      if(!convTo(&rArg.type,&param->type)) {tkerr("in call, cannot convert the argument type to parameter type");}
      param=param->next;
     while(consume(COMMA)){
      if(expr(&rArg)){
	if(!param) {tkerr("too many arguments in function call");}
	if(!convTo(&rArg.type,&param->type)) {tkerr("in call, cannot convert the argument type to parameter type");}
	param=param->next;
      }
      else {tkerr("Missing expression after ,");}
     }
    }
     if(consume(RPAR)){
       if(param) {tkerr("too few arguments in function call");}
       *r=(Ret){s->type,false,true};
       if(s->kind==SK_FN) {tkerr("a function can only be called");}
       *r=(Ret){s->type,true,s->type.n>=0};
      return true;
     }else {tkerr("Missing )");}
   }
    return true;
  }
  else if(consume(DOUBLE)){
    *r=(Ret){{TB_DOUBLE,NULL,-1},false,true};
    return true;
  }
  else if(consume(CHAR)){
    *r=(Ret){{TB_CHAR,NULL,-1},false,true};
    return true;
  }
  else if(consume(STRING)){
     *r=(Ret){{TB_CHAR,NULL,0},false,true};
    return true;
  }
  else if(consume(INT)){
     *r=(Ret){{TB_INT,NULL,-1},false,true};
    return true;
  }
  else if(consume(LPAR)){
    if(expr(r)){
     if(consume(RPAR)){
      return true;
     }else {tkerr("Missing )");}
    }else {tkerr("Missing expression after (");}
  }
 iTk = start;
 return false;
}

    
bool stm(){
  printf("#stm\n");
  Token* start= iTk;
  Ret rCond, rExpr;
  if(stmCompound(true)){
    return true;
  }
  if(consume(IF)){
   if(consume(LPAR)){
    if(expr(&rCond)){
     if (!canBeScalar(&rCond)) {tkerr("the if condition must be a scalar value");}
     if(consume(RPAR)){
      if(stm()){
       if(consume(ELSE)){
	if(stm()) {
          return true;
	}else {tkerr("Else statement missing");}
       }
	return true;
      }else {tkerr("If statement missing");}
     }else {tkerr("Missing ) for if");}
   }else {tkerr("Missing if expression");}
  }else {tkerr("Missing ( after if");}
    iTk=start;
  }
   else if(consume(WHILE)){
         if(consume(LPAR)){
	  if(expr(&rCond)){
	   if(!canBeScalar(&rCond)) {tkerr("the while condition must be a scalar value");}
	   if(consume(RPAR)){
	    if(stm()){
	      return true;
	    }else {tkerr("Missing while body");}
	   }else {tkerr("Missing ) for while");}
	  }else {tkerr("Missing while condition");}
	 }else {tkerr("Missing ( after while keyword");}
	  iTk=start;
   }
    else if(consume(RETURN)){
	  if(expr(&rExpr)){
	    if(owner->type.tb == TB_VOID) {tkerr("a void function cannot return a value");}
	    if(!canBeScalar(&rExpr)) {tkerr("the return value must be a scalar value");}
	    if(!convTo(&rExpr.type, &owner->type)) {tkerr("cannot convert the return expression type to the function return type");}
	  } else {if(owner->type.tb != TB_VOID) {tkerr("a non-void function must return a value");}}
	  
	  if(consume(SEMICOLON)){
	     return true;
	   }else {tkerr("Missing ; at return statement");}
	  iTk=start;
        }
    else if(expr(&rExpr)){
          if(consume(SEMICOLON)){
            return true;
	  }
    }
     iTk = start;
     return false;
}


bool stmCompound(bool newDomain){
	printf("StmCompound\n");
	Token* start = iTk;
	if(consume(LACC)){
	  if(newDomain)pushDomain();
			for(;;){
			if(varDef()){}
			else if(stm()){}
			else break;
		}
		if(consume(RACC)){
		  if(newDomain)dropDomain();
			return true;
		}else {tkerr("Missing } for a statement");}
	}
	iTk = start;
	return false;
}

	
bool fnParam(){
  printf("#fnParam\n");
  Token *start=iTk;
  Type t;
	if (typeBase(&t)) {
		if (consume(ID)) {
			Token *tkName = consumedTk;
			if (arrayDecl(&t)) {
				t.n=0;
			}
			Symbol *param=findSymbolInDomain(symTable,tkName->text);
			if(param){tkerr("symbol redefinition: %s",tkName->text);}
			param=newSymbol(tkName->text,SK_PARAM);
			param->type=t;
			param->owner=owner;
			param->paramIdx=symbolsLen(owner->fn.params);
			// parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
			addSymbolToDomain(symTable,param);
			addSymbolToList(&owner->fn.params,dupSymbol(param));
			return true;
		} else {tkerr("Missing parameter function name");}
	}
	iTk = start;
	return false;
}

bool fnDef(){
  printf("#fnDef\n");
  Token *start= iTk;
  Type t;
  if (typeBase(&t)) {
		if (consume(ID)) {
			Token *tkName = consumedTk;
			if (consume(LPAR)) {
				Symbol *fn=findSymbolInDomain(symTable,tkName->text);
				if(fn) {tkerr("symbol redefinition: %s",tkName->text);}
				fn=newSymbol(tkName->text,SK_FN);
				fn->type=t;
				addSymbolToDomain(symTable,fn);
				owner=fn;
				pushDomain();
				if (fnParam()) {
					for (;;) {
						if (consume(COMMA)) {
							if (fnParam()) {}
							else {tkerr("Expected type specifier in function after ,");}
						} else break;
					}
				}
				if (consume(RPAR)) {
					if (stmCompound(false)) {
						dropDomain();
						owner=NULL;
						return true;
					} else {tkerr("Missing body function");}
				} else {tkerr("Missing ) in function");}
			}
		} else {tkerr("Missing name(id) for function");}
	} else if (consume(VOID)) {
		t.tb=TB_VOID;
		if (consume(ID)) {
			Token *tkName = consumedTk;
			if (consume(LPAR)) {
				Symbol *fn=findSymbolInDomain(symTable,tkName->text);
				if(fn){tkerr("symbol redefinition: %s",tkName->text);}
				fn=newSymbol(tkName->text,SK_FN);
				fn->type=t;
				addSymbolToDomain(symTable,fn);
				owner=fn;
				pushDomain();
				if (fnParam()) {
					for (;;) {
						if (consume(COMMA)) {
							if (fnParam()) {}
							else {tkerr("Expected type specifier in function after ,");}
						} else break;
					}
				}
				if (consume(RPAR)) {
					if (stmCompound(false)) {
						dropDomain();
						owner=NULL;
						return true;
					} else {tkerr("Missing body function");}
				} else {tkerr("Missing ) in function");}
			}
		} else {tkerr("Missing name(id) for function");}
	}
	iTk = start;
	return false;
}
 
bool structDef(){
  printf("#structDef\n");
  Token *start=iTk;
	if(consume(STRUCT)){
		if(consume(ID)){
			Token *tkName = consumedTk;
			if(consume(LACC)){
				Symbol *s = findSymbolInDomain(symTable, tkName->text);
				if(s) {tkerr("symbol redefinition: %s", tkName->text);}
				s = addSymbolToDomain(symTable,newSymbol(tkName->text, SK_STRUCT));
				s->type.tb = TB_STRUCT;
				s->type.s = s;
				s->type.n = -1;
				pushDomain();
				owner = s;
				while(varDef()){}
				if(consume(RACC)){
					if(consume(SEMICOLON))
					{
						owner = NULL;
						dropDomain();
						return true;
					}else {tkerr("Missing ; at struct definition");}
				}else {tkerr("Missing } from struct initialisation");}
			}
		}else {tkerr("Missing struct name");}
	}
	iTk = start;
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
	} else {tkerr("End of file not reached!");}
  return false;
}

void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
}
