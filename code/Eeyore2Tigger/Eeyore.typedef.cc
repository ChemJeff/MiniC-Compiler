/* Type definitions corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include "Eeyore.typedef.hh"

int Expr::expr_cnt = 0;
int Var::var_cnt = 0;
int Var::global_cnt = 0;
int Istr::istr_cnt = 0;

Expr* exprTab[MAX_EXPRS];
Var* varTab[MAX_VARS];
Istr* istrTab[MAX_ISTRS];
int expr_cnt = 0;
int func_cnt = 0;
int var_cnt = 0;
int istr_cnt = 0;

ExprPrinter exprPrinter;

void print_expr() {
    for (int i=0; i<expr_cnt; i++)
        exprTab[i]->accept(exprPrinter);
}