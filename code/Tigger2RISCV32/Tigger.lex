/* Lexical analyzer corresponding to Tigger grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

%{
#include <stdio.h>
#include <stdlib.h>
#include "Tigger.y.tab.hh"
#include "Tigger.typedef.hh"

#ifdef PRINTLEX
#define printf_lex(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define printf_lex(fmt, ...)
#endif

void updateTokPos();
void printTok(int hasattr, const char* lexcls);
int reg2idx(const char* str);
int lineno = 1;
int tokenepos = 1;
int tokenspos = 1;
int tokensno = 0;
int tabscount = 0;                                          // for better display
int isnewline = 1;  

%}

integer         -?[0-9]+
function        f_[a-zA-Z_][a-zA-Z_0-9]*
label           l[0-9]+
variable        v[0-9]+
reg             x0|s0|s1|s2|s3|s4|s5|s6|s7|s8|s9|s10|s11|a0|a1|a2|a3|a4|a5|a6|a7|t0|t1|t2|t3|t4|t5|t6
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

malloc          {printTok(0, "malloc"); tokensno++;     return MALLOC;}
load            {printTok(0, "load");   tokensno++;     return LOAD;}
store           {printTok(0, "store");  tokensno++;     return STORE;}
loadaddr        {printTok(0, "loadaddr");tokensno++;    return LOADADDR;}
if              {printTok(0, "if");     tokensno++;     return IF;}
goto            {printTok(0, "goto");   tokensno++;     return GOTO;}
return          {printTok(0, "return"); tokensno++;     return RETURN;}
call            {printTok(0, "call");   tokensno++;     return CALL;}
end             {printTok(0, "end");    tokensno++;     return END;}


{function}      {printTok(1, "func");       tokensno++;         yylval.string_value = strdup(yytext);   return FUNCTION;}
{variable}      {printTok(1, "variable");   tokensno++;         yylval.int_value = atoi(yytext + 1);     return VARIABLE;}
{label}         {printTok(1, "label");      tokensno++;         yylval.int_value = atoi(yytext + 1);    return LABEL;}
{integer}       {printTok(1, "integer");    tokensno++;         yylval.int_value = atoi(yytext);        return INTEGER;}
{reg}           {printTok(1, "reg");        tokensno++;         yylval.int_value = reg2idx(yytext);     return REG;}

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

int reg2idx(const char* str) {
    int reg_base, ofst;
    switch(str[0]){
        case 'x': reg_base = 0; break;
        case 's': reg_base = REG_S; break;
        case 'a': reg_base = REG_A; break;
        case 't': reg_base = REG_T; break;
        default: fprintf(stderr, "Error (line %d:%d): invalid register `%s`\n", lineno, tokenspos, str);
    }
    ofst = atoi(str + 1);
    return reg_base + ofst;
}

int yywrap() {
    printf_lex("\nline count: %d\n", lineno);
    printf_lex("tokens count: %d\n", tokensno);
    printf_lex(">>>>>>>>>>>>>> End of lexical analysis <<<<<<<<<<<<<<\n\n");
    return 1;
}
