/* Type definitions corresponding to Tigger grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __TIGGER_TYPEDEF_H__
#define __TIGGER_TYPEDEF_H__


#ifdef DEBUGTYPE
#define dprintf_type(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_type(fmt, ...)
#endif

#define MAX_EXPRS 8192

#define EXPR_NONE 0
#define EXPR_FUNC 1
#define EXPR_GLOBAL_DECL 2
#define EXPR_OP2 3
#define EXPR_OP1 4
#define EXPR_ASS 5
#define EXPR_CONDI_GOTO 6
#define EXPR_GOTO 7
#define EXPR_LABEL 8
#define EXPR_CALL 9
#define EXPR_STORE 10
#define EXPR_LOAD 11
#define EXPR_LOAD_ADDR 12
#define EXPR_RET 13
#define EXPR_FUNC_END 14

// define as 'OPR_*' to avoid conflict with token 'OP_*'
#define OPR_NONE 0
#define OPR_ADD 1
#define OPR_SUB 2
#define OPR_MUL 3
#define OPR_DIV 4
#define OPR_MOD 5
#define OPR_ASS 6
#define OPR_AND 7
#define OPR_OR 8
#define OPR_LT 9
#define OPR_GT 10
#define OPR_EQ 11
#define OPR_NE 12
#define OPR_LE 13        // not implemented yet
#define OPR_GE 14        // not implemented yet
#define OPR_LSH 15       // not implemented yet
#define OPR_RSH 16       // not implemented yet
#define OPR_NOT 17
#define OPR_NEG 18
#define OPR_INT 19       // not appliable here
#define OPR_ID 20        // not appliable here
#define OPR_INCRET 21    // not appliable here
#define OPR_RETINC 22    // not appliable here
#define OPR_DECRET 23    // not appliable here
#define OPR_RETDEC 24    // not appliable here
#define OPR_IDX 25       // not appliable here
#define OPR_FUNC 26      // not appliable here
#define OPR_FUNCNOARG 27 // not appliable here

#define ASS_NONE 0
#define ASS_NO_ARRAY 1
#define ASS_ARRAY_LEFT 2
#define ASS_ARRAY_RIGHT 3

#define REG_S 1
#define REG_T 13
#define REG_A 20
#define REG_MIN 1
#define REG_MAX 28 // not including

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int lineno;
extern int tokenspos;

struct Expr;
struct ExprVisitor;

struct Func;
struct GlobalDecl;
struct UnaryOp;
struct BinaryOp;
struct Assign;
struct CondiGoto;
struct Goto;
struct Label;
struct Call;
struct Store;
struct Load;
struct LoadAddr;
struct Ret;
struct FuncEnd;

// Abstract class
struct ExprVisitor {
    char* name;
    virtual void visit(Func*) = 0;
    virtual void visit(GlobalDecl*) = 0;
    virtual void visit(UnaryOp*) = 0;
    virtual void visit(BinaryOp*) = 0;
    virtual void visit(Assign*) = 0;
    virtual void visit(CondiGoto*) = 0;
    virtual void visit(Goto*) = 0;
    virtual void visit(Label*) = 0;
    virtual void visit(Call*) = 0;
    virtual void visit(Store*) = 0;
    virtual void visit(Load*) = 0;
    virtual void visit(LoadAddr*) = 0;
    virtual void visit(Ret*) = 0;
    virtual void visit(FuncEnd*) = 0;
};

struct Expr {
    static int expr_cnt;

    int index;
    int expr_type;

    virtual void accept(ExprVisitor &visitor) = 0;

    Expr() {
        this->index = Expr::expr_cnt++;
    }
    ~Expr() {Expr::expr_cnt--;}
};

struct Func: public Expr {
    char* name;
    int num_param;
    int stack_size;
    Func(char* _name, int _num_param, int _stack_size): Expr() {
        this->name = strdup(_name);
        this->num_param = _num_param;
        this->stack_size = _stack_size;
        this->expr_type = EXPR_FUNC;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Func visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct GlobalDecl: public Expr {
    // global variables declartion
    int global_idx;
    bool isArray;
    int arraySize;
    GlobalDecl(int _global_idx, bool _isArray, int _arraySize=0): Expr() {
        this->global_idx = _global_idx;
        this->isArray = _isArray;
        this->arraySize = _arraySize;
        this->expr_type = EXPR_GLOBAL_DECL;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: GlobalDecl visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct UnaryOp: public Expr {
    int dst_reg;
    int op;
    int src_reg;
    UnaryOp(int _dst_reg, int _op, int _src_reg): Expr() {
        this->dst_reg = _dst_reg;
        this->op = _op;
        this->src_reg = _src_reg;
        this->expr_type = EXPR_OP1;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: UnaryOp visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct BinaryOp: public Expr {
    int dst_reg;
    int src1_reg;
    int op;
    bool isSrc2Reg;
    int src2;
    BinaryOp(int _dst_reg, int _src1_reg, int _op, bool _isSrc2Reg, int _src2): Expr() {
        this->dst_reg = _dst_reg;
        this->src1_reg = _src1_reg;
        this->op = _op;
        this->isSrc2Reg = _isSrc2Reg;
        this->src2 = _src2;
        this->expr_type = EXPR_OP2;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: BinaryOp visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Assign: public Expr {
    int dst_reg;
    int src;
    bool isSrcReg;
    int array_ofst;
    int ass_type;
    Assign(int _dst_reg, bool _isSrcReg, int _src, int _array_ofst, int _ass_type): Expr() {
        this->dst_reg = _dst_reg;
        this->isSrcReg = _isSrcReg;
        this->src = _src;
        this->array_ofst = _array_ofst;
        this->ass_type = _ass_type;
        this->expr_type = EXPR_ASS;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Assign visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct CondiGoto: public Expr {
    int opr1_reg;
    int logiop;
    int opr2_reg;
    int label;
    CondiGoto(int _opr1_reg, int _logiop, int _opr2_reg, int _label): Expr() {
        this->opr1_reg = _opr1_reg;
        this->logiop = _logiop;
        this->opr2_reg = _opr2_reg;
        this->label = _label;
        this->expr_type = EXPR_CONDI_GOTO;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: CondiGoto visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Goto: public Expr {
    int label;
    Goto(int _label): Expr() {
        this->label = _label;
        this->expr_type = EXPR_LABEL;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Goto visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Label: public Expr {
    int label_idx;
    Label(int _label_idx): Expr() {
        this->label_idx = _label_idx;
        this->expr_type = EXPR_LABEL;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Label visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Call: public Expr {
    char* name;
    Call(char* _name): Expr() {
        this->name = strdup(_name);
        this->expr_type = EXPR_CALL;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Call visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Store: public Expr {
    int src_reg;
    int dst_imm;
    Store(int _src_reg, int _dst_imm): Expr() {
        this->src_reg = _src_reg;
        this->dst_imm = _dst_imm;
        this->expr_type = EXPR_STORE;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Store visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Load: public Expr {
    int dst_reg;
    bool isSrcGlobal;
    int src;
    Load(bool _isSrcGlobal, int _src, int _dst_reg): Expr() {
        this->isSrcGlobal = _isSrcGlobal;
        this->src = _src;
        this->dst_reg = _dst_reg;
        this->expr_type = EXPR_LOAD;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Load visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct LoadAddr: public Expr {
    int dst_reg;
    bool isSrcGlobal;
    int src;
    LoadAddr(bool _isSrcGlobal, int _src, int _dst_reg): Expr() {
        this->isSrcGlobal = _isSrcGlobal;
        this->src = _src;
        this->dst_reg = _dst_reg;
        this->expr_type = EXPR_LOAD_ADDR;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: LoadAddr visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}   
};

struct Ret: public Expr {
    Ret(): Expr() {
        this->expr_type = EXPR_RET;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Ret visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}   
};

struct FuncEnd: public Expr {
    char* name;
    FuncEnd(char* _name): Expr() {
        this->name = strdup(_name);
        this->expr_type = EXPR_FUNC_END;
    }
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: FuncEnd visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct ExprPrinter: public ExprVisitor {
    ExprPrinter(): ExprVisitor() {this->name = strdup("ExprPrinter");}
    ~ExprPrinter() {delete this->name;}
    virtual void visit(Func* func) {
        fprintf(stderr, "%d: Func (name = %s, num_param = %d, stack_size = %d)\n", func->index, func->name, func->num_param, func->stack_size);
    }
    virtual void visit(GlobalDecl* decl) {
        if (decl->isArray) {
            fprintf(stderr, "%d: GlobalDecl (global_idx = %d, arraySize = %d)\n", decl->index, decl->global_idx, decl->arraySize);
        }
        else {
            fprintf(stderr, "%d: GlobalDecl (global_idx = %d, val = %d)\n", decl->index, decl->global_idx, decl->arraySize);
        }
    }
    virtual void visit(UnaryOp* unaryop) {
        fprintf(stderr, "%d: UnaryOp (dst_reg = %d, op = %d, src_reg = %d)\n", unaryop->index, unaryop->dst_reg, unaryop->op, unaryop->src_reg);
    }
    virtual void visit(BinaryOp* binaryop) {
        fprintf(stderr, "%d: BinaryOp (dst_reg = %d, src1_reg = %d, op = %d, isSrc2Reg = %d, src2 = %d)\n", binaryop->index,
         binaryop->dst_reg, binaryop->src1_reg, binaryop->op, binaryop->isSrc2Reg, binaryop->src2);
    }
    virtual void visit(Assign* assign) {
        fprintf(stderr, "%d: Assign (dst_reg = %d, isSrc2Reg = %d, src = %d, array_ofst = %d, ass_type = %d)\n",
         assign->index, assign->dst_reg, assign->isSrcReg, assign->src, assign->array_ofst, assign->ass_type);
    }
    virtual void visit(CondiGoto* condigoto) {
        fprintf(stderr, "%d: CondiGoto (opr1_reg = %d, logiop = %d, opr2_reg = %d, label = %d)\n", condigoto->index, 
         condigoto->opr1_reg, condigoto->logiop, condigoto->opr2_reg, condigoto->label);
    }
    virtual void visit(Goto* got) {
        fprintf(stderr, "%d: Goto (label = %d)\n", got->index, got->label);
    }
    virtual void visit(Label* label) {
        fprintf(stderr, "%d: Label (label_idx = %d)\n", label->index, label->label_idx);
    }
    virtual void visit(Call* call) {
        fprintf(stderr, "%d: Call (name = %s)\n", call->index, call->name);
    }
    virtual void visit(Store* store) {
        fprintf(stderr, "%d: Store (src_reg = %d, dst_imm = %d)\n", store->index, store->src_reg, store->dst_imm);
    }
    virtual void visit(Load* load) {
        fprintf(stderr, "%d: Load (isSrcGlobal = %d, src = %d, dst_reg = %d)\n", load->index,
         load->isSrcGlobal, load->src, load->dst_reg);
    } 
    virtual void visit(LoadAddr* loadaddr) {
        fprintf(stderr, "%d: LoadAddr (isSrcGlobal = %d, src = %d, dst_reg = %d)\n", loadaddr->index,
         loadaddr->isSrcGlobal, loadaddr->src, loadaddr->dst_reg);
    }
    virtual void visit(Ret* ret) {
        fprintf(stderr, "%d: Ret\n", ret->index);
    }
    virtual void visit(FuncEnd* funcend) {
        fprintf(stderr, "%d: FuncEnd (name = %s)\n", funcend->index, funcend->name);
    }
};

void print_expr();
// NOTE: these indexes is not in consistence with the RISC-V spec for convenience
const char idx2reg[32][8] = {
    "x0",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
}; 
// from id to ABI name
// 0: x0; 1~12: a0-a11, callee-saved; 13~20: t0-t7, caller-saved; 21~27: a0-a7, caller-saved 


#endif