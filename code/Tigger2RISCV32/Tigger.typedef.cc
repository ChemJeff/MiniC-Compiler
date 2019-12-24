/* Type definitions corresponding to Tigger grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include "Tigger.typedef.hh"

int Expr::expr_cnt = 0;

Expr* exprTab[MAX_EXPRS];
int expr_cnt = 0;

// // NOTE: these indexes is not in consistence with the RISC-V spec for convenience
// const char idx2reg[32][8] = {
// 	"x0",
// 	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
// 	"t0", "t1", "t2", "t3", "t4", "t5", "t6",
// 	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
// }; 
// // from id to ABI name
// // 0: x0; 1~12: a0-a11, callee-saved; 13~20: t0-t7, caller-saved; 21~27: a0-a7, caller-saved 

ExprPrinter exprPrinter;

void print_expr() {
    for (int i=0; i<expr_cnt; i++)
        exprTab[i]->accept(exprPrinter);
}