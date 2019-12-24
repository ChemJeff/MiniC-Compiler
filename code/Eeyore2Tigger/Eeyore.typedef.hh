/* Type definitions corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __EEYORE_TYPEDEF_H__
#define __EEYORE_TYPEDEF_H__


#ifdef DEBUGTYPE
#define dprintf_type(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_type(fmt, ...)
#endif

#define MAX_EXPRS 4096
#define MAX_VARS 1024
#define MAX_ISTRS 8192
#define MAX_FUNC_PARAM 8

#define EXPR_NONE 0
#define EXPR_FUNC 1
#define EXPR_DECL 2
#define EXPR_OP2 3
#define EXPR_OP1 4
#define EXPR_ASS 5
#define EXPR_CONDI_GOTO 6
#define EXPR_GOTO 7
#define EXPR_LABEL 8
#define EXPR_PARAM 9
#define EXPR_CALL 10
#define EXPR_RET 11
#define EXPR_FUNC_END 12

#define ISTR_NONE 0
#define ISTR_FUNC 1
#define ISTR_DECL 2
#define ISTR_OP2 3
#define ISTR_OP1 4
#define ISTR_ASS 5
#define ISTR_CONDI_GOTO 6
#define ISTR_GOTO 7
#define ISTR_LABEL 8
#define ISTR_CALL 9
#define ISTR_STORE 10
#define ISTR_LOAD 11
#define ISTR_LOAD_ADDR 12
#define ISTR_RET 13
#define ISTR_FUNC_END 14

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
#define OPR_INT 19 		 // not appliable here
#define OPR_ID 20 		 // not appliable here
#define OPR_INCRET 21 	 // not appliable here
#define OPR_RETINC 22 	 // not appliable here
#define OPR_DECRET 23 	 // not appliable here
#define OPR_RETDEC 24 	 // not appliable here
#define OPR_IDX 25 		 // not appliable here
#define OPR_FUNC 26 	 // not appliable here
#define OPR_FUNCNOARG 27 // not appliable here

#define VAR_IMMEDIATE 0
#define VAR_NATIVE 1
#define VAR_TEMP 2
#define VAR_PARAM 3

#define ASS_NONE 0
#define ASS_NO_ARRAY 1
#define ASS_ARRAY_LEFT 2
#define ASS_ARRAY_RIGHT 3

#define ALLOC_NONE 0
#define ALLOC_REG 1
#define ALLOC_STK 2

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int lineno;
extern int tokenspos;

struct Rval;

struct Expr;
struct ExprVisitor;
struct Istr;
struct IstrVisitor;
struct Var;

struct Func;
struct Decl;
struct UnaryOp;
struct BinaryOp;
struct Assign;
struct CondiGoto;
struct Goto;
struct Label;
struct Param;
struct Call;
struct Ret;
struct FuncEnd;

struct Istr_Func;
struct Istr_Decl;
struct Istr_Op1;
struct Istr_Op2;
struct Istr_Assign;
struct Istr_CondiGoto;
struct Istr_Goto;
struct Istr_Label;
struct Istr_Call;
struct Istr_Store;
struct Istr_Load;
struct Istr_LoadAddr;
struct Istr_Ret;
struct Istr_FuncEnd;


// struct for distinguishing different types of var/const
struct Rval {
	int val;
	int var_type;

	bool operator==(Rval &other) {
		return (this->val == other.val) && (this->var_type == other.var_type);
	}
};

// Abstract class
struct ExprVisitor {
    char* name;
    virtual void visit(Func*) = 0;
    virtual void visit(Decl*) = 0;
    virtual void visit(UnaryOp*) = 0;
    virtual void visit(BinaryOp*) = 0;
    virtual void visit(Assign*) = 0;
    virtual void visit(CondiGoto*) = 0;
    virtual void visit(Goto*) = 0;
    virtual void visit(Label*) = 0;
    virtual void visit(Param*) = 0;
    virtual void visit(Call*) = 0;
    virtual void visit(Ret*) = 0;
    virtual void visit(FuncEnd*) = 0;
};

struct IstrVisitor {
    char* name;
    virtual void visit(Istr_Func*) = 0;
    virtual void visit(Istr_Decl*) = 0;
    virtual void visit(Istr_Op1*) = 0;
    virtual void visit(Istr_Op2*) = 0;
    virtual void visit(Istr_Assign*) = 0;
    virtual void visit(Istr_CondiGoto*) = 0;
    virtual void visit(Istr_Goto*) = 0;
    virtual void visit(Istr_Label*) = 0;
    virtual void visit(Istr_Call*) = 0;
    virtual void visit(Istr_Store*) = 0;
    virtual void visit(Istr_Load*) = 0;
    virtual void visit(Istr_LoadAddr*) = 0;
    virtual void visit(Istr_Ret*) = 0;
    virtual void visit(Istr_FuncEnd*) = 0;
};

struct Expr {
	static int expr_cnt;

	int index;
	int expr_type;
	bool isBlockEntry;		// true if it's the first instrcution of the basic block
	bool isBlockExit;		// true if it's the last instruction of the basic block

	virtual void accept(ExprVisitor &visitor) = 0;

	Expr() {
		this->index = Expr::expr_cnt++;
		this->isBlockEntry = false;
	}
	~Expr() {Expr::expr_cnt--;}
};

struct Istr {
	static int istr_cnt;

	int index;
	int istr_type;

	virtual void accept(IstrVisitor &visitor) = 0;

	Istr() {
		this->index = Istr::istr_cnt++;
	}
	~Istr() {Istr::istr_cnt--;}
};

struct Func: public Expr {
	char* name;
	int num_param;
	Func(char* _name, int _num_param): Expr() {
		this->name = strdup(_name);
		this->num_param = _num_param;
		this->expr_type = EXPR_FUNC;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Func visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Decl: public Expr {
	Rval val;
	int arraySize;		// in unit of `bytes`, not `ints`
	bool isArray;
	bool isGlobal;
	Decl(Rval _val, bool _isGlobal, int _arraySize): Expr() {
		this->val = _val;
		this->isGlobal = _isGlobal;
		if (_arraySize == 0) {
			this->isArray = false;
		}
		else {
			this->isArray = true;
		}
		this->arraySize = _arraySize;
	}
    void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Decl visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct BinaryOp: public Expr {
	Rval dst;
	Rval opr1;
	int op;
	Rval opr2;
	BinaryOp(Rval _dst, Rval _opr1, int _op, Rval _opr2): Expr() {
		this->dst = _dst;
		this->opr1 = _opr1;
		this->op = _op;
		this->opr2 = _opr2;
		this->expr_type = EXPR_OP2;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: BinaryOp visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct UnaryOp: public Expr {
	Rval dst;
	int op;
	Rval opr;
	UnaryOp(Rval _dst, int _op, Rval _opr): Expr() {
		this->dst = _dst;
		this->op = _op;
		this->opr = _opr;
		this->expr_type = EXPR_OP1;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: UnaryOp visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Assign: public Expr {
	Rval dst;
	Rval ofst_dst;
	Rval src;
	Rval ofst_src;
	int ass_type;
	Assign(Rval _dst, Rval _ofst_dst, Rval _src, Rval _ofst_src, int _ass_type): Expr() {
		assert(_ass_type);
		this->dst = _dst;
		this->ofst_dst = _ofst_dst;
		this->src = _src;
		this->ofst_src = _ofst_src;
		this->ass_type = _ass_type;
		this->expr_type = EXPR_ASS;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Assign visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct CondiGoto: public Expr {
	Rval opr1;
	int logiop;
	Rval opr2;
	int label;
	CondiGoto(Rval _opr1, int _logiop, Rval _opr2, int _label): Expr() {
		this->opr1 = _opr1;
		this->logiop = _logiop;
		this->opr2 = _opr2;
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
		this->expr_type = EXPR_GOTO;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Goto visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Label: public Expr {
	int val;
	Label(int _val): Expr() {
		this->val = _val;
		this->expr_type = EXPR_LABEL;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Label visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Param: public Expr {
	Rval val;
	Param(Rval _val): Expr() {
		this->val = _val;
		this->expr_type = EXPR_PARAM;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Param visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Call: public Expr {
	Rval dst;
	bool hasDst;
	char* name;
	Call(char* _name, bool _hasDst, Rval _dst): Expr() {
		this->name = strdup(_name);
		this->hasDst = _hasDst;
		this->dst = _dst;
		this->expr_type = EXPR_CALL;
	}
	void accept(ExprVisitor& visitor) override {
        dprintf_type("%d: Call visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Ret: public Expr {
	Rval val;
	Ret(Rval _val): Expr() {
		this->val = _val;
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

struct Istr_Func: public Istr {
	char* name;
	int num_param;
	int stack_size;
	Istr_Func(char* _name, int _num_param): Istr() {
		this->name = strdup(_name);
		this->num_param = _num_param;
		this->stack_size = -1; // need to be calcuated later
		this->istr_type = ISTR_FUNC;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Func visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Decl: public Istr {
	// global variables declartion
	int global_idx;
	bool isArray;
	int val;
	Istr_Decl(int _global_idx, bool _isArray, int _val): Istr() {
		this->global_idx = _global_idx;
		this->isArray = _isArray;
		this->val = _val;
		this->istr_type = ISTR_DECL;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Decl visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Op1: public Istr {
	int dst_reg;
	int op;
	int src_reg;
	Istr_Op1(int _dst_reg, int _op, int _src_reg): Istr() {
		this->dst_reg = _dst_reg;
		this->op = _op;
		this->src_reg = _src_reg;
		this->istr_type = ISTR_OP1;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Op1 visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Op2: public Istr {
	int dst_reg;
	int src1_reg;
	int op;
	bool isSrc2Reg;
	int src2;
	Istr_Op2(int _dst_reg, int _src1_reg, int _op, bool _isSrc2Reg, int _src2): Istr() {
		this->dst_reg = _dst_reg;
		this->src1_reg = _src1_reg;
		this->op = _op;
		this->isSrc2Reg = _isSrc2Reg;
		this->src2 = _src2;
		this->istr_type = ISTR_OP2;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Op2 visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Assign: public Istr {
	int dst_reg;
	int src;
	bool isSrcReg;
	int array_ofst;
	int ass_type;
	Istr_Assign(int _dst_reg, bool _isSrcReg, int _src, int _array_ofst, int _ass_type): Istr() {
		this->dst_reg = _dst_reg;
		this->isSrcReg = _isSrcReg;
		this->src = _src;
		this->array_ofst = _array_ofst;
		this->ass_type = _ass_type;
		this->istr_type = ISTR_ASS;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Assign visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_CondiGoto: public Istr {
	int opr1_reg;
	int logiop;
	int opr2_reg;
	int label;
	Istr_CondiGoto(int _opr1_reg, int _logiop, int _opr2_reg, int _label): Istr() {
		this->opr1_reg = _opr1_reg;
		this->logiop = _logiop;
		this->opr2_reg = _opr2_reg;
		this->label = _label;
		this->istr_type = ISTR_CONDI_GOTO;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_CondiGoto visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Goto: public Istr {
	int label;
	Istr_Goto(int _label): Istr() {
		this->label = _label;
		this->istr_type = ISTR_GOTO;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Goto visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Label: public Istr {
	int label_idx;
	Istr_Label(int _label_idx): Istr() {
		this->label_idx = _label_idx;
		this->istr_type = ISTR_LABEL;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Label visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Call: public Istr {
	char* name;
	Istr_Call(char* _name): Istr() {
		this->name = strdup(_name);
		this->istr_type = ISTR_CALL;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Call visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Store: public Istr {
	int src_reg;
	int dst_imm;
	Istr_Store(int _src_reg, int _dst_imm): Istr() {
		this->src_reg = _src_reg;
		this->dst_imm = _dst_imm;
		this->istr_type = ISTR_STORE;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Store visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_Load: public Istr {
	int dst_reg;
	bool isSrcGlobal;
	int src;
	Istr_Load(int _dst_reg, bool _isSrcGlobal, int _src): Istr() {
		this->dst_reg = _dst_reg;
		this->isSrcGlobal = _isSrcGlobal;
		this->src = _src;
		this->istr_type = ISTR_LOAD;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Load visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Istr_LoadAddr: public Istr {
	int dst_reg;
	bool isSrcGlobal;
	int src;
	Istr_LoadAddr(int _dst_reg, bool _isSrcGlobal, int _src): Istr() {
		this->dst_reg = _dst_reg;
		this->isSrcGlobal = _isSrcGlobal;
		this->src = _src;
		this->istr_type = ISTR_LOAD_ADDR;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_LoadAddr visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}	
};

struct Istr_Ret: public Istr {
	Istr_Ret(): Istr() {
		this->istr_type = ISTR_RET;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_Ret visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}	
};

struct Istr_FuncEnd: public Istr {
	char* name;
	Istr_FuncEnd(char* _name): Istr() {
		this->name = strdup(_name);
		this->istr_type = ISTR_FUNC_END;
	}
	void accept(IstrVisitor& visitor) override {
        dprintf_type("%d: Istr_FuncEnd visited by %s\n", this->index, visitor.name);
        visitor.visit(this);}
};

struct Var {
	static int var_cnt;
	static int global_cnt;

	int index;
	int val;
	int var_type;
	bool isGlobal;
	bool isArray;
	int arraySize;		// in unit of `bytes`, not `ints`
	int globalID;		// only if it's a global variable
	int liveBegin;		// begin of liveness interval (including), in `expr_index`
	int liveEnd;		// end of liveness interval (not including), in `expr_index`
	int assignedReg;	// >0 for alloced variables, [1, 27]
	int allocedStk;		// >=0 for ofst to stack, in `int`, <0 for not alloc stk yet
	int allocedStatus;	// 0 for not initialized, 1 for in reg, 2 for in stack

	Var(Rval _rval, bool _isGlobal, int _arraySize) {
		this->index = Var::var_cnt++;
		this->var_type = _rval.var_type;
		this->val = _rval.val;
		this->isGlobal = _isGlobal;
		if (_isGlobal) {
			this->globalID = Var::global_cnt++;
		}
		if (_arraySize == 0) {
			assert(this->var_type);
			this->isArray = false;
		}
		else {
			this->isArray = true;
		}
		this->arraySize = _arraySize;
		this->liveBegin = 0x7fffffff;
		this->liveEnd = 0;
		this->assignedReg = -1;
		this->allocedStk = -1;
		this->allocedStatus = ALLOC_NONE;
	}
	~Var() {Var::var_cnt--;}
};

struct ExprPrinter: public ExprVisitor {
    ExprPrinter(): ExprVisitor() {this->name = strdup("ExprPrinter");}
    ~ExprPrinter() {delete this->name;}
    virtual void visit(Func* func) {
    	fprintf(stderr, "%d: Func(name = %s, num_param = %d)\n", func->index, func->name, func->num_param);
    }
    virtual void visit(Decl* decl) {
    	fprintf(stderr, "%d: Decl(val = {%d, %d}, isGlobal = %d, isArray = %d, arraySize = %d)\n", decl->index, decl->val.var_type, decl->val.val,
    	 decl->isGlobal, decl->isArray, decl->arraySize);
    }
    virtual void visit(UnaryOp* unaryop) {
    	fprintf(stderr, "%d: UnaryOp(dst = {%d, %d}, op = %d, opr = {%d, %d})\n", unaryop->index, unaryop->dst.var_type, unaryop->dst.val,
    	 unaryop->op, unaryop->opr.var_type, unaryop->opr.val);
    }
    virtual void visit(BinaryOp* binaryop) {
    	fprintf(stderr, "%d: BinaryOp(dst = {%d, %d}, opr1 = {%d, %d}, op = %d, opr2 = {%d, %d})\n", binaryop->index,
    	 binaryop->dst.var_type, binaryop->dst.val, binaryop->opr1.var_type, binaryop->opr1.val, 
    	 binaryop->op, binaryop->opr2.var_type, binaryop->opr2.val);
    }
    virtual void visit(Assign* assign) {
    	fprintf(stderr, "%d: Assign(dst = {%d, %d}, ofst_dst = {%d, %d}, src = {%d, %d}, ofst_src = {%d, %d}, ass_type = %d)\n",
    	 assign->index, assign->dst.var_type, assign->dst.val, assign->ofst_dst.var_type, assign->ofst_dst.val, 
    	 assign->src.var_type, assign->src.val, assign->ofst_src.var_type, assign->ofst_src.val, assign->ass_type);
    }
    virtual void visit(CondiGoto* condigoto) {
    	fprintf(stderr, "%d: CondiGoto(opr1 = {%d, %d}, logiop = %d, opr2 = {%d, %d}, label = %d)\n", condigoto->index, condigoto->opr1.var_type, condigoto->opr1.val,
    	 condigoto->logiop, condigoto->opr2.var_type, condigoto->opr2.val, condigoto->label);
    }
    virtual void visit(Goto* got) {
    	fprintf(stderr, "%d: Goto(label = %d)\n", got->index, got->label);
    }
    virtual void visit(Label* label) {
    	fprintf(stderr, "%d: Label(val = %d)\n", label->index, label->val);
    }
    virtual void visit(Param* param) {
    	fprintf(stderr, "%d: Param(val = {%d, %d})\n", param->index, param->val.var_type, param->val.val);
    }
    virtual void visit(Call* call) {
    	fprintf(stderr, "%d: Call(name = %s, hasDst = %d, dst = {%d, %d})\n", call->index, call->name, call->hasDst, call->dst.var_type, call->dst.val);
    }
    virtual void visit(Ret* ret) {
    	fprintf(stderr, "%d: Ret(val = {%d, %d})\n", ret->index, ret->val.var_type, ret->val.val);
    }
    virtual void visit(FuncEnd* funcend) {
    	fprintf(stderr, "%d: FuncEnd(name = %s)\n", funcend->index, funcend->name);
    }
};

void print_expr();


#endif