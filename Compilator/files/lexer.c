#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;		// the last token in list

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code){
	Token *tk=safeAlloc(sizeof(Token));
	tk->code=code;
	tk->line=line;
	tk->next=NULL;
	if(lastTk){
		lastTk->next=tk;
		}else{
		tokens=tk;
		}
	lastTk=tk;
	return tk;
	}

char *extract(const char *begin, const char *end)
{
	int length = end - begin;
	char *result = safeAlloc(length + 1);
	strncpy(result, begin, length);
	result[length] = '\0';
	return result;
}

const char* tokenName(int position){
  switch(position){
  case ID: return "ID";
  case TYPE_CHAR: return "TYPE_CHAR";
  case TYPE_DOUBLE: return "TYPE_DOUBLE";
  case ELSE: return "ELSE";
  case IF: return "IF";
  case TYPE_INT: return "TYPE_INT";
  case RETURN: return "RETURN";
  case STRUCT: return "STRUCT";
  case VOID: return "VOID";
  case WHILE: return "WHILE";
  case COMMA: return "COMMA";
  case SEMICOLON: return "SEMICOLON";
  case LPAR: return "LPAR";
  case RPAR: return "RPAR";
  case LBRACKET: return "LBRACKET";
  case RBRACKET: return "RBRACKET";
  case LACC: return "LACC";
  case RACC: return "RACC";
  case END: return "END";
  case ADD: return "ADD";
  case SUB: return "SUB";
  case MUL: return "MUL";
  case DIV: return "DIV";
  case DOT: return "DOT";
  case AND: return "AND";
  case OR: return "OR";
  case NOT: return "NOT";
  case ASSIGN: return "ASSIGN";
  case EQUAL: return "EQUAL";
  case NOTEQ: return "NOTEQ";
  case LESS: return "LESS";
  case LESSEQ: return "LESSEQ";
  case GREATER: return "GREATER";
  case GREATEREQ: return "GREATEREQ";
  default: return "ERROR - TOKEN NOT FROM LIST";
  }
}

Token *tokenize(const char *pch){
	const char *start;
	Token *tk;
	for(;;){
		switch(*pch){
			case ' ':case '\t':pch++;break;
			case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
				if(pch[1]=='\n')pch++;
				// fallthrough to \n
			case '\n':
				line++;
				pch++;
				break;
			case '\0':addTk(END);return tokens;
			case ',':addTk(COMMA);pch++;break;
			case '=':
				if(pch[1]=='='){
					addTk(EQUAL);
					pch+=2;
					}else{
					addTk(ASSIGN);
					pch++;
					}
				break;
		        case '(': addTk(LPAR);pch++;break;
		        case ')': addTk(RPAR);pch++;break;
		        case '{': addTk(LACC);pch++;break;
		        case '}': addTk(RACC);pch++;break;
		        case '[': addTk(LBRACKET);pch++;break;
		        case ']': addTk(RBRACKET);pch++;break;
		        case '+': addTk(ADD);pch++;break;
		        case '-': addTk(SUB);pch++;break;
		        case '*': addTk(MUL);pch++;break;
		        case '/': addTk(DIV);pch++;break;
		        case '.': addTk(DOT);pch++;break;
		        case ';': addTk(SEMICOLON);pch++;break;
		        case '!':
		                  if(pch[1]=='='){ addTk(NOTEQ);pch+=2;}
		                  else{ addTk(NOT);pch++;}
		                  break;
		        case '<':
		                  if(pch[1]=='='){ addTk(LESSEQ);pch+=2;}
		                  else{ addTk(LESS);pch++;}
		                  break;
		        case '>':
		                 if(pch[1]=='='){ addTk(GREATEREQ);pch+=2;}
		                 else{ addTk(GREATER);pch++;}
		                 break;
		        case '|':
		                 if(pch[1]=='|'){ addTk(OR);pch+=2;}
		                 break;
		        case '&':
		                 if(pch[1]=='&'){ addTk(AND);pch+=2;}
		                 break;	 
			default:
				if(isalpha(*pch)||*pch=='_'){
					for(start=pch++;isalnum(*pch)||*pch=='_';pch++){}
					char *text=extract(start,pch);
					if(strcmp(text,"char")==0)addTk(TYPE_CHAR);
					else if(strcmp(text,"int")==0)addTk(TYPE_INT);
					else if(strcmp(text,"double")==0)addTk(TYPE_DOUBLE);
					else if(strcmp(text,"else")==0)addTk(ELSE);
					else if(strcmp(text,"if")==0)addTk(IF);
					else if(strcmp(text,"return")==0)addTk(RETURN);
					else if(strcmp(text,"void")==0)addTk(VOID);
					else if(strcmp(text,"struct")==0)addTk(STRUCT);
					else if(strcmp(text,"while")==0)addTk(WHILE);
					else{
						tk=addTk(ID);
						tk->text=text;
						}
					}
				else if(isdigit(*pch)){
				  int with_dot = 0;
				  int with_e = 0;
				  for (start = pch++; isalnum(*pch) || *pch == '_' || *pch == '.' || *pch == 'e' || *pch=='E' || *pch == '-' || *pch == '+'; pch++){
				      if (*pch == '.'){
						with_dot=1;
				      }
				      if (*pch == 'e' || *pch == 'E'){
						with_e=1;
				      }
				    }
				    char *text=extract(start, pch);

				    if (with_dot || with_e){
					tk=addTk(DOUBLE);
					char *endptr;
					tk->d=strtod(text, &endptr);
				    }
				    else{
					tk=addTk(INT);
					tk->i=atoi(text);
				    }
				}
				else if (isalpha(*pch) || *pch == '"' || *pch == '\''){
				  pch++;
				  int one_quote=0;
				  for(start=pch++;isalnum(*pch) || *pch=='"' || *pch=='\'';pch++){
				    if(*pch=='\''){
				      one_quote=1;
				    }
				  }
				  if(one_quote){ 
				    char *text = extract(start,pch-1);
					tk=addTk(CHAR);
					tk->c=*text;
				  }
				  else{
				    char *text=extract(start, pch - 1);
					tk=addTk(STRING);
					tk->text=text;
				  }
				}
				else err("invalid char: %c (%d)",*pch,*pch);
		}
	}
}

void showTokens(const Token *tokens){
	for(const Token *tk=tokens;tk;tk=tk->next){
	  printf("%d - %s\n",tk->line,tokenName(tk->code));
	}
}
