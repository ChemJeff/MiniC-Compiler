/* Syntactic parser corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

%{
#include <stdio.h>
#include <string.h>
#include "Eeyore.typedef.hh"

extern Expr* exprTab[];
extern Var* varTab[];
extern int expr_cnt;
extern int func_cnt;
extern int var_cnt;

extern ExprPrinter exprPrinter;

extern char* yytext;
extern int lineno;
extern int tokenepos;
extern int tokenspos;
extern FILE* yyout;

bool isGlobal = true;
void yyerror(const char*, int=0, int=0);
int yylex(void);

%}

// token_type definitions
%code requires{
#include "Eeyore.typedef.hh"
}
%union{
    int int_value;
    char* string_value;
    Rval rval_value;
}

// token definitions
%token <rval_value> INTEGER
%token <rval_value> VARIABLE
%token <int_value> LABEL
%token <string_value> FUNCTION
%token ENDF 0 // end of file
%token EOL
%token VAR
%token IF GOTO
%token PARAM CALL RETURN END
%token OP_AND OP_OR OP_EQ OP_NE

%type <rval_value> RightValue
%type <int_value> OP1 LOGICALOP2 OP2
%type <string_value> FuncName

// specifying start symbol
%start Goal

%%

Goal:
      /* Empty */       {}
    | Goal VarDecl      {}
    | Goal FuncDecl     {}
    ;

VarDecl:
      VAR VARIABLE EOL {
          varTab[var_cnt++] = new Var($2, isGlobal, 0);
          exprTab[expr_cnt++] = new Decl($2, isGlobal, 0);

      }
    | VAR INTEGER VARIABLE EOL {
          varTab[var_cnt++] = new Var($3, isGlobal, $2.val);
          exprTab[expr_cnt++] = new Decl($3, isGlobal, $2.val);
      }
    ;

FuncDecl:
      FuncName FuncBody FuncEnd     {}
    ;

FuncName:
      FUNCTION '[' INTEGER ']' EOL {
          exprTab[expr_cnt++] = new Func($1, $3.val);
          isGlobal = false;
          for (int i=0; i<$3.val; i++) {
              varTab[var_cnt++] = new Var(Rval{func_cnt*8 + i, VAR_PARAM},
               isGlobal, 0);
          }
      }
    ;

FuncBody:
      Expression            {}
    | FuncBody Expression   {}
    ;

FuncEnd:
      END FUNCTION EOL {
          exprTab[expr_cnt++] = new FuncEnd($2);
          func_cnt++;
          isGlobal = true;
      }
    ;

Expression:
      VARIABLE '=' RightValue OP2 RightValue EOL {
          exprTab[expr_cnt++] = new BinaryOp($1, $3, $4, $5);
      }
    | VARIABLE '=' OP1 RightValue EOL {
          exprTab[expr_cnt++] = new UnaryOp($1, $3, $4);
      }
    | VARIABLE '=' RightValue EOL {
          exprTab[expr_cnt++] = new Assign($1, Rval{0, 0}, $3, Rval{0, 0}, ASS_NO_ARRAY);
      }
    | VARIABLE '[' RightValue ']' '=' RightValue EOL {
          exprTab[expr_cnt++] = new Assign($1, $3, $6, Rval{0, 0}, ASS_ARRAY_LEFT);
      }
    | VARIABLE '=' VARIABLE '[' RightValue ']' EOL {
          exprTab[expr_cnt++] = new Assign($1, Rval{0, 0}, $3, $5, ASS_ARRAY_RIGHT);
      }
    | IF RightValue LOGICALOP2 RightValue GOTO LABEL EOL {
          exprTab[expr_cnt++] = new CondiGoto($2, $3, $4, $6);
      }
    | GOTO LABEL EOL {
          exprTab[expr_cnt++] = new Goto($2);
      }
    | LABEL ':' EOL {
          exprTab[expr_cnt++] = new Label($1);
      }
    | PARAM RightValue EOL {
          exprTab[expr_cnt++] = new Param($2);
      }
    | VARIABLE '=' CALL FUNCTION EOL {
          exprTab[expr_cnt++] = new Call($4, true, $1);
      }
    | CALL FUNCTION EOL {
          exprTab[expr_cnt++] = new Call($2, false, Rval{0, 0});
      }
    | VAR VARIABLE EOL {
          varTab[var_cnt++] = new Var($2, isGlobal, 0);
          exprTab[expr_cnt++] = new Decl($2, isGlobal, 0);
      }
    | VAR INTEGER VARIABLE EOL {
          varTab[var_cnt++] = new Var($3, isGlobal, $2.val);
          exprTab[expr_cnt++] = new Decl($3, isGlobal, $2.val);
      }
    | RETURN RightValue EOL {
          exprTab[expr_cnt++] = new Ret($2);
      }
    | EOL {}
    ;

RightValue:
      VARIABLE      {$$ = $1;}
    | INTEGER       {$$ = $1;}
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
