/* Lexical analyzer corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

%{
#include <stdio.h>
#include <stdlib.h>
#include "MiniC.y.tab.hh"

#ifdef PRINTLEX
#define printf_lex(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define printf_lex(fmt, ...)
#endif

void updateTokPos();
void printTok(int hasattr, const char* lexcls);
int lineno = 1;
int tokenepos = 1;
int tokenspos = 1;
int tokensno = 0;
int tabscount = 0;                                          // for better display
int isnewline = 1;                                          // new display line
%}

alpha           [A-Za-z_]
digit           [0-9]
integer         ([0-9])+
identifier      [a-zA-Z_]([a-zA-Z_0-9])*
whitespace      [ \t]
eol             (\r)?\n
emptyline       ^([ \r\t])*(\r)?\n

%s SCOMMENT
%s MCOMMENT                                                 

%%

"{"             {printTok(0, "{");      tabscount++;           tokensno++;      return '{';}
"}"             {tabscount--;           printTok(0, "}");      tokensno++;      return '}';}
"("             {printTok(0, "(");      tokensno++;     return '(';}
")"             {printTok(0, ")");      tokensno++;     return ')';}
"["             {printTok(0, "[");      tokensno++;     return '[';}
"]"             {printTok(0, "]");      tokensno++;     return ']';}
","             {printTok(0, ",");      tokensno++;     return ',';}
";"             {printTok(0, ";");      tokensno++;     return ';';}

"//"            {printTok(0, "//COMMENT"); BEGIN SCOMMENT;}     // single line comment state
<SCOMMENT>.*    {updateTokPos(); /* do nothing with comments */}
<SCOMMENT>{eol} {updateTokPos(); BEGIN INITIAL; printTok(0, "EOL"); printf_lex("\n"); lineno++; isnewline = 1;}

"/*"            {printTok(0, "/*COMMENT*/"); BEGIN MCOMMENT;}   // multiple line comment state
<MCOMMENT>[^*\n]* {updateTokPos(); /* do nothing with comments */}
<MCOMMENT>\*/[^/] {updateTokPos(); /* do nothing with comments */}
<MCOMMENT>"*/"  {updateTokPos(); BEGIN INITIAL; /* back to initial state (0) */}
<MCOMMENT>{eol} {lineno++;}


{whitespace}    {updateTokPos(); /* omit optional whitespace */}
{emptyline}     {printTok(0, "EMPTYLINE"); printf_lex("\n"); lineno++; isnewline = 1;}
{eol}           {printTok(0, "EOL");       printf_lex("\n"); lineno++; isnewline = 1;}

"!="            {printTok(1, "OP");     tokensno++;     return OP_NE;}
"!"             {printTok(1, "OP");     tokensno++;     return '!';}
"=="            {printTok(1, "OP");     tokensno++;     return OP_EQ;}
">="            {printTok(1, "OP");     tokensno++;     return OP_GE;}        // not implemented yet
"<="            {printTok(1, "OP");     tokensno++;     return OP_LE;}        // not implemented yet
"<"             {printTok(1, "OP");     tokensno++;     return '<';}
">"             {printTok(1, "OP");     tokensno++;     return '>';}
"="             {printTok(1, "OP");     tokensno++;     return '=';}
"&&"            {printTok(1, "OP");     tokensno++;     return OP_AND;}
"||"            {printTok(1, "OP");     tokensno++;     return OP_OR;}

"++"            {printTok(1, "OP");     tokensno++;     return OP_INC;}
"--"            {printTok(1, "OP");     tokensno++;     return OP_DEC;}

"+"             {printTok(1, "OP");     tokensno++;     return '+';}
"-"             {printTok(1, "OP");     tokensno++;     return '-';}
"*"             {printTok(1, "OP");     tokensno++;     return '*';}
"/"             {printTok(1, "OP");     tokensno++;     return '/';}
"%"             {printTok(1, "OP");     tokensno++;     return '%';}

"<<"            {printTok(1, "OP");     tokensno++;     return OP_LSH;}       // not implemented yet
">>"            {printTok(1, "OP");     tokensno++;     return OP_RSH;}       // not implemented yet

int             {printTok(0, "int");    tokensno++;     return INT;}
if              {printTok(0, "if");     tokensno++;     return IF;}
else            {printTok(0, "else");   tokensno++;     return ELSE;}
while           {printTok(0, "while");  tokensno++;     return WHILE;}
return          {printTok(0, "return"); tokensno++;     return RETURN;}

"break"         {printTok(0, "break");  tokensno++;     return BREAK;}        // not implemented yet
"continue"      {printTok(0, "continue"); tokensno++;   return CONTINUE;}     // not implemented yet

"main"          {printTok(1, "ID");     tokensno++;     return MAIN;}

{identifier}    {printTok(1, "ID");     tokensno++;     yylval.string_value = strdup(yytext); return IDENTIFIER;}
{integer}       {printTok(1, "INT");    tokensno++;     yylval.int_value = atoi(yytext);    return INT_CONSTANT;}

.               {fprintf(stderr, "Syntax error (line %d:%d): unknown symbol '%c'\n", lineno, tokenspos,*yytext);}

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
        for (int i = 0; i < tabscount; i++)
            printf_lex("    ");
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

int yywrap() {
    printf_lex("\nline count: %d\n", lineno);
    printf_lex("tokens count: %d\n", tokensno);
    printf_lex(">>>>>>>>>>>>>> End of lexical analysis <<<<<<<<<<<<<<\n\n");
    return 1;
}
