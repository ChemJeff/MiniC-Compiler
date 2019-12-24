/* Symbol table corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __MINIC_SYMTAB_H__
#define __MINIC_SYMTAB_H__


//#define DEBUGSYM
#ifdef DEBUGSYM
#define dprintf_sym(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_sym(fmt, ...)
#endif

#define VAR_NONE 0
#define VAR_NATIVE 1
#define VAR_TEMP 2
#define VAR_PARAM 3

#define RES_NONE 0
#define RES_CONF 1
#define RES_REINS 2
#define RES_NEWINS 3
#define RES_FOUND 4
#define RES_NOTFD 5

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MiniC.AST.hh"

struct ParamEntry;
struct SymTabEntry;
struct ScopeEntry;
struct SymTab;

struct ParamEntry {
	char* name;
	int type;
	int array_size;
	ParamEntry* next = NULL;
	~ParamEntry() {
		delete name;
	}
};

struct SymTabEntry {
	char* name;
	ScopeEntry* scope;
	int type; // NONE, INT, INT_ARRAY, FUNC_INT, FUNC_INT_ARRAY
	int num_param = 0;
	int array_size = 0;
	int var_type = VAR_NONE;
	int var_no;
	SymTabEntry* prev = NULL;
	SymTabEntry* next = NULL;
	ParamEntry* paramList = NULL;
	ParamEntry* paramListEnd = NULL;
	~SymTabEntry() {
		ParamEntry* oldptr;
		for (ParamEntry* ptr = paramList; ptr != NULL;) {
			oldptr = ptr;
			ptr = ptr->next;
			delete oldptr;
		}
		delete name;
	}

	int add_list(List* paramList) {
		dprintf_sym("in `add_list` with paramList %s\n", (paramList==NULL?"empty":"non-empty"));
		assert(this->type == TYPE_FUNC_INT || this->type == TYPE_FUNC_INT_ARRAY);
		if (num_param != 0 && paramList != NULL || paramList == NULL)
			return -1;
		ASTNode* ptr = paramList->first_child;
		for (; ptr != NULL; ptr = ptr->next) {
			//dprintf_sym("name = %s, type = %d\n", ((Identifier*)(ptr->children[0]))->name, ptr->type);
			if (ptr->type == TYPE_INT_ARRAY)
				add_param(((Identifier*)(ptr->children[0]))->name, ptr->type, ((VarDecl*)ptr)->arraySize);
			else if (ptr->type == TYPE_INT)
				add_param(((Identifier*)(ptr->children[0]))->name, ptr->type);
			else
				return -1;
		}
		return 0;
	}

	int check_param(List* paramList) {
		dprintf_sym("in `check_param` with paramList %s\n", (paramList==NULL?"empty":"non-empty"));
		if (paramList == NULL && num_param > 0 || paramList != NULL && num_param != paramList->num_item) {
			dprintf_sym("Type checking for params fail\n");
			return -1;
		}
		if (paramList == NULL && num_param == 0)
			return 0;
		ASTNode* decl = paramList->first_child;
		ParamEntry* ptr = this->paramList;
		for (; decl != NULL && ptr != NULL; decl = decl->next, ptr = ptr->next) {
			if (decl->type != ptr->type) {
				dprintf_sym("Type checking for params fail\n");
				return -1;
			}
			if (decl->type == TYPE_INT_ARRAY && ptr->array_size != ((VarDecl*)decl)->arraySize) {
				dprintf_sym("Type checking for params fail\n");
				return -1;
			}
			// param_name in FuncDefn may differ from those in FuncDecl, update name
			delete ptr->name;
			ptr->name = strdup(((Identifier*)(decl->children[0]))->name);
		}
		return 0;		
	}

	int check_args(List* argList) {
		dprintf_sym("in `check_args` with argList %s\n", (argList==NULL?"empty":"non-empty"));
		if (argList == NULL && num_param > 0 || argList != NULL && num_param != argList->num_item) {
			dprintf_sym("Type checking for args fail\n");
			return -1;
		}
		if (argList == NULL && num_param == 0)
			return 0;
		ASTNode* expr = argList->first_child;
		ParamEntry* ptr = this->paramList;
		for (; expr != NULL && ptr != NULL; expr = expr->next, ptr = ptr->next) {
			if (expr->type != ptr->type) {
				dprintf_sym("Type checking for args fail\n");
				return -1;
			}
		}
		return 0;
	}

	void add_param(char* name, int type, int array_size=0) {
		dprintf_sym("add param \"%s\" of type %d to function \"%s\"\n", name, type, this->name);
		assert(type == TYPE_INT || type == TYPE_INT_ARRAY ||
			type == TYPE_FUNC_INT || type == TYPE_FUNC_INT_ARRAY);
		num_param++;
		ParamEntry* new_param = new ParamEntry();
		new_param->name = strdup(name);
		new_param->type = type;
		new_param->array_size = array_size;
		if (paramList == NULL) {
			paramList = paramListEnd = new_param;
		}
		else {
			paramListEnd->next = new_param;
			paramListEnd = new_param;
		}
	}
};

struct ScopeEntry {
	char* scope_name;
	SymTabEntry* symTabScope = NULL;
	ScopeEntry* prev = NULL;
	ScopeEntry* next = NULL;
	~ScopeEntry() {
		SymTabEntry* oldptr;
		for (SymTabEntry* ptr = symTabScope; ptr != NULL;) {
			oldptr = ptr;
			ptr = ptr->next;
			dprintf_sym("NOTE: Remove symbol \"%s\" of type %d at scope \"%s\"\n"
				, oldptr->name, oldptr->type, scope_name);
			delete oldptr;
		}
		delete scope_name;
	}
};

struct SymTab {
	int scopeDepth;
	int symCount;
	int paramcnt;
	int last_result;
	ScopeEntry* globalScope;
	ScopeEntry* currScope;
	SymTab() {
		globalScope = new ScopeEntry();
		globalScope->scope_name = strdup("global");
		currScope = globalScope;
		scopeDepth = 1;
		symCount = 0;
		paramcnt = 0;
		last_result = RES_NONE;
		// regisiter("main", TYPE_FUNC_INT);
		// regisiter("getint", TYPE_FUNC_INT);
		// regisiter("putint", TYPE_FUNC_INT);
		// regisiter("getchar", TYPE_FUNC_INT);
		// regisiter("putchar", TYPE_FUNC_INT);
	}
	~SymTab() {
		ScopeEntry* oldptr;
		for (ScopeEntry* ptr = globalScope; ptr != NULL;) {
			oldptr = ptr;
			ptr = ptr->next;
			delete oldptr;
		}
	}
	SymTabEntry* regisiter(char* id, int type, int var_type = VAR_NONE, int var_no = 0) {
		SymTabEntry* rtVal = lookup_scope(id, currScope);
		if (rtVal != NULL) {
			if (rtVal->type != type) {
				dprintf_sym("NOTE: Insert conflict found for symbol \"%s\" at scope \"%s\"\n",
				 id, currScope->scope_name);
				last_result = RES_CONF;
				return NULL;
			}
			else {
				dprintf_sym("NOTE: Reinsert symbol \"%s\" of type %d, var_type %d, var_no %d at scope \"%s\"\n",
					id, type, var_type, var_no, currScope->scope_name);
				symCount++;
				last_result = RES_REINS;
				return rtVal;
			}
		}
		else {
			rtVal = new SymTabEntry();
			rtVal->name = strdup(id);
			rtVal->type = type;
			if (var_type != VAR_NONE) {
				rtVal->var_type = var_type;
				rtVal->var_no = var_no;
			}
			if (currScope->symTabScope == NULL)
				currScope->symTabScope = rtVal;
			else {
				currScope->symTabScope->prev = rtVal;
				rtVal->next = currScope->symTabScope;
				currScope->symTabScope = rtVal;
			}
			dprintf_sym("NOTE: Insert symbol \"%s\" of type %d, var_type %d, var_no %d at scope \"%s\"\n",
					id, type, var_type, var_no, currScope->scope_name);
			symCount++;
			last_result = RES_NEWINS;
			return rtVal;
		}
	}
	SymTabEntry* lookup(char* id) {
		ScopeEntry* scope_ptr;
		SymTabEntry* rtVal;
		for (scope_ptr = currScope; scope_ptr != NULL; scope_ptr = scope_ptr->prev) {
			rtVal = lookup_scope(id, scope_ptr);
			if (rtVal != NULL) {
				dprintf_sym("NOTE: Symbol \"%s\" found of type %d, var_type %d, var_no %d\n",
					id, rtVal->type, rtVal->var_type, rtVal->var_no);
				return rtVal;
			}
		}
		dprintf_sym("NOTE: Symbol \"%s\" not found\n", id);
		return NULL;
	}
	SymTabEntry* lookup_scope(char* id, ScopeEntry* scope) {
		SymTabEntry* rtVal;
		for (rtVal = scope->symTabScope; rtVal != NULL; rtVal = rtVal->next) {
			if (strcmp(rtVal->name, id) == 0)
				return rtVal;
		}
		return NULL;
	}
	void enter(const char* scope_name, ParamEntry* paramList=NULL) {
		dprintf_sym("NOTE: Entering scope: \"%s\", with paramList %s ...\n", scope_name, (paramList==NULL?"empty":"non-empty"));
		ScopeEntry* newScope = new ScopeEntry();
		newScope->scope_name = strdup(scope_name);
		currScope->next = newScope;
		newScope->prev = currScope;
		currScope = newScope;
		scopeDepth++;
		paramcnt = 0;
		for (; paramList != NULL; paramList = paramList->next) {
			regisiter(paramList->name, paramList->type, VAR_PARAM, paramcnt++);
		}
		dprintf_sym("NOTE: Entering success\n");
	}
	void leave() {
		dprintf_sym("NOTE: Leaving scope: \"%s\" ...\n", currScope->scope_name);
		assert(currScope != globalScope);
		ScopeEntry* oldScope = currScope;
		currScope = currScope->prev;
		currScope->next = NULL;
		delete oldScope;
		scopeDepth--;
		paramcnt = 0;
		dprintf_sym("NOTE: Leaving success\n");
	}
};


#endif