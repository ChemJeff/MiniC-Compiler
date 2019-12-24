/* Lexical analyzer corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

%{
#include <stdio.h>
#include <stdlib.h>
#include "Eeyore.y.tab.hh"

#ifdef PRINTLEX
#define printf_lex(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define printf_lex(fmt, ...)
#endif

//#define T(x) (((x) << 2) | 1)
//#define t(x) (((x) << 2) | 2)
//#define p(x) (((x) << 2) | 3)

void updateTokPos();
void printTok(int hasattr, const char* lexcls);
int var2type(const char* str);
int lineno = 1;
int tokenepos = 1;
int tokenspos = 1;
int tokensno = 0;
int tabscount = 0;                                          // for better display
int isnewline = 1;                                          // new display line
%}

integer         -?[0-9]+
function        f_[a-zA-Z_][a-zA-Z_0-9]*
variable        [Ttp][0-9]+
label           l[0-9]+
whitespace      [ \t]
eol             (\r)?\n
emptyline       ^[ \r\t]*(\r)?\n

%s SCOMMENT

%%

"["             {printTok(0, "[");      tokensno++;     return '[';}
"]"             {printTok(0, "]");      tokensno++;     return ']';}
":"             {printTok(0, ":");      tokensno++;     return ':';}

"//"            {printTok(0, "//COMMENT"); BEGIN SCOMMENT;}     // single line comment state
<SCOMMENT>.*    {updateTokPos(); /* do nothing with comments */}
<SCOMMENT>{eol} {updateTokPos(); BEGIN INITIAL; printTok(0, "EOL"); printf_lex("\n"); lineno++; isnewline = 1;      return EOL;}

{whitespace}    {updateTokPos(); /* omit optional whitespace */}
{emptyline}     {printTok(0, "EMPTYLINE"); printf_lex("\n"); lineno++; isnewline = 1;}
{eol}           {printTok(0, "EOL");       printf_lex("\n"); lineno++; isnewline = 1;       return EOL;}

"!="            {printTok(1, "OP");     tokensno++;     return OP_NE;}
"!"             {printTok(1, "OP");     tokensno++;     return '!';}
"=="            {printTok(1, "OP");     tokensno++;     return OP_EQ;}
"<"             {printTok(1, "OP");     tokensno++;     return '<';}
">"             {printTok(1, "OP");     tokensno++;     return '>';}
"&&"            {printTok(1, "OP");     tokensno++;     return OP_AND;}
"||"            {printTok(1, "OP");     tokensno++;     return OP_OR;}

"="             {printTok(1, "OP");     tokensno++;     return '=';}

"+"             {printTok(1, "OP");     tokensno++;     return '+';}
"-"             {printTok(1, "OP");     tokensno++;     return '-';}
"*"             {printTok(1, "OP");     tokensno++;     return '*';}
"/"             {printTok(1, "OP");     tokensno++;     return '/';}
"%"             {printTok(1, "OP");     tokensno++;     return '%';}

var             {printTok(0, "var");    tokensno++;     return VAR;}
if              {printTok(0, "if");     tokensno++;     return IF;}
goto            {printTok(0, "goto");   tokensno++;     return GOTO;}
return          {printTok(0, "return"); tokensno++;     return RETURN;}
param           {printTok(0, "param");  tokensno++;     return PARAM;}
call            {printTok(0, "call");   tokensno++;     return CALL;}
end             {printTok(0, "end");    tokensno++;     return END;}


{function}      {printTok(1, "func");       tokensno++;         yylval.string_value = strdup(yytext);   return FUNCTION;}
{variable}      {printTok(1, "variable");   tokensno++;         yylval.rval_value = Rval{atoi(yytext + 1), var2type(yytext)};     return VARIABLE;}
{label}         {printTok(1, "label");      tokensno++;         yylval.int_value = atoi(yytext + 1);    return LABEL;}
{integer}       {printTok(1, "integer");    tokensno++;         yylval.rval_value = Rval{atoi(yytext), VAR_IMMEDIATE};        return INTEGER;}

.               {fprintf(stderr, "Syntax error (line %d:%d): unknown symbol '%c'\n", lineno, tokenspos, *yytext);}

%%

void updateTokPos() {
    if (isnewline)
        tokenspos = 1;
    else
        tokenspos = tokenepos;
    tokenepos = tokenspos + yyleng;
}

void printTok(int hasattr, const char* lexcls) {
    updateTokPos();
    if (isnewline) {
        if (strcmp(lexcls, "end") == 0) {
            tabscount--;
        }
        if (strcmp(lexcls, "label") != 0) {
            for (int i = 0; i < tabscount; i++)
                printf_lex("    ");
        }
        if (strcmp(lexcls, "func") == 0) {
            tabscount++;
        }
        isnewline = 0;
    }
    else {
        printf_lex(" ");
    }
    if (hasattr) {
        printf_lex("<%s, %s>", lexcls, yytext);
    }
    else {
        printf_lex("<%s>", lexcls);
    }
}

int var2type(const char* str) {
    switch(str[0]){
        case 'T':
            return VAR_NATIVE;
        case 't':
            return VAR_TEMP;
        case 'p':
            return VAR_PARAM;
        default:
            fprintf(stderr, "Error (line %d:%d): invalid variable `%s`\n", lineno, tokenspos, str);
            ;
    }
    return atoi(str + 1) << 2;
}

int yywrap() {
    printf_lex("\nline count: %d\n", lineno);
    printf_lex("tokens count: %d\n", tokensno);
    printf_lex(">>>>>>>>>>>>>> End of lexical analysis <<<<<<<<<<<<<<\n\n");
    return 1;
}