/* Generating Eeyore code corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __MINIC2EEYORE_H__
#define __MINIC2EEYORE_H__


#ifndef DEUBGGENE
#define dprintf_gene(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_gene(fmt, ...)
#endif

#define LIST_NONE 0
#define LIST_DECL 1 // ParamList in FuncDecl
#define LIST_DEFN 2 // ParamList in FuncDefn
#define LIST_ARGS 3

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stack>
#include "MiniC.AST.hh"
#include "MiniC.symtab.hh"

struct EeyoreGenerator;

using namespace std;

extern void yyerror(const char *, int, int);

struct EeyoreGenerator: public ASTVisitor {
	int nativecnt = 0;
	int tempcnt = 0;
    // nativecnt and tempcnt count from 0, globally
	// paramcnt stored in symtab, recount from 0 in each function
	int labelcnt = 0;
	char symbuf[128];
    // EVAL EXPR: use a stack to store current name of exprssion rval
	stack<char *> symstack;
	// TYPE CHECK: type of node gradually filled during traversal
	// types: NONE, INT, INT_ARRAY, FUNC_INT, FUNC_INT_ARRAY
	SymTab symtab;
	int list_stat = LIST_NONE;
    int indentDepth = 0;
    EeyoreGenerator(): ASTVisitor() {this->name = strdup("EeyoreGenerator");}
    ~EeyoreGenerator() {delete this->name;}

	inline void pushSymStack(char* symbol) {
		symstack.push(strdup(symbol));
	}

	inline char* popSymStack() {
		char* rtVal;
		rtVal = symstack.top();
		symstack.pop();
		return rtVal;
	}

    inline void setIndent() {
        indentDepth++;
    }

    inline void unsetIndent() {
        indentDepth--;
    }

    inline void indent() {
        for (int i = 0; i < indentDepth; i++)
            printf("    ");
    }

    virtual void visit(Goal* node) {
        for (ASTNode* i = node->first_child; i != NULL; i = i->next) {
        	i->accept(*this);
        }
    }
    virtual void visit(VarDefn* node) {
    	Identifier* id = (Identifier*)node->children[0];
    	// array size check also here TO BE ADD
    	SymTabEntry* rtVal;
    	if (node->isArray) {
    		rtVal = symtab.regisiter(id->name, TYPE_INT_ARRAY, VAR_NATIVE, nativecnt);
    		assert(rtVal != NULL);
            if (symtab.last_result == RES_REINS) {
                sprintf(symbuf, "redefinition of Identifier `%s`\n", id->name);
                yyerror(symbuf, node->line_no, node->token_pos);
            }
    		rtVal->array_size = node->arraySize;
    		indent(); printf("var %d T%d // %s[%d]\n", node->arraySize*4, nativecnt++, id->name, node->arraySize);
    	}
    	else {
    		rtVal = symtab.regisiter(id->name, TYPE_INT, VAR_NATIVE, nativecnt);
    		assert(rtVal != NULL);
            if (symtab.last_result == RES_REINS) {
                sprintf(symbuf, "redefinition of Identifier `%s`\n", id->name);
                yyerror(symbuf, node->line_no, node->token_pos);
            }
    		indent(); printf("var T%d // %s\n", nativecnt++, id->name);
    	}
    }
    virtual void visit(VarDecl* node) {
    	Identifier* id = (Identifier*)node->children[0];
    }
    virtual void visit(FuncDefn* node) {
    	FuncBody* funcbody;
    	if (node->isMain) {
    		funcbody = (FuncBody*)node->children[0];
    		indent(); printf("f_main [0]\n");
    		symtab.enter("func_main");
            setIndent();
    		funcbody->accept(*this);
            unsetIndent();
    		symtab.leave();
    		indent(); printf("end f_main\n");
    	}
    	else {
	    	Identifier* id = (Identifier*)node->children[0];
	    	SymTabEntry* rtVal;
	    	int pass;
	    	rtVal = symtab.regisiter(id->name, TYPE_FUNC_INT);
	    	assert(rtVal != NULL);
	    	if (node->hasParam) {
	    		List* paramList = (List*)node->children[1];
	    		if (symtab.last_result == RES_REINS)
	    			pass = rtVal->check_param(paramList);
	    		else
	    			pass = rtVal->add_list(paramList);
	    		assert(pass >= 0);
	    		funcbody = (FuncBody*)node->children[2];
	    		indent(); printf("f_%s [%d]\n", id->name, paramList->num_item);
	    	}
	    	else {
	    		if (symtab.last_result == RES_REINS)
	    			pass = rtVal->check_param(NULL);
	    		assert(pass >= 0);
	    		funcbody = (FuncBody*)node->children[1];
	    		indent(); printf("f_%s [0]\n", id->name);
	    	}
	    	sprintf(symbuf, "func_%s", id->name);
	    	symtab.enter(symbuf, rtVal->paramList);
            setIndent();
	    	funcbody->accept(*this);
            unsetIndent();
	    	symtab.leave();
	    	indent(); printf("end f_%s\n", id->name);
	    }
        if (!funcbody->has_return) {
            yyerror("not all code paths return a value\n", node->line_no, node->token_pos);
            // some test points (e.g. long_code) violate this
        }
    }
    virtual void visit(FuncDecl* node) {
    	Identifier* id = (Identifier*)node->children[0];
    	SymTabEntry* rtVal;
	    rtVal = symtab.regisiter(id->name, TYPE_FUNC_INT);
	    assert(rtVal != NULL);
    	if (node->hasParam) {
    		List* paramList = (List*)node->children[1];
			int pass = rtVal->add_list(paramList);
	    	assert(pass >= 0);
    	}
    }
    virtual void visit(FuncBody* node) {
    	// ENTERING INNER ENV (in visit(FuncDefn*))
        for (ASTNode* i = node->first_child; i != NULL; i = i->next) {
        	i->accept(*this);
            if (i->has_return){
                node->has_return = true;
            }
        }
        // LEAVING INNER ENV (in visit(FuncDefn*))
    }
    virtual void visit(List* node) {
    	// should distinguish param and arg
        if (list_stat == LIST_ARGS) {
	        for (ASTNode* i = node->last_child; i != NULL; i = i->prev) {
	        	i->accept(*this);
	        }
	    }
    }
    virtual void visit(Stmt* node) {
        switch(node->stmt_type) {
            case STMT_EMPTY: break;
            case STMT_MULTI: {
            	// ENTERING INNER ENV
            	symtab.enter("stmt_block");
            	for (ASTNode* i = node->first_child; i != NULL; i = i->next) {
        			i->accept(*this);
                    if (i->has_return){
                        node->has_return = true;
                    }
        		} 
        		symtab.leave();
        		break; 
        		// LEAVING INNER ENV
        	}
            case STMT_MATCHED_IF: {
            	Expr* expr = (Expr*)node->children[0];
            	Stmt* stmt_true = (Stmt*)node->children[1];
            	Stmt* stmt_false = (Stmt*)node->children[2];
            	expr->accept(*this);
            	char* symbol = popSymStack();
            	// int labeltrue = labelcnt++;
            	int labelfalse = labelcnt++;
            	int labelmerge = labelcnt++;
            	indent(); printf("if %s==0 goto l%d\n", symbol, labelfalse);
            	// indent(); printf("goto l%d\n", labelfalse);
            	// printf("l%d:\n", labeltrue); // label_true
            	// ENTERING INNER ENV
            	symtab.enter("if_stmt_true");
            	stmt_true->accept(*this);
            	symtab.leave();
            	// LEAVING INNER ENV
            	indent(); printf("goto l%d\n", labelmerge);
            	printf("l%d:\n", labelfalse); // label_false
            	// ENTERING INNER ENV
            	symtab.enter("if_stmt_false");
            	stmt_false->accept(*this);
            	symtab.leave();
            	// LEAVING INNER ENV
            	printf("l%d:\n", labelmerge); // label_merge
                if (stmt_true->has_return && stmt_false->has_return){
                    node->has_return = true;
                }
            	break; 
            }
            case STMT_OPEN_IF: { 
            	Expr* expr = (Expr*)node->children[0];
            	Stmt* stmt_true = (Stmt*)node->children[1];
            	expr->accept(*this);
            	char* symbol = popSymStack();
            	int labelmerge = labelcnt++;
            	indent(); printf("if %s==0 goto l%d\n", symbol, labelmerge);
            	// ENTERING INNER ENV
            	symtab.enter("if_stmt_true");
            	stmt_true->accept(*this);
            	symtab.leave();
            	// LEAVING INNER ENV
            	printf("l%d:\n", labelmerge); // label_merge
            	break; 
            }
            case STMT_WHILE: { 
            	Expr* expr = (Expr*)node->children[0];
            	Stmt* stmt = (Stmt*)node->children[1];
            	int labelcheck = labelcnt++;
            	int labelbreak = labelcnt++;
            	printf("l%d:\n", labelcheck); // label_check
            	expr->accept(*this);
            	char* symbol = popSymStack();
            	indent(); printf("if %s==0 goto l%d\n", symbol, labelbreak);
            	// ENTERING	INNER ENV
            	symtab.enter("while_stmt");
            	stmt->accept(*this);
            	symtab.leave();
            	// LEAVING INNER ENV
            	indent(); printf("goto l%d\n", labelcheck);
            	printf("l%d:\n", labelbreak); // label_break
            	break; 
            }
            case STMT_ASSIGN: { 
            	Identifier* id = (Identifier*)node->children[0];
            	Expr* expr = (Expr*)node->children[1];
            	expr->accept(*this);
            	char* symbol_rval = popSymStack();
            	id->accept(*this);
            	char* symbol_id = popSymStack();
            	indent(); printf("%s = %s\n", symbol_id, symbol_rval);
            	break; 
            }
            case STMT_ARRAY_ASSIGN: { 
            	Identifier* id = (Identifier*)node->children[0];
            	Expr* expr_idx = (Expr*)node->children[1];
            	Expr* expr_rval = (Expr*)node->children[2];
            	expr_rval->accept(*this);
            	char* symbol_rval = popSymStack();
            	expr_idx->accept(*this);
            	char* symbol_idx = popSymStack();
            	int temp_id = tempcnt++;
            	indent(); printf("var t%d\n", temp_id);
            	indent(); printf("t%d = 4 * %s\n", temp_id, symbol_idx);
            	id->accept(*this);
            	char* symbol_id = popSymStack();
            	indent(); printf("%s[t%d] = %s\n", symbol_id, temp_id, symbol_rval);
            	break; 
            }
            case STMT_FUNC_CALL: { 
            	Identifier* id = (Identifier*)node->children[0];
            	List* arglist = (List*)node->children[1];
            	SymTabEntry* rtVal = symtab.lookup(id->name);
            	assert(rtVal != NULL);
            	assert(rtVal->type == TYPE_FUNC_INT);
            	list_stat = LIST_ARGS;
            	arglist->accept(*this);
            	list_stat = LIST_NONE;
            	int pass = rtVal->check_args(arglist);
            	assert(pass >= 0);
            	// TYPE CHECKING HERE (ARGLIST!)
		        for (int i = 0; i < arglist->num_item; i++) {
		        	indent(); printf("param %s\n", popSymStack());
                }
            	indent(); printf("call f_%s\n", id->name);
            	break; 
            }
            case STMT_FUNC_CALL_NOARG: { 
            	Identifier* id = (Identifier*)node->children[0];
            	SymTabEntry* rtVal = symtab.lookup(id->name);
            	assert(rtVal != NULL);
            	assert(rtVal->type == TYPE_FUNC_INT);
            	indent(); printf("call f_%s\n", id->name);
            	break; 
            }
            case STMT_EXPR: { 
            	Expr* expr = (Expr*)node->children[0];
            	expr->accept(*this);
            	popSymStack();
            	break; 
            }
            case STMT_DEFN: { 
            	VarDefn* vardefn = (VarDefn*)node->children[0];
            	vardefn->accept(*this);
            	break; 
            }
            case STMT_RET: { 
            	Expr* expr = (Expr*)node->children[0];
            	expr->accept(*this);
            	assert(expr->type == TYPE_INT);
            	char* symbol_ret = popSymStack();
            	indent(); printf("return %s\n", symbol_ret);
                node->has_return = true;
            	break; 
            }
        }
    }
    virtual void visit(Expr* node) {
    	// push a temp symbol at the end of visit(Expr*)
    	// pop can be flexible to demand
    	switch(node->op) {
            case OPR_INT: { 
            	Integer* integer = (Integer*)node->children[0];
            	integer->accept(*this);
            	node->type = TYPE_INT;
            	break;
        	}
            case OPR_ID: {
            	Identifier* id = (Identifier*)node->children[0];
            	id->accept(*this);
            	node->type = id->type;
            	break;
            }
            case OPR_IDX: { 
            	Identifier* id = (Identifier*)node->children[0];
            	Expr* expr_idx = (Expr*)node->children[1];
            	id->accept(*this);
            	assert(id->type == TYPE_INT_ARRAY);
            	char* symbol_id = popSymStack();
            	expr_idx->accept(*this);
            	assert(expr_idx->type == TYPE_INT);
            	char* symbol_idx = popSymStack();
            	node->type = TYPE_INT;
            	int temp_idx = tempcnt++;
            	indent(); printf("var t%d\n", temp_idx);
            	indent(); printf("t%d = 4 * %s\n", temp_idx, symbol_idx);
            	int temp_id = tempcnt++;
            	indent(); printf("var t%d\n", temp_id);
            	indent(); printf("t%d = %s[t%d]\n", temp_id, symbol_id, temp_idx);
            	sprintf(symbuf, "t%d", temp_id);
            	pushSymStack(symbuf);
            	break;
            }
            case OPR_FUNC: { 
            	Identifier* id = (Identifier*)node->children[0];
            	List* arglist = (List*)node->children[1];
            	SymTabEntry* rtVal = symtab.lookup(id->name);
            	assert(rtVal != NULL);
            	assert(rtVal->type == TYPE_FUNC_INT);            	
            	list_stat = LIST_ARGS;
            	arglist->accept(*this);
            	list_stat = LIST_NONE;
            	int pass = rtVal->check_args(arglist);
            	assert(pass >= 0);
            	// TYPE CHECKING HERE (ARGLIST!)
            	node->type = TYPE_INT;
            	int temp_id = tempcnt++;
            	indent(); printf("var t%d\n", temp_id);
		        for (int i = 0; i < arglist->num_item; i++){
		        	indent(); printf("param %s\n", popSymStack());            	
                }
            	indent(); printf("t%d = call f_%s\n", temp_id, id->name);
            	sprintf(symbuf, "t%d", temp_id);
            	pushSymStack(symbuf);
            	break;
            }
            case OPR_FUNCNOARG: { 
            	Identifier* id = (Identifier*)node->children[0];
            	SymTabEntry* rtVal = symtab.lookup(id->name);
            	assert(rtVal != NULL);
            	assert(rtVal->type == TYPE_FUNC_INT);
            	node->type = TYPE_INT;
            	int temp_id = tempcnt++;
            	indent(); printf("var t%d\n", temp_id);
            	indent(); printf("t%d = call f_%s\n", temp_id, id->name);
            	sprintf(symbuf, "t%d", temp_id);
            	pushSymStack(symbuf);
            	break;
        	} 
            case OPR_AND: { // short-circuiting code
                Expr* expr_left = (Expr*)node->children[0];
                Expr* expr_right = (Expr*)node->children[1];
                expr_left->accept(*this);
                char* symbol_left = popSymStack();
                assert(expr_left->type == TYPE_INT);
                int temp_id = tempcnt++;
                indent(); printf("var t%d\n", temp_id);
                int labelfalse = labelcnt++;
                // when logic expr in right need NOT to be evaled
                int labelnext  = labelcnt++;
                indent(); printf("if %s==0 goto l%d\n", symbol_left, labelfalse);
                expr_right->accept(*this);
                char* symbol_right = popSymStack();
                assert(expr_right->type == TYPE_INT);
                indent(); printf("if %s==0 goto l%d\n", symbol_right, labelfalse);
                indent(); printf("t%d = 1\n", temp_id);
                indent(); printf("goto l%d\n", labelnext);
                printf("l%d:\n", labelfalse);
                indent(); printf("t%d = 0\n", temp_id);
                printf("l%d:\n", labelnext);
                node->type = TYPE_INT;
                sprintf(symbuf, "t%d", temp_id);
                pushSymStack(symbuf);       
                break;
            }
            case OPR_OR: { // short-circuiting code
                Expr* expr_left = (Expr*)node->children[0];
                Expr* expr_right = (Expr*)node->children[1];
                expr_left->accept(*this);
                char* symbol_left = popSymStack();
                assert(expr_left->type == TYPE_INT);
                int temp_id = tempcnt++;
                indent(); printf("var t%d\n", temp_id);
                int labelright = labelcnt++; 
                // when logic expr in right need to be evaled
                int labelfalse = labelcnt++;
                int labelnext  = labelcnt++;
                indent(); printf("if %s==0 goto l%d\n", symbol_left, labelright);
                indent(); printf("t%d = 1\n", temp_id);
                indent(); printf("goto l%d\n", labelnext);
                printf("l%d:\n", labelright);
                expr_right->accept(*this);
                char* symbol_right = popSymStack();
                assert(expr_right->type == TYPE_INT);
                indent(); printf("if %s==0 goto l%d\n", symbol_right, labelfalse);
                indent(); printf("t%d = 1\n", temp_id);
                indent(); printf("goto l%d\n", labelnext);
                printf("l%d:\n", labelfalse);
                indent(); printf("t%d = 0\n", temp_id);
                printf("l%d:\n", labelnext);
                node->type = TYPE_INT;
                sprintf(symbuf, "t%d", temp_id);
                pushSymStack(symbuf);
                break;
            }
            case OPR_ASS: break; // not implemented yet
            case OPR_INCRET: break; // not implemented yet
            case OPR_RETINC: break; // not implemented yet
            case OPR_DECRET: break; // not implemented yet
            case OPR_RETDEC: break; // not implemented yet
    		default:
		    	switch(node->num_operand) {
		    		case 2: {
		            	Expr* expr_left = (Expr*)node->children[0];
                        Expr* expr_right = (Expr*)node->children[1];
		            	expr_left->accept(*this);
		            	char* symbol_left = popSymStack();
                        expr_right->accept(*this);
                        char* symbol_right = popSymStack();
		            	assert(expr_left->type == TYPE_INT);
                        assert(expr_right->type == TYPE_INT);
		            	int temp_id = tempcnt++;
		            	indent(); printf("var t%d\n", temp_id);
		            	const char* str_op;
		            	switch(node->op) {
		            		case OPR_ADD: str_op = "+"; break;
		            		case OPR_SUB: str_op = "-"; break;
		            		case OPR_MUL: str_op = "*"; break;
		            		case OPR_DIV: str_op = "/"; break;
		            		case OPR_MOD: str_op = "%"; break;
		            		case OPR_LT: str_op = "<"; break;
		            		case OPR_GT: str_op = ">"; break;
		            		case OPR_EQ: str_op = "=="; break;
		            		case OPR_NE: str_op = "!="; break;
		            		case OPR_LE: str_op = "<="; break; // not implemented yet
		            		case OPR_GE: str_op = ">="; break; // not implemented yet
		            		case OPR_LSH: str_op = "<<"; break; // not implemented yet
		            		case OPR_RSH: str_op = ">>"; break; // not implemented yet
		            		default: yyerror("invalid binary operation\n", node->line_no, node->token_pos);
		            	}
		            	node->type = TYPE_INT;
		            	indent(); printf("t%d = %s %s %s\n", temp_id, symbol_left, str_op, symbol_right);
		            	sprintf(symbuf, "t%d", temp_id);
		            	pushSymStack(symbuf);
		    			break;
		    		}
		    		case 1: {
		    			Expr* expr = (Expr*)node->children[0];
		    			expr->accept(*this);
		    			char* symbol = popSymStack();
		            	// set type for lval of the expr here
		    			int temp_id = tempcnt++;
		    			indent(); printf("var t%d\n", temp_id);
		    			const char* str_op;
		    			switch(node->op) {
				            case OPR_NOT: str_op = "!"; break;
				            case OPR_NEG: str_op = "-"; break;
				            default: yyerror("invalid unary operation\n", node->line_no, node->token_pos);
		            	}
		            	node->type = expr->type;
		            	indent(); printf("t%d = %s %s\n", temp_id, str_op, symbol);
		            	sprintf(symbuf, "t%d", temp_id);
		            	pushSymStack(symbuf);
		    			break;
		    		}
		    		default: indent(); printf("Error at expression!\n");
		    	}
    	}
    }
    virtual void visit(Identifier* node)  {
		SymTabEntry* rtVal = symtab.lookup(node->name);
		assert(rtVal != NULL);
		node->type = rtVal->type;
		switch(rtVal->var_type) {
			case VAR_NATIVE: sprintf(symbuf, "T%d", rtVal->var_no); break;
			case VAR_TEMP: sprintf(symbuf, "t%d", rtVal->var_no); break;
			case VAR_PARAM: sprintf(symbuf, "p%d", rtVal->var_no); break;
		}
    	pushSymStack(symbuf);
    }
    virtual void visit(Integer* node) {
    	sprintf(symbuf, "%d", node->val);
    	pushSymStack(symbuf);
    }
};

#endif