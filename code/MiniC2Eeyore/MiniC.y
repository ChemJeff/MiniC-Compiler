/* Syntactic parser corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

%{
#include <stdio.h>
#include <string.h>
#include "MiniC.AST.hh"
#include "MiniC.symtab.hh"
#include "MiniC2Eeyore.hh"

extern char* yytext;
extern int lineno;
extern int tokenepos;
extern int tokenspos;
extern FILE* yyout;
void yyerror(const char*, int=0, int=0);
int yylex(void);

int ASTNode::printDepth = 0;
int ASTNode::nodesno = 0;
ASTNode *root, *curr;

%}

// token_type definitions
%code requires{
#include "MiniC.AST.hh"
}
%union{
    int int_value;
    char* string_value;
    ASTNode* node_value;
}

// token definitions
%token <int_value> INT_CONSTANT
%token <string_value> IDENTIFIER
%token INT
%token MAIN
%token BREAK CONTINUE                                                   // not implemented yet
%token IF ELSE WHILE
%token RETURN
%token OP_GE OP_LE                                                      // not implemented yet
%token OP_AND OP_OR OP_EQ OP_NE
%token OP_INC OP_DEC
%token OP_LSH OP_RSH                                                    // not implemented yet

%type <int_value> Type
%type <node_value> Goal VarDefn VarDecl FuncDefn FuncDecl ParamList ArgList MainFunc FuncBody Statement Statements Expression Integer Identifier

// priority and associativity
// operations with lower priority listed first
// can use some 'placeholer tokens' to specify
%right '='
%left OP_OR
%left OP_AND
%left OP_EQ OP_NE
%left '<' '>' OP_LE OP_GE
%left OP_LSH OP_RSH
%left '+' '-'
%left '*' '/' '%'
%right OP_NEG OP_POS
%right '!'
%right OP_INC OP_DEC

// specifying start symbol
%start Goal

%%

Goal: 
      VarDefn Goal                                          {
                                                                curr = $2;
                                                                ((Goal*)curr)->add_block();
                                                                curr->insert($1, 1);
                                                                $$ = curr; }
    | FuncDefn Goal                                         {
                                                                curr = $2;
                                                                ((Goal*)curr)->add_block();
                                                                curr->insert($1, 1);
                                                                $$ = curr; }
    | FuncDecl Goal                                         {
                                                                curr = $2;
                                                                ((Goal*)curr)->add_block();
                                                                curr->insert($1, 1);
                                                                $$ = curr; }
    | MainFunc                                              {
                                                                curr = new Goal();
                                                                curr->insert($1);
                                                                $$ = curr;
                                                                root = curr; }
    ;

VarDefn: 
      Type Identifier ';'                                   {
                                                                curr = new VarDefn(false);
                                                                curr->setType($1);
                                                                $2->setType($1);
                                                                curr->insert($2);
                                                                $$ = curr; }
    | Type Identifier '[' Integer ']' ';'              {
                                                                curr = new VarDefn(true, ((Integer*)$4)->val);
                                                                curr->setType($1 + TYPE_ARRAY);
                                                                $2->setType($1 + TYPE_ARRAY);
                                                                curr->insert($2);
                                                                curr->insert($4);
                                                                $$ = curr; }
    ;

VarDecl: 
      Type Identifier                                       {
                                                                curr = new VarDecl(false);
                                                                curr->setType($1);
                                                                $2->setType($1);
                                                                curr->insert($2);
                                                                $$ = curr; }
    | Type Identifier '[' Integer ']'                  {
                                                                curr = new VarDecl(true, ((Integer*)$4)->val);
                                                                curr->setType($1 + TYPE_ARRAY);
                                                                $2->setType($1 + TYPE_ARRAY);
                                                                curr->insert($2);
                                                                curr->insert($4);
                                                                $$ = curr; }
    | Type Identifier '[' ']'                               {
                                                                curr = new VarDecl(true);
                                                                curr->setType($1 + TYPE_ARRAY);
                                                                $2->setType($1 + TYPE_ARRAY);
                                                                curr->insert($2);
                                                                $$ = curr; }
    ;

FuncDefn: 
      Type Identifier '(' ParamList ')' '{' FuncBody '}'    {
                                                                curr = new FuncDefn(false, true);
                                                                $2->setType($1 + TYPE_FUNC);
                                                                curr->insert($2);
                                                                curr->insert($4);
                                                                curr->insert($7);
                                                                $$ = curr; }
    | Type Identifier '(' ')' '{' FuncBody '}'              { // reduce empty rules, the same below
                                                                curr = new FuncDefn(false, false);
                                                                $2->setType($1 + TYPE_FUNC);
                                                                curr->insert($2);
                                                                curr->insert($6);
                                                                $$ = curr; }
    ;

FuncDecl:
      Type Identifier '(' ParamList ')' ';'                 {
                                                                curr = new FuncDecl(true);
                                                                curr->setType($1 + TYPE_FUNC);
                                                                $2->setType($1 + TYPE_FUNC);
                                                                curr->insert($2);
                                                                curr->insert($4);
                                                                $$ = curr; }
    | Type Identifier '(' ')' ';'                           {
                                                                curr = new FuncDecl(false);
                                                                curr->setType($1 + TYPE_FUNC);
                                                                $2->setType($1 + TYPE_FUNC);
                                                                curr->insert($2);
                                                                $$ =curr; }
    ;

Type: 
      INT                                                   {
                                                                $$ = TYPE_INT; }
    ;

ParamList: 
      ParamList ',' VarDecl                                 {
                                                                curr = $1;
                                                                ((List*)curr)->add_item();
                                                                curr->insert($3);
                                                                $$ = curr; }
    | VarDecl                                               {
                                                                curr = new List(1, false);
                                                                curr->insert($1);
                                                                $$ = curr; }
    ;

ArgList: 
      ArgList ',' Expression                                {
                                                                curr = $1;
                                                                ((List*)curr)->add_item();
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression                                            {
                                                                curr = new List(1, true);
                                                                curr->insert($1);
                                                                $$ = curr; }    
    ;

MainFunc: 
      INT MAIN '(' ')' '{' FuncBody '}'                     {
                                                                curr = new FuncDefn(true, false);
                                                                curr->insert($6);
                                                                $$ = curr; }
    ;

FuncBody: 
      FuncBody FuncDecl                                     {
                                                                curr = $1;
                                                                ((FuncBody*)curr)->add_stmt();
                                                                curr->insert($2);
                                                                $$ = curr; }
    | FuncBody Statement                                    {
                                                                curr = $1;
                                                                ((FuncBody*)curr)->add_stmt();
                                                                curr->insert($2);
                                                                $$ = curr; }
    | Statement                                             { // No default return value assigned in miniC grammar
                                                                curr = new FuncBody(1);
                                                                curr->insert($1);
                                                                $$ = curr; }
    |                                                       { // empty
                                                                curr = new FuncBody(0);
                                                                $$ = curr; }
    ;

Statement: 
      '{' '}'                                               { // EMPTY CODE BLOCK
                                                                curr = new Stmt(STMT_EMPTY);
                                                                $$ = curr; }
    | '{' Statements '}'                                    { // CODE BLOCK
                                                                $$ = $2; }
    | IF '(' Expression ')' Statement ELSE Statement        { // MATCHED_IF_STMT
                                                                curr = new Stmt(STMT_MATCHED_IF);
                                                                curr->insert($3);
                                                                curr->insert($5);
                                                                curr->insert($7);
                                                                $$ = curr; }
    | IF '(' Expression ')' Statement                       { // OPEN_IF_STMT
                                                                curr = new Stmt(STMT_OPEN_IF);
                                                                curr->insert($3);
                                                                curr->insert($5);
                                                                $$ = curr; }
    | WHILE '(' Expression ')' Statement                    { // WHILE_STMT
                                                                curr = new Stmt(STMT_WHILE);
                                                                curr->insert($3);
                                                                curr->insert($5);
                                                                $$ = curr; }
    | Identifier '=' Expression ';'                         { // ASSGN_STMT
                                                                curr = new Stmt(STMT_ASSIGN);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Identifier '[' Expression ']' '=' Expression ';'      { // ARRAY_ASSGN_STMT
                                                                curr = new Stmt(STMT_ARRAY_ASSIGN);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                curr->insert($6);
                                                                $$ = curr; }
    | Identifier '(' ArgList ')' ';'                        { // FUNC_CALL_STMT
                                                                curr = new Stmt(STMT_FUNC_CALL);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Identifier '(' ')' ';'                                { // FUNC_CALL_NOARG_STMT
                                                                curr = new Stmt(STMT_FUNC_CALL_NOARG);
                                                                curr->insert($1);
                                                                $$ = curr;}
    | Expression ';'                                        { // EXPR_AS_STMT
                                                                curr = new Stmt(STMT_EXPR);
                                                                curr->insert($1);
                                                                $$ = curr; }
    | VarDefn                                               { // DEFN_STMT
                                                                curr = new Stmt(STMT_DEFN);
                                                                curr->insert($1);
                                                                $$ = curr; }
    | RETURN Expression ';'                                 { // RET_STMT
                                                                curr = new Stmt(STMT_RET);
                                                                curr->insert($2);
                                                                $$ = curr; }
    ;

Statements: 
      Statements Statement                                  {
                                                                curr = $1;
                                                                curr->insert($2);
                                                                $$ = curr; }
    | Statement                                             {
                                                                curr = new Stmt(STMT_MULTI);
                                                                curr->insert($1);
                                                                $$ = curr; }
    ;

Expression: 
      '(' Expression ')'                                    { // TERM
                                                                $$ = $2; }
    | Expression '+' Expression                             { // ADDITION
                                                                curr = new Expr(OPR_ADD, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '-' Expression                             { // SUBSTRACTION
                                                                curr = new Expr(OPR_SUB, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '*' Expression                             { // MULTIPLICATION
                                                                curr = new Expr(OPR_MUL, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '/' Expression                             { // DIVISION
                                                                curr = new Expr(OPR_DIV, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '%' Expression                             { // MODULO OPERATION
                                                                curr = new Expr(OPR_MOD, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '=' Expression                             { // ASSIGN OPERATION
                                                                curr = new Expr(OPR_ASS, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression OP_AND Expression                          { // LOGIC AND
                                                                curr = new Expr(OPR_AND, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression OP_OR Expression                           { // LOGIC OR
                                                                curr = new Expr(OPR_OR, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '<' Expression                             { // LOGIC LT
                                                                curr = new Expr(OPR_LT, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression '>' Expression                             { // LOGIC GT
                                                                curr = new Expr(OPR_GT, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression OP_EQ Expression                           { // LOGIC EQ
                                                                curr = new Expr(OPR_EQ, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression OP_NE Expression                           { // LOGIC NE
                                                                curr = new Expr(OPR_NE, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Expression OP_LE Expression // LOGIC LE, not implemented yet
    | Expression OP_GE Expression // LOGIC GE, not implemented yet
    | Expression OP_LSH Expression // LOGIC LEFT SHIFT, not implemented yet
    | Expression OP_RSH Expression // LOGIC RIGHT SHIFT, not implemented yet
    | '!' Expression                                        { // LOGIC NOT
                                                                curr = new Expr(OPR_NOT, 1);
                                                                curr->insert($2);
                                                                $$ = curr; }
    | '-' Expression %prec OP_NEG                           { // NEGATIVE
                                                                curr = new Expr(OPR_NEG, 1);
                                                                curr->insert($2);
                                                                $$ = curr; }
    | Integer                                               {
                                                                curr = new Expr(OPR_INT, 1);
                                                                curr->insert($1);
                                                                $$ = curr; }
    | Identifier                                            {
                                                                curr = new Expr(OPR_ID, 1);
                                                                curr->insert($1);
                                                                $$ = curr; }
    | OP_INC Identifier                                     { // INC AND RETURN
                                                                curr = new Expr(OPR_INCRET, 1);
                                                                curr->insert($2);
                                                                $$ = curr; }            
    | Identifier OP_INC                                     { // RETURN AND INC
                                                                curr = new Expr(OPR_RETINC, 1);
                                                                curr->insert($1);
                                                                $$ = curr; }
    | OP_DEC Identifier                                     { // DEC AND RETURN
                                                                curr = new Expr(OPR_DECRET, 1);
                                                                curr->insert($2);
                                                                $$ = curr; }
    | Identifier OP_DEC                                     { // RETURN AND DEC
                                                                curr = new Expr(OPR_RETDEC, 1);
                                                                curr->insert($1);
                                                                $$ = curr; }
    | Identifier '[' Expression ']'                         { // INDEX_OP
                                                                curr = new Expr(OPR_IDX, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Identifier '(' ArgList ')'                            { // FUNC_CALL
                                                                curr = new Expr(OPR_FUNC, 2);
                                                                curr->insert($1);
                                                                curr->insert($3);
                                                                $$ = curr; }
    | Identifier '(' ')'                                    { // FUNC_CALL WITH NO ARGS
                                                                curr = new Expr(OPR_FUNCNOARG, 1);
                                                                curr->insert($1);
                                                                $$ = curr; }
    ;

Identifier: 
      IDENTIFIER                                            {
                                                                curr = new Identifier($1);
                                                                $$ = curr; }
    ;

Integer: 
      INT_CONSTANT                                          {
                                                                curr = new Integer($1);
                                                                $$ = curr; }
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
