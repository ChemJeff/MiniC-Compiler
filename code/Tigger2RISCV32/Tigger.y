/* Syntactic parser corresponding to Tigger grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

%{
#include <stdio.h>
#include <string.h>
#include "Tigger.typedef.hh"

extern Expr* exprTab[];
extern int expr_cnt;

extern ExprPrinter exprPrinter;

extern char* yytext;
extern int lineno;
extern int tokenepos;
extern int tokenspos;
extern FILE* yyout;

void yyerror(const char*, int=0, int=0);
int yylex(void);

%}

// token_type definitions
%code requires{
#include "Tigger.typedef.hh"
}
%union{
    int int_value;
    char* string_value;
}

// token definitions
%token <int_value> INTEGER
%token <int_value> VARIABLE
%token <int_value> LABEL
%token <int_value> REG
%token <string_value> FUNCTION
%token ENDF 0 // end of file
%token EOL
%token MALLOC
%token LOAD STORE LOADADDR
%token IF GOTO
%token CALL RETURN END
%token OP_AND OP_OR OP_EQ OP_NE

%type <int_value> OP1 LOGICALOP2 OP2
%type <string_value> FuncName

// specifying start symbol
%start Goal

%%

Goal:
      /* Empty */           {}
    | Goal FuncDecl         {}
    | Goal GloablVarDecl    {}
    ;

GloablVarDecl:
      VARIABLE '=' INTEGER EOL {
          exprTab[expr_cnt++] = new GlobalDecl($1, false, $3);
      }
    | VARIABLE '=' MALLOC INTEGER EOL {
          exprTab[expr_cnt++] = new GlobalDecl($1, true, $4);
     }
    ;

FuncDecl:
      FuncName FuncBody FuncEnd     {}
    ;

FuncName:
      FUNCTION '[' INTEGER ']' '[' INTEGER ']' EOL {
          exprTab[expr_cnt++] = new Func($1, $3, $6);
      }
    ;

FuncBody:
      Expression            {}
    | FuncBody Expression   {}
    ;

FuncEnd:
      END FUNCTION EOL {
          exprTab[expr_cnt++] = new FuncEnd($2);
      }
    ;

Expression:
      REG '=' REG OP2 REG EOL {
          exprTab[expr_cnt++] = new BinaryOp($1, $3, $4, true, $5);
      }
    | REG '=' REG OP2 INTEGER EOL {
          exprTab[expr_cnt++] = new BinaryOp($1, $3, $4, false, $5);
     }
    | REG '=' OP1 REG EOL {
          exprTab[expr_cnt++] = new UnaryOp($1, $3, $4);
     }
    | REG '=' REG EOL {
          exprTab[expr_cnt++] = new Assign($1, true, $3, 0, ASS_NO_ARRAY);
     }
    | REG '=' INTEGER EOL {
          exprTab[expr_cnt++] = new Assign($1, false, $3, 0, ASS_NO_ARRAY);
     }
    | REG '[' INTEGER ']' '=' REG EOL {
          exprTab[expr_cnt++] = new Assign($1, true, $6, $3, ASS_ARRAY_LEFT);
     }
    | REG '=' REG '[' INTEGER ']' EOL {
          exprTab[expr_cnt++] = new Assign($1, true, $3, $5, ASS_ARRAY_RIGHT);
     }
    | IF REG LOGICALOP2 REG GOTO LABEL EOL {
          exprTab[expr_cnt++] = new CondiGoto($2, $3, $4, $6);
     }
    | GOTO LABEL EOL {
          exprTab[expr_cnt++] = new Goto($2);
     }
    | LABEL ':' EOL {
          exprTab[expr_cnt++] = new Label($1);
     }
    | CALL FUNCTION EOL {
          exprTab[expr_cnt++] = new Call($2);
     }
    | STORE REG INTEGER EOL {
          exprTab[expr_cnt++] = new Store($2, $3);
     }
    | LOAD INTEGER REG EOL {
          exprTab[expr_cnt++] = new Load(false, $2, $3);
     }
    | LOAD VARIABLE REG EOL {
          exprTab[expr_cnt++] = new Load(true, $2, $3);
     }
    | LOADADDR INTEGER REG EOL {
          exprTab[expr_cnt++] = new LoadAddr(false, $2, $3);
     }
    | LOADADDR VARIABLE REG EOL {
          exprTab[expr_cnt++] = new LoadAddr(true, $2, $3);
     }
    | RETURN EOL {
          exprTab[expr_cnt++] = new Ret();
     }
    | EOL {}
    ;

OP2: 
      '+'           {$$ = OPR_ADD;}
    | '-'           {$$ = OPR_SUB;}
    | '*'           {$$ = OPR_MUL;}
    | '/'           {$$ = OPR_DIV;}
    | '%'           {$$ = OPR_MOD;}
    | LOGICALOP2    {$$ = $1;}
    ;

LOGICALOP2:
      OP_NE         {$$ = OPR_NE;}
    | OP_EQ         {$$ = OPR_EQ;}
    | OP_AND        {$$ = OPR_AND;}
    | OP_OR         {$$ = OPR_OR;}
    | '>'           {$$ = OPR_GT;}
    | '<'           {$$ = OPR_LT;}
    ;

OP1:
      '-'           {$$ = OPR_NEG;}
    | '!'           {$$ = OPR_NOT;}
    ;

%%

void yyerror(const char *msg, int line_no, int token_pos) {
    if (line_no) { // use specified lineno
        fprintf(stdout, "\nError at \"%s\" (line %d:%d): %s \n", yytext, line_no, token_pos, msg);
    }
    else { // use global lineno
        fprintf(stdout, "\nError at \"%s\" (line %d:%d): %s \n", yytext, lineno, tokenspos, msg);
    }
}

