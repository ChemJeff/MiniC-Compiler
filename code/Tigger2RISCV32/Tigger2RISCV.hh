/* Generating RISCV assembly corresponding to Tigger grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __TIGGER2RISCV_H__
#define __TIGGER2RISCV_H__


#include <assert.h>
#include <string.h>

#include "Tigger.typedef.hh"

extern Expr* exprTab[MAX_EXPRS];
extern int expr_cnt;
extern const char idx2reg[32][8];

const char idx2op[32][4] = {
    "+", "-", "*", "/", "%",
    "&&", "||", "<", ">", "==", "!=", "<=", ">=",
    "<<", ">>",
    "-", "!"
};


struct RISCVGenerator: public ExprVisitor {
    bool inFunc;
    int func_stack_size;
    const char (&_idx2op)[32][4];
    const char (&_idx2reg)[32][8];
    RISCVGenerator(): ExprVisitor(), _idx2op(idx2op), _idx2reg(idx2reg) {
        this->name = strdup("RISCVGenerator");
        this->inFunc = false;
        this->func_stack_size = -1;
    }
    ~RISCVGenerator() {delete this->name;}
    inline void indent() {printf("    "); }
    virtual void visit(Func* func) {
        int stk = (func->stack_size/4 + 1)*16;
        func_stack_size = stk;
        indent(); printf(".text\n");
        indent(); printf(".align    2\n");
        // hotfix about function with name `main`, `getint`, `getchar`, `putint`, `putchar`
        if (strcmp(func->name, "f_main") == 0 || strcmp(func->name, "f_getint") == 0
            || strcmp(func->name, "f_getchar") == 0 || strcmp(func->name, "f_putint") == 0
            || strcmp(func->name, "f_putchar") == 0) {
            func->name = func->name + 2;
        }
        indent(); printf(".global   %s\n", func->name);
        indent(); printf(".type     %s, @function\n", func->name);
        printf("%s:\n", func->name);
        // using pesudo instructions: li
        // NOTE: check whether the immeadiate is in 12-bit range
        if (stk <= 2048) {
            indent(); printf("add       sp, sp, -%d\n", stk);
        }
        else {
            indent(); printf("li        s0, -%d\n", stk);
            indent(); printf("add       sp, sp, s0\n");
        }
        if (stk < 2048 + 4) {
            indent(); printf("sw        ra, %d(sp)\n", stk-4);
        }
        else {
            indent(); printf("li        s0, %d\n", stk-4);
            indent(); printf("add       s0, s0, sp\n");
            indent(); printf("sw        ra, (s0)\n");
        }
    }
    virtual void visit(GlobalDecl* decl) {
        if (decl->isArray) {
            indent(); printf(".comm     v%d, %d, 4\n", decl->global_idx, decl->arraySize);
        }
        else {
            indent(); printf(".global   v%d\n", decl->global_idx);
            indent(); printf(".section  .sdata\n");
            indent(); printf(".align    2\n");
            indent(); printf(".type     v%d, @object\n", decl->global_idx);
            indent(); printf(".size     v%d, 4\n", decl->global_idx);
            printf("v%d:\n", decl->global_idx);
            indent(); printf(".word     %d\n", decl->arraySize);
        }
    }
    virtual void visit(UnaryOp* unaryop) {
        // using pesudo instructions: neg, seqz
        switch(unaryop->op) {
            case OPR_NEG: indent(); printf("neg       %s, %s\n", _idx2reg[unaryop->dst_reg], _idx2reg[unaryop->src_reg]); break;
            case OPR_NOT: indent(); printf("seqz      %s, %s\n", _idx2reg[unaryop->dst_reg], _idx2reg[unaryop->src_reg]); break;
        }
    }
    virtual void visit(BinaryOp* binaryop) {
        // using pesudo instructions: sgt, snez, seqz, not
        if (binaryop->isSrc2Reg) {
            switch(binaryop->op) {
                case OPR_ADD: indent(); printf("add       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                case OPR_SUB: indent(); printf("sub       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                case OPR_MUL: indent(); printf("mul       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                case OPR_DIV: indent(); printf("div       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                case OPR_MOD: indent(); printf("rem       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                case OPR_LT: indent(); printf("slt       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                case OPR_GT: indent(); printf("sgt       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]); break;
                // NOTE: tricky here for AND (C-semantic, not bitwise) operation
                case OPR_AND: {
                    indent(); printf("snez      %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg]);
                    indent(); printf("addi      %s, %s, -1\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg]);
                    indent(); printf("not       %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg]);
                    indent(); printf("and       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src2]);
                    indent(); printf("snez      %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg]);
                    break;
                }
                case OPR_OR: {
                    indent(); printf("or        %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]);
                    indent(); printf("snez      %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg]);
                    break;
                }
                case OPR_NE: {
                    indent(); printf("xor       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]);
                    indent(); printf("snez      %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg]);
                    break;
                }
                case OPR_EQ: {
                    indent(); printf("xor       %s, %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], _idx2reg[binaryop->src2]);
                    indent(); printf("seqz      %s, %s\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->dst_reg]);
                    break;
                }
            }
        }
        else {
            switch(binaryop->op) {
                case OPR_ADD: indent(); printf("addi      %s, %s, %d\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], binaryop->src2); break;
                case OPR_LT: indent(); printf("slti      %s, %s, %d\n", _idx2reg[binaryop->dst_reg], _idx2reg[binaryop->src1_reg], binaryop->src2); break;
            }
        }
    }
    virtual void visit(Assign* assign) {
        // using pesudo instructions: mv
        switch(assign->ass_type) {
            case ASS_NO_ARRAY: {
                if (assign->isSrcReg) {
                    indent(); printf("mv        %s, %s\n", _idx2reg[assign->dst_reg], _idx2reg[assign->src]);
                }
                else {
                    indent(); printf("li        %s, %d\n", _idx2reg[assign->dst_reg], assign->src);
                }
                break;
            }
            case ASS_ARRAY_LEFT: {
                // NOTE: check whether the immeadiate is in 12-bit range
                if (assign->array_ofst < 2048 && assign->array_ofst >= -2048) {
                    indent(); printf("sw        %s, %d(%s)\n", _idx2reg[assign->src], assign->array_ofst, _idx2reg[assign->dst_reg]);
                }
                else {
                    indent(); printf("li        s0, %d\n", assign->array_ofst);
                    indent(); printf("add       s0, s0, %s\n", _idx2reg[assign->dst_reg]);
                    indent(); printf("sw        %s, (%s)\n", _idx2reg[assign->src], _idx2reg[assign->dst_reg]);
                }
                break;
            }
            case ASS_ARRAY_RIGHT: {
                // NOTE: check whether the immeadiate is in 12-bit range
                if (assign->array_ofst < 2048 && assign->array_ofst >= -2048) {
                    indent(); printf("lw        %s, %d(%s)\n", _idx2reg[assign->dst_reg], assign->array_ofst, _idx2reg[assign->src]);
                }
                else {
                    indent(); printf("li        s0, %d\n", assign->array_ofst);
                    indent(); printf("add       s0, s0, %s\n", _idx2reg[assign->src]);
                    indent(); printf("lw        %s, (%s)\n", _idx2reg[assign->dst_reg], _idx2reg[assign->src]);
                }
                break;
            }
        }
    }
    virtual void visit(CondiGoto* condigoto) {
        switch(condigoto->logiop) {
            case OPR_LT: indent(); printf("blt       %s, %s, .l%d\n", _idx2reg[condigoto->opr1_reg], _idx2reg[condigoto->opr2_reg], condigoto->label); break;
            case OPR_GT: indent(); printf("bgt       %s, %s, .l%d\n", _idx2reg[condigoto->opr1_reg], _idx2reg[condigoto->opr2_reg], condigoto->label); break;
            case OPR_NE: indent(); printf("bne       %s, %s, .l%d\n", _idx2reg[condigoto->opr1_reg], _idx2reg[condigoto->opr2_reg], condigoto->label); break;
            case OPR_EQ: indent(); printf("beq       %s, %s, .l%d\n", _idx2reg[condigoto->opr1_reg], _idx2reg[condigoto->opr2_reg], condigoto->label); break;
            case OPR_LE: indent(); printf("ble       %s, %s, .l%d\n", _idx2reg[condigoto->opr1_reg], _idx2reg[condigoto->opr2_reg], condigoto->label); break;
            case OPR_GE: indent(); printf("ble       %s, %s, .l%d\n", _idx2reg[condigoto->opr2_reg], _idx2reg[condigoto->opr1_reg], condigoto->label); break;
            case OPR_AND: // NOTE: unable to convert, this type should be eliminated in Tigger code
            case OPR_OR: // NOTE: unable to convert, this type should be eliminated in Tigger code
            default: break;
        }
    }
    virtual void visit(Goto* got) {
        indent(); printf("j         .l%d\n", got->label);
    }
    virtual void visit(Label* label) {
        printf(".l%d:\n", label->label_idx);
    }
    virtual void visit(Call* call) {
        // using pesudo instructions: call
        // hotfix about function with name `main`, `getint`, `getchar`, `putint`, `putchar`
        if (strcmp(call->name, "f_main") == 0 || strcmp(call->name, "f_getint") == 0
            || strcmp(call->name, "f_getchar") == 0 || strcmp(call->name, "f_putint") == 0
            || strcmp(call->name, "f_putchar") == 0) {
            call->name = call->name + 2;
        }
        indent(); printf("call      %s\n", call->name);
    }
    virtual void visit(Store* store) {
        assert(func_stack_size > 0);
        // NOTE: check whether the immeadiate is in 12-bit range
        if (store->dst_imm*4 < 2048) {
            indent(); printf("sw        %s, %d(sp)\n", _idx2reg[store->src_reg], store->dst_imm*4);
        }
        else {
            indent(); printf("li        s0, %d\n", store->dst_imm*4);
            indent(); printf("add       s0, s0, sp\n");
            indent(); printf("sw        %s, (s0)\n", _idx2reg[store->src_reg]);
        }
    }
    virtual void visit(Load* load) {
        if (load->isSrcGlobal) {
            indent(); printf("lui       %s, %%hi(v%d)\n", _idx2reg[load->dst_reg], load->src);
            indent(); printf("lw        %s, %%lo(v%d)(%s)\n", _idx2reg[load->dst_reg], load->src, _idx2reg[load->dst_reg]);
        }
        else {
            assert(func_stack_size > 0);
            // NOTE: check whether the immeadiate is in 12-bit range
            if (load->src*4 < 2048) {
                indent(); printf("lw        %s, %d(sp)\n", _idx2reg[load->dst_reg], load->src*4);
            }
            else {
                indent(); printf("li        s0, %d\n", load->src*4);
                indent(); printf("add       s0, s0, sp\n");
                indent(); printf("lw        %s, (s0)\n", _idx2reg[load->dst_reg]);
            }
        }
    }
    virtual void visit(LoadAddr* loadaddr) {
        if (loadaddr->isSrcGlobal) {
            indent(); printf("lui       %s, %%hi(v%d)\n", _idx2reg[loadaddr->dst_reg], loadaddr->src);
            indent(); printf("add       %s, %s, %%lo(v%d)\n", _idx2reg[loadaddr->dst_reg], _idx2reg[loadaddr->dst_reg], loadaddr->src);
        }
        else {
            assert(func_stack_size > 0);
            // NOTE: check whether the immeadiate is in 12-bit range
            if (loadaddr->src*4 < 2048) {
                indent(); printf("add       %s, sp, %d\n", _idx2reg[loadaddr->dst_reg], loadaddr->src*4);
            }
            else {
                indent(); printf("li        s0, %d\n", loadaddr->src*4);
                indent(); printf("add       %s, sp, s0\n", _idx2reg[loadaddr->dst_reg]);
            }
        }
    }
    virtual void visit(Ret* ret) {
        assert(func_stack_size > 0);
        // NOTE: check whether the immeadiate is in 12-bit range
        if (func_stack_size - 4 < 2048) {
            indent(); printf("lw        ra, %d(sp)\n", func_stack_size - 4);
        }
        else {
            indent(); printf("li        s0, %d\n", func_stack_size - 4);
            indent(); printf("add       s0, s0, sp\n");
            indent(); printf("lw        ra, (s0)\n");
        }
        if (func_stack_size < 2048) {
            indent(); printf("add       sp, sp, %d\n", func_stack_size);
        }
        else {
            indent(); printf("li        s0, %d\n", func_stack_size);
            indent(); printf("add       sp, sp, s0\n");
        }
        indent(); printf("jr        ra\n");
        // func_stack_size = -1;
    }
    virtual void visit(FuncEnd* funcend) {
        // hotfix about function with name `main`, `getint`, `getchar`, `putint`, `putchar`
        if (strcmp(funcend->name, "f_main") == 0 || strcmp(funcend->name, "f_getint") == 0
            || strcmp(funcend->name, "f_getchar") == 0 || strcmp(funcend->name, "f_putint") == 0
            || strcmp(funcend->name, "f_putchar") == 0) {
            funcend->name = funcend->name + 2;
        }
        indent(); printf(".size     %s, .-%s\n", funcend->name, funcend->name);
        func_stack_size = -1;
    }
};


void generate_riscv(RISCVGenerator& riscvGenerator) {
    riscvGenerator.inFunc = false;
    riscvGenerator.func_stack_size = -1;
    for (int i=0; i<expr_cnt; i++) {
        exprTab[i]->accept(riscvGenerator);
    }
    printf("    .ident  \"MiniC Compiler by Jiajun Tang, 1500011776, CS, EECS, PKU\"\n");
}


#endif