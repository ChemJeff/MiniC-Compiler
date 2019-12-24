/* Liveness analysis corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __EEYORE_LIVENESS_H__
#define __EEYORE_LIVENESS_H__

#ifdef DEBUGLIVENESS
#define dprintf_liveness(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_liveness(fmt, ...)
#endif

#include <assert.h>
#include <bitset>
#include <vector>

#include "Eeyore.typedef.hh"

using namespace std;

// NOTE: liveness at a point x here is the liveness exactly before executing instruction x
// division of basic blocks also here
struct LivenessAnalyzer: public ExprVisitor {
	bool fixed;
    int expr_cnt;
	int func_cnt_backwards;
    Expr* (&exprTab)[MAX_EXPRS];
	bitset<MAX_VARS> (&livenessTab)[MAX_EXPRS + 1];
	bitset<MAX_VARS> livenessTemp;
	int (&var2idx)[3][MAX_VARS];
	int (&label2idx)[MAX_EXPRS];
    LivenessAnalyzer(
        Expr* (&_exprTab)[MAX_EXPRS],
    	bitset<MAX_VARS> (&_livenessTab)[MAX_EXPRS + 1],
    	int (&_var2idx)[3][MAX_VARS],
    	int (&_label2idx)[MAX_EXPRS]
    	): ExprVisitor(), exprTab(_exprTab), livenessTab(_livenessTab),
    	 var2idx(_var2idx), label2idx(_label2idx) {
    	this->name = strdup("LivenessAnalyzer");
    	this->fixed = true;
        this->expr_cnt = MAX_EXPRS;
    	this->func_cnt_backwards = 0x7fffffff;
    	this->livenessTemp.reset();
    }
    ~LivenessAnalyzer() {delete this->name;}
    inline int get_var_idx(Rval _val) {
    	int var_type = _val.var_type;
    	int val = _val.val;
    	if (var_type > VAR_IMMEDIATE && var_type < VAR_PARAM) {
    		return var2idx[var_type - VAR_NATIVE][val];
    	}
    	else if (var_type == VAR_PARAM) {
    		return var2idx[var_type - VAR_NATIVE][val + func_cnt_backwards*8];
    	}
    	else {
    		return -1;
    	}
    }
    inline void check_and_set(bitset<MAX_VARS> &dst, bitset<MAX_VARS> src) {
    	if (dst != src) {fixed = false;}
    	dst = src;
    }
    virtual void visit(Func* func) {
    	livenessTab[func->index] = livenessTab[func->index + 1];
        func->isBlockEntry = true;      // ordinary entries of basic blocks
    }
    virtual void visit(Decl* decl) {
    	int var_idx = get_var_idx(decl->val);
    	livenessTemp.set();
    	assert(var_idx >= 0);
    	livenessTemp.reset(var_idx);
    	check_and_set(livenessTab[decl->index],
    	 livenessTab[decl->index + 1] & livenessTemp);
    }
    virtual void visit(UnaryOp* unaryop) {
    	livenessTemp = livenessTab[unaryop->index + 1];
    	int var_idx = get_var_idx(unaryop->dst);
    	assert(var_idx >= 0);
    	livenessTemp.reset(var_idx);
    	var_idx = get_var_idx(unaryop->opr);
    	if (var_idx >= 0) {
    		livenessTemp.set(var_idx);
    	}
    	check_and_set(livenessTab[unaryop->index], livenessTemp);
    }
    virtual void visit(BinaryOp* binaryop) {
    	livenessTemp = livenessTab[binaryop->index + 1];
    	int var_idx = get_var_idx(binaryop->dst);
    	assert(var_idx >= 0);
    	livenessTemp.reset(var_idx);
    	var_idx = get_var_idx(binaryop->opr1);
    	if (var_idx >= 0) {
    		livenessTemp.set(var_idx);
    	}
    	var_idx = get_var_idx(binaryop->opr2);
    	if (var_idx >= 0) {
    		livenessTemp.set(var_idx);
    	}
    	check_and_set(livenessTab[binaryop->index], livenessTemp);
    }
    virtual void visit(Assign* assign) {
    	livenessTemp = livenessTab[assign->index + 1];
    	int var_idx = get_var_idx(assign->dst);
    	assert(var_idx >= 0);
    	livenessTemp.reset(var_idx);
    	switch(assign->ass_type) {
    		case ASS_NO_ARRAY: {
    			var_idx = get_var_idx(assign->src);
		    	if (var_idx >= 0) {
		    		livenessTemp.set(var_idx);
		    	}
    			break;
    		}
    		case ASS_ARRAY_LEFT: {
    			var_idx = get_var_idx(assign->ofst_dst);
    			if (var_idx >= 0) {
    				livenessTemp.set(var_idx);
    			}
    			var_idx = get_var_idx(assign->src);
    			if (var_idx >= 0) {
    				livenessTemp.set(var_idx);
    			}
                var_idx = get_var_idx(assign->dst);
                if (var_idx >= 0) {
                    livenessTemp.set(var_idx);
                }
    			break;
    		}
    		case ASS_ARRAY_RIGHT: {
    			var_idx = get_var_idx(assign->ofst_src);
    			if (var_idx >= 0) {
    				livenessTemp.set(var_idx);
    			}
    			var_idx = get_var_idx(assign->src);
    			if (var_idx >= 0) {
    				livenessTemp.set(var_idx);
    			}
    			break;
    		}
    		default:
    			break;
    	}
    	
    	check_and_set(livenessTab[assign->index], livenessTemp);
    }
    virtual void visit(CondiGoto* condigoto) {
    	int goto_dst = label2idx[condigoto->label];
    	assert(goto_dst >= 0);
    	livenessTemp.reset();
    	int var_idx = get_var_idx(condigoto->opr1);
    	if (var_idx >= 0) livenessTemp.set(var_idx);
    	var_idx = get_var_idx(condigoto->opr2);
    	if (var_idx >= 0) livenessTemp.set(var_idx);
    	check_and_set(livenessTab[condigoto->index],
    	 livenessTab[condigoto->index + 1] | livenessTab[goto_dst] | livenessTemp);
        // basic block division
        exprTab[goto_dst]->isBlockEntry = true;
        if (condigoto->index + 1 < expr_cnt)
            exprTab[condigoto->index + 1]->isBlockEntry = true;
    }
    virtual void visit(Goto* got) {
    	int goto_dst = label2idx[got->label];
    	// destination of goto must exist and be valid
    	assert(goto_dst >= 0);
    	livenessTab[got->index] = livenessTab[goto_dst];
        // basic block division
        exprTab[goto_dst]->isBlockEntry = true;
        if (got->index + 1 < expr_cnt)
            exprTab[got->index + 1]->isBlockEntry = true;
    }
    virtual void visit(Label* label) {
    	livenessTab[label->index] = livenessTab[label->index + 1];
    }
    virtual void visit(Param* param) {
    	// assume all parameters are passed via registers
    	int var_idx = get_var_idx(param->val);
    	livenessTemp.reset();
    	if (var_idx >= 0) {
    		livenessTemp.set(var_idx);
    	}
    	check_and_set(livenessTab[param->index],
    	 livenessTab[param->index + 1] | livenessTemp);
    }
    virtual void visit(Call* call) {
    	livenessTab[call->index] = livenessTab[call->index + 1];
        // // basic blocks division
        // if (call->index + 1 < expr_cnt)
        //     exprTab[call->index + 1]->isBlockEntry = true;
    }
    virtual void visit(Ret* ret) {
    	int var_idx = get_var_idx(ret->val);
    	livenessTemp.reset();
    	if (var_idx >= 0) {
    		livenessTemp.set(var_idx);
    	}
    	check_and_set(livenessTab[ret->index],
    	 livenessTab[ret->index + 1] | livenessTemp);
        // basic blocks division
        if (ret->index + 1 < expr_cnt)
            exprTab[ret->index + 1]->isBlockEntry = true;
    }
    virtual void visit(FuncEnd* funcend) {
    	func_cnt_backwards--;
    	livenessTab[funcend->index] = livenessTab[funcend->index + 1];
        // funcend->isBlockExit = true; // ordinary exits of basic blocks
    }
};

void calc_var2idx();
void calc_label2idx();
void init_liveness();
void iter_liveness();
void calc_liveness_interval();
void calc_blocks();
void print_liveness_line();
void print_liveness();
void print_blocks();


#endif