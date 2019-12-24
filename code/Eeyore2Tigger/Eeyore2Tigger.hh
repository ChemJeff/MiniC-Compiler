/* Generating Tigger code corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __EEYORE2TIGGER_H__
#define __EEYORE2TIGGER_H__


#include <assert.h>

#include "Eeyore.typedef.hh"
#include "Eeyore.liveness.hh"
#include "Eeyore.regalloc.hh"

extern Expr* exprTab[MAX_EXPRS];
extern Var* varTab[MAX_VARS];
extern Istr* istrTab[MAX_ISTRS];
extern Reg registers[32];
extern int var2idx[3][MAX_VARS];
extern int expr_cnt;
extern int var_cnt;
extern int istr_cnt;
extern int func_stack_size;
extern const char idx2reg[32][8];

const char idx2op[32][4] = {
	"+", "-", "*", "/", "%",
	"&&", "||", "<", ">", "==", "!=", "<=", ">=",
	"<<", ">>",
	"-", "!"
};


struct TiggerGenerator: public ExprVisitor {
	bool inFunc;
	int global_cnt;
	int func_cnt;
	int func2istr_idx;
	int func_param_cnt;
	int &_func_stack_size;
	int (&_var2idx)[3][MAX_VARS];
	TiggerGenerator(): ExprVisitor(), _var2idx(var2idx), _func_stack_size(func_stack_size) {
		this->name = strdup("TiggerGenerator");
		this->inFunc = false;
		this->global_cnt = 0;
		this->func_cnt = 0;
		this->func2istr_idx = -1;
		this->func_param_cnt = 0;
	}
	~TiggerGenerator() {delete this->name;}
	inline int get_var_idx(Rval _val) {
    	int var_type = _val.var_type;
    	int val = _val.val;
    	if (var_type > VAR_IMMEDIATE && var_type < VAR_PARAM) {
    		return _var2idx[var_type - VAR_NATIVE][val];
    	}
    	else if (var_type == VAR_PARAM) {
    		return _var2idx[var_type - VAR_NATIVE][val + func_cnt*8];
    	}
    	else {
    		return -1;
    	}
    }
	inline void _enter_block(int expr_idx) {
		enter_block(expr_idx);
	}
    inline void _exit_block(int expr_idx, bool save_local=true) {
        exit_block(expr_idx, save_local);
    }
	inline int _alloc_register(Rval rval, int expr_idx) {
		if (rval.var_type == VAR_PARAM) {
			rval.val += this->func_cnt*8;
		}
		return alloc_register(rval, expr_idx);
	}
	inline int _alloc_temp_register(int expr_idx) {
		return alloc_temp_register(expr_idx);
	}
	inline void _bind_register(int reg_idx, Rval rval, int expr_idx
		, bool var_to_reg=true, bool save_old=true, bool load_new=true, bool copy_dup=false) {
		if (rval.var_type == VAR_PARAM) {
			rval.val += this->func_cnt*8;
		}
		bind_register(reg_idx, rval, expr_idx, var_to_reg, save_old, load_new, copy_dup);
	}
	inline int _eval_op1(int op, int imm) {
		switch(op) {
			case OPR_NEG: return -imm; break;
			case OPR_NOT: return !imm; break;
			default: // not in standard Eeyore grammar
				return 0x80000000;
		}
	}
	inline int _eval_op2(int imm1, int op, int imm2) {
		switch(op) {
			case OPR_ADD: return imm1 + imm2; break;
			case OPR_SUB: return imm1 - imm2; break;
			case OPR_MUL: return imm1 * imm2; break;
			case OPR_DIV: return imm1 / imm2; break;
			case OPR_MOD: return imm1 % imm2; break;
			case OPR_NE: return imm1 != imm2; break;
			case OPR_EQ: return imm1 == imm2; break;
			case OPR_AND: return imm1 && imm2; break;
			case OPR_OR: return imm1 || imm2; break;
			case OPR_GT: return imm1 > imm2; break;
			case OPR_LT: return imm1 < imm2; break;
			default: // not in standard Eeyore grammar
				return 0x80000000;
		}
	}
    virtual void visit(Func* func) {
    	this->inFunc = true;
    	this->func2istr_idx = istr_cnt;
    	this->func_param_cnt = 0;
    	this->_func_stack_size = 0;
    	istrTab[istr_cnt++] = new Istr_Func(func->name, func->num_param);
        if (func->isBlockEntry) {
            _enter_block(func->index);
        }
    	for (int i=0; i<func->num_param; i++) {
    		// associate values passed by ax with param px
            // NOTE: also allocate stack space for those parameters
    		_bind_register(REG_A + i, Rval{i, VAR_PARAM}, func->index
    			, false, false, false);
            Var* varEntry = varTab[get_var_idx(Rval{i, VAR_PARAM})];
            varEntry->allocedStk = _func_stack_size++;
    	}
        if (func->isBlockExit) {
            _exit_block(func->index);
        }
    }
    virtual void visit(Decl* decl) {
        if (decl->isBlockEntry) {
            _enter_block(decl->index);
        }
    	if (decl->isGlobal) {
    		if (decl->isArray) {
    			istrTab[istr_cnt++] = new Istr_Decl(global_cnt, true, decl->arraySize);
    		}
    		else {
    			// all global variables are initialized as 0
    			istrTab[istr_cnt++] = new Istr_Decl(global_cnt, false, 0);
    		}
    		// set the var to a global var
    		Var* varEntry = varTab[get_var_idx(decl->val)];
    		varEntry->isGlobal = true;
    		global_cnt++;
    	}
    	else {
            // NOTE: plain allocation for stack space
            // alloc stack space for local variable ONLY at first time
            // no matter whether needed in the future
            Var* varEntry = varTab[get_var_idx(decl->val)];
            varEntry->isGlobal = false;
            if (decl->isArray) {
                varEntry->allocedStk = _func_stack_size;
                _func_stack_size += varEntry->arraySize / 4;
            }
            else {
                varEntry->allocedStk = _func_stack_size++;
                varEntry->allocedStatus = ALLOC_STK;
            }
    	}
        if (decl->isBlockExit) {
            _exit_block(decl->index);
        }
    }
    virtual void visit(UnaryOp* unaryop) {
        if (unaryop->isBlockEntry) {
            _enter_block(unaryop->index);
        }
    	int dst_reg = _alloc_register(unaryop->dst, unaryop->index);
    	if (unaryop->opr.var_type == VAR_IMMEDIATE) {
    		int src_imm = _eval_op1(unaryop->op, unaryop->opr.val);
    		istrTab[istr_cnt++] = new Istr_Assign(dst_reg, false, src_imm, 0, ASS_NO_ARRAY);
    	}
    	else {
    		int src_reg = _alloc_register(unaryop->opr, unaryop->index);
			istrTab[istr_cnt++] = new Istr_Op1(dst_reg, unaryop->op, src_reg);
    	}
        registers[dst_reg].dirty = true;
        if (unaryop->isBlockExit) {
            _exit_block(unaryop->index);
        }
    }
    virtual void visit(BinaryOp* binaryop) {
        if (binaryop->isBlockEntry) {
            _enter_block(binaryop->index);
        }
    	int dst_reg = _alloc_register(binaryop->dst, binaryop->index);
    	if ((binaryop->opr1.var_type != VAR_IMMEDIATE &&
    	 binaryop->opr2.var_type != VAR_IMMEDIATE) || 
            (binaryop->opr1.var_type == VAR_IMMEDIATE && (binaryop->opr1.val >= 2048 || binaryop->opr1.val < -2048)) || 
            (binaryop->opr2.var_type == VAR_IMMEDIATE && (binaryop->opr2.val >= 2048 || binaryop->opr2.val < -2048))) {
    		// var OP2 var
            // NOTE: immeadiate out of 12-bit range also translate in this way
    		int opr1_reg = _alloc_register(binaryop->opr1, binaryop->index);
    		int opr2_reg = _alloc_register(binaryop->opr2, binaryop->index);
    		istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr1_reg, binaryop->op, true, opr2_reg);
    	}
    	else if (binaryop->opr1.var_type != VAR_IMMEDIATE &&
    	 binaryop->opr2.var_type == VAR_IMMEDIATE) {
    		// var OP2 imm MAY TURN TO var OP2 var
    		int opr1_reg = _alloc_register(binaryop->opr1, binaryop->index);
    		if (binaryop->op == OPR_ADD || binaryop->op == OPR_LT) {
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr1_reg, binaryop->op, false, binaryop->opr2.val);
    		}
    		else if (binaryop->op == OPR_SUB) {
    			// convert to addition
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr1_reg, OPR_ADD, false, -binaryop->opr2.val);
    		}
    		else {
    			// load imm into reg
    			int opr2_reg = _alloc_register(binaryop->opr2, binaryop->index);
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr1_reg, binaryop->op, true, opr2_reg);
    		}
    	}
    	else if (binaryop->opr1.var_type == VAR_IMMEDIATE &&
    	 binaryop->opr2.var_type != VAR_IMMEDIATE) {
    		// imm OP2 var TURN TO var OP2 imm (if possible) or var OP2 var
    		int opr2_reg = _alloc_register(binaryop->opr2, binaryop->index);
    		if (binaryop->op == OPR_ADD) {
    			// commutativity of addition
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr2_reg, OPR_ADD, false, binaryop->opr1.val);
    		}
    		else if (binaryop->op == OPR_GT) {
    			// convert to LT
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr2_reg, OPR_LT, false, binaryop->opr1.val);
    		}
    		else if (binaryop->op == OPR_SUB) {
    			// dst_reg = -opr2_reg first
    			// then convert to addition
    			istrTab[istr_cnt++] = new Istr_Op1(dst_reg, OPR_NEG, opr2_reg);
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, dst_reg, OPR_ADD, false, binaryop->opr1.val);
    		}
    		else {
    			// load imm into reg
    			int opr1_reg = _alloc_register(binaryop->opr1, binaryop->index);
    			istrTab[istr_cnt++] = new Istr_Op2(dst_reg, opr1_reg, binaryop->op, true, opr2_reg);
    		}
    	}
    	else if (binaryop->opr1.var_type == VAR_IMMEDIATE &&
    	 binaryop->opr2.var_type == VAR_IMMEDIATE) {
    	 	// imm OP2 imm REDUCE TO imm (assign)
    		int src_imm = _eval_op2(binaryop->opr1.val, binaryop->op, binaryop->opr2.val);
    		istrTab[istr_cnt++] = new Istr_Assign(dst_reg, false, src_imm, 0, ASS_NO_ARRAY);
    	}
    	registers[dst_reg].dirty = true;
        if (binaryop->isBlockExit) {
            _exit_block(binaryop->index);
        }
    }
    virtual void visit(Assign* assign) {
        if (assign->isBlockEntry) {
            _enter_block(assign->index);
        }
    	switch(assign->ass_type) {
    		case ASS_NO_ARRAY: {
    			int dst_reg = _alloc_register(assign->dst, assign->index);
    			if (assign->src.var_type == VAR_IMMEDIATE) {
    				// var = imm
    				istrTab[istr_cnt++] = new Istr_Assign(dst_reg,
    				 false, assign->src.val, 0, ASS_NO_ARRAY);
    			}
    			else {
    				// var1 = var2
    				int src_reg = _alloc_register(assign->src, assign->index);
    				istrTab[istr_cnt++] = new Istr_Assign(dst_reg,
    				 true, src_reg, 0, ASS_NO_ARRAY);
    			}
                registers[dst_reg].dirty = true;
    			break;
    		}
    		case ASS_ARRAY_LEFT: {
    			int dst_reg = _alloc_register(assign->dst, assign->index);
    			if (assign->ofst_dst.var_type == VAR_IMMEDIATE) {
    				// var1 [imm] = var2/imm
    				int src_reg = _alloc_register(assign->src, assign->index);
    				istrTab[istr_cnt++] = new Istr_Assign(dst_reg, 
    				 true, src_reg, assign->ofst_dst.val, ASS_ARRAY_LEFT);
    			}
    			else {
    				// var1 [var2] = var3/imm
    				// translate into
    				// assign var1 into reg1
    				// reg1 = reg1 + reg2
    				// reg1 [0] = reg3
    				int idx_reg = _alloc_register(assign->ofst_dst, assign->index);
    				int src_reg = _alloc_register(assign->src, assign->index);
    				int addr_reg = _alloc_temp_register(assign->index);
    				istrTab[istr_cnt++] = new Istr_Op2(addr_reg, dst_reg, OPR_ADD, true, idx_reg);
    				istrTab[istr_cnt++] = new Istr_Assign(addr_reg, true, src_reg, 0, ASS_ARRAY_LEFT);
    			}
    			break;
    		}
    		case ASS_ARRAY_RIGHT: {
    			int dst_reg = _alloc_register(assign->dst, assign->index);
    			if (assign->ofst_src.var_type == VAR_IMMEDIATE) {
    				// var1 = var2 [imm]
    				int src_reg = _alloc_register(assign->src, assign->index);
    				istrTab[istr_cnt++] = new Istr_Assign(dst_reg,
    				 true, src_reg, assign->ofst_src.val, ASS_ARRAY_RIGHT);
    			}
    			else {
    				// var1 = var2 [var3]
    				// translate into 
    				// assign var2 into reg2
    				// reg2 = reg2 + reg3
    				// reg1 = reg2 [0]
    				int src_reg = _alloc_register(assign->src, assign->index);
    				int idx_reg = _alloc_register(assign->ofst_src, assign->index);
    				int addr_reg = _alloc_temp_register(assign->index);
    				istrTab[istr_cnt++] = new Istr_Op2(addr_reg, src_reg, OPR_ADD, true, idx_reg);
    				istrTab[istr_cnt++] = new Istr_Assign(dst_reg, true, addr_reg, 0, ASS_ARRAY_RIGHT);
    			}
                registers[dst_reg].dirty = true;
    			break;
    		}
    		default: // invalid assign stmt
    			break;
    	}
        if (assign->isBlockExit) {
            _exit_block(assign->index);
        }
    }
    virtual void visit(CondiGoto* condigoto) {
        if (condigoto->isBlockEntry) {
            _enter_block(condigoto->index);
        }
        if (condigoto->isBlockExit) {
            _exit_block(condigoto->index);
        }
    	int opr1_reg = _alloc_register(condigoto->opr1, condigoto->index);
    	int opr2_reg = _alloc_register(condigoto->opr2, condigoto->index);
    	istrTab[istr_cnt++] = new Istr_CondiGoto(opr1_reg,
    	 condigoto->logiop, opr2_reg, condigoto->label);
    }
    virtual void visit(Goto* got) {
        if (got->isBlockEntry) {
            _enter_block(got->index);
        }
        if (got->isBlockExit) {
            _exit_block(got->index);
        }
    	istrTab[istr_cnt++] = new Istr_Goto(got->label);
    }
    virtual void visit(Label* label) {
        if (label->isBlockExit) {
            _exit_block(label->index);
        }
    	istrTab[istr_cnt++] = new Istr_Label(label->val);
        if (label->isBlockEntry) {
        	_enter_block(label->index);
        }
    }
    virtual void visit(Param* param) {
    	// assign var to ax
    	_bind_register(REG_A + this->func_param_cnt++, param->val, param->index, true, true, true, true);
    }
    virtual void visit(Call* call) {
    	// save caller-saved registers 
    	// ...
        exit_block(call->index, true, false);
    	istrTab[istr_cnt++] = new Istr_Call(call->name);
    	if (call->hasDst) {
    		// need to perserve a0
    		_bind_register(REG_A, call->dst, call->index, false, false, false);
    	}
    	// pop caller-saved registers (not explicitly)
    	// ...
    	this->func_param_cnt = 0;
    }
    virtual void visit(Ret* ret) {
    	// assign var to a0
        if (ret->isBlockEntry) {
            _enter_block(ret->index);
        }
        _bind_register(REG_A, ret->val, ret->index, true, true, true, true);
    	if (ret->isBlockExit) {
            _exit_block(ret->index);
        }
    	istrTab[istr_cnt++] = new Istr_Ret();
    }
    virtual void visit(FuncEnd* funcend) {
        if (funcend->isBlockEntry) {
            _enter_block(funcend->index);
        }
        if (funcend->isBlockExit) {
            _exit_block(funcend->index);
        }
    	istrTab[istr_cnt++] = new Istr_FuncEnd(funcend->name);
    	// update stack_size for this function
    	assert(this->func2istr_idx >= 0);
    	((Istr_Func*)(istrTab[this->func2istr_idx]))->stack_size = this->_func_stack_size;
    	this->inFunc = false;
    	this->func2istr_idx = -1;
    	this->func_cnt++;
    }
};


struct IstrEmitter: public IstrVisitor {
	bool inFunc;
	const char (&_idx2op)[32][4];
	const char (&_idx2reg)[32][8];
	IstrEmitter(): IstrVisitor(), _idx2op(idx2op), _idx2reg(idx2reg) {
		this->inFunc = false;
		this->name = strdup("IstrEmitter");
	}
	~IstrEmitter() {delete this->name;}
	inline void indent() {if (inFunc) printf("    "); }
	inline const char* __idx2op(int op) {
		switch(op) {
			case OPR_ADD: return idx2op[0]; break;
			case OPR_SUB: return idx2op[1]; break;
			case OPR_MUL: return idx2op[2]; break;
			case OPR_DIV: return idx2op[3]; break;
			case OPR_MOD: return idx2op[4]; break;
			case OPR_AND: return idx2op[5]; break;
			case OPR_OR: return idx2op[6]; break;
			case OPR_LT: return idx2op[7]; break;
			case OPR_GT: return idx2op[8]; break;
			case OPR_EQ: return idx2op[9]; break;
			case OPR_NE: return idx2op[10]; break;
			case OPR_LE: return idx2op[11]; break;
			case OPR_GE: return idx2op[12]; break;
			case OPR_LSH: return idx2op[13]; break;
			case OPR_RSH: return idx2op[14]; break;
			case OPR_NEG: return idx2op[15]; break;
			case OPR_NOT: return idx2op[16]; break;
			default: // invalid operator 
				return NULL;
		}
	}
    virtual void visit(Istr_Func* istrfunc) {
    	indent(); printf("%s [%d] [%d]\n",
    	 istrfunc->name, istrfunc->num_param, istrfunc->stack_size);
    	this->inFunc = true;
    }
    virtual void visit(Istr_Decl* istrdecl) {
    	if (istrdecl->isArray) {
    		indent(); printf("v%d = malloc %d\n",
    		 istrdecl->global_idx, istrdecl->val);
    	}
    	else {
    		indent(); printf("v%d = %d\n",
    		 istrdecl->global_idx, istrdecl->val);
    	}
    }
    virtual void visit(Istr_Op1* istrop1) {
    	indent(); printf("%s = %s %s\n",
    	 _idx2reg[istrop1->dst_reg], __idx2op(istrop1->op), _idx2reg[istrop1->src_reg]);
    }
    virtual void visit(Istr_Op2* istrop2) {
    	if (istrop2->isSrc2Reg) {
    		indent(); printf("%s = %s %s %s\n",
    		 _idx2reg[istrop2->dst_reg], _idx2reg[istrop2->src1_reg],
    		 __idx2op(istrop2->op), _idx2reg[istrop2->src2]);
    	}
    	else {
    		assert(istrop2->op == OPR_ADD || istrop2->op == OPR_LT);
    		indent(); printf("%s = %s %s %d\n",
    		 _idx2reg[istrop2->dst_reg], _idx2reg[istrop2->src1_reg],
    		 __idx2op(istrop2->op), istrop2->src2);
    	}
    }
    virtual void visit(Istr_Assign* istrassign) {
    	switch(istrassign->ass_type) {
    		case ASS_NO_ARRAY: {
    			if (istrassign->isSrcReg) {
    				indent(); printf("%s = %s\n",
    				 _idx2reg[istrassign->dst_reg], _idx2reg[istrassign->src]);
    			}
    			else {
    				indent(); printf("%s = %d\n",
    				 _idx2reg[istrassign->dst_reg], istrassign->src);
    			}
    			break;
    		}
    		case ASS_ARRAY_LEFT: {
				indent(); printf("%s [%d] = %s\n",
				_idx2reg[istrassign->dst_reg], istrassign->array_ofst,
				_idx2reg[istrassign->src]);
    			break;
    		}
    		case ASS_ARRAY_RIGHT: {
    			indent(); printf("%s = %s [%d]\n",
    			_idx2reg[istrassign->dst_reg], _idx2reg[istrassign->src],
    			istrassign->array_ofst);
    			break;
    		}
    		default: {
    			// invalid assign istr
    		}
    	}
    }
    virtual void visit(Istr_CondiGoto* condigoto) {
    	indent(); printf("if %s %s %s goto l%d\n",
    	 _idx2reg[condigoto->opr1_reg], __idx2op(condigoto->logiop),
    	  _idx2reg[condigoto->opr2_reg], condigoto->label);
    }
    virtual void visit(Istr_Goto* istrgoto) {
    	indent(); printf("goto l%d\n", istrgoto->label);
    }
    virtual void visit(Istr_Label* istrlabel) {
    	printf("l%d:\n", istrlabel->label_idx);
    }
    virtual void visit(Istr_Call* istrcall) {
    	indent(); printf("call %s\n", istrcall->name);
    }
    virtual void visit(Istr_Store* istrstore) {
    	indent(); printf("store %s %d\n",
    	 _idx2reg[istrstore->src_reg], istrstore->dst_imm);
    }
    virtual void visit(Istr_Load* istrload) {
    	if (istrload->isSrcGlobal) {
    		indent(); printf("load v%d %s\n",
    		 istrload->src, _idx2reg[istrload->dst_reg]);
    	}
    	else {
    		indent(); printf("load %d %s\n",
    		 istrload->src, _idx2reg[istrload->dst_reg]);
    	}
    }
    virtual void visit(Istr_LoadAddr* istrloadaddr) {
    	if (istrloadaddr->isSrcGlobal) {
    		indent(); printf("loadaddr v%d %s\n",
    		 istrloadaddr->src, _idx2reg[istrloadaddr->dst_reg]);
    	}
    	else {
    		indent(); printf("loadaddr %d %s\n",
    		 istrloadaddr->src, _idx2reg[istrloadaddr->dst_reg]);
    	}
    }
    virtual void visit(Istr_Ret* istrret) {
    	indent(); printf("return\n");
    }
    virtual void visit(Istr_FuncEnd* istrfuncend) {
    	this->inFunc = false;
    	indent(); printf("end %s\n", istrfuncend->name);
    }	
};

void generate_tigger(TiggerGenerator& tiggerGenerator) {
	tiggerGenerator.inFunc = false;
	tiggerGenerator.func_cnt = 0;
	tiggerGenerator.func2istr_idx = -1;
	tiggerGenerator._func_stack_size = 0;
	init_registers();
	for (int i=0; i<expr_cnt; i++) {
		exprTab[i]->accept(tiggerGenerator);
	}
}

void emit_tigger(IstrEmitter& istrEmitter) {
	for (int i=0; i<istr_cnt; i++) {
		istrTab[i]->accept(istrEmitter);
	}
}


#endif