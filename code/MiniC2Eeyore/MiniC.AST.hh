/* Abstract grammar tree corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __MINIC_AST_H__
#define __MINIC_AST_H__


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUGAST
#define dprintf_ast(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_ast(fmt, ...)
#endif

#ifdef PRINTAST
#define printf_ast(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define printf_ast(fmt, ...)
#endif

#define TYPE_NONE 0
#define TYPE_INT 1
#define TYPE_FUNC 0x80000
#define TYPE_ARRAY 0x8000
#define TYPE_INT_ARRAY 0x8001
#define TYPE_FUNC_INT 0x80001
#define TYPE_FUNC_INT_ARRAY 0x88001

#define NODE_UNDEF 0
#define NODE_GOAL 1
#define NODE_VARDEFN 2
#define NODE_VARDECL 3
#define NODE_FUNCDEFN 4
#define NODE_FUNCDECL 5
#define NODE_FUNCBODY 6
#define NODE_LIST 7
#define NODE_STMT 8
#define NODE_EXPR 9
#define NODE_IDENTIFIER 10
#define NODE_INTEGER 11

#define MAX_CHILDREN_NODES 128
#define MAX_NODES 4096

#define STMT_EMPTY 0
#define STMT_MULTI 1
#define STMT_MATCHED_IF 2
#define STMT_OPEN_IF 3
#define STMT_WHILE 4
#define STMT_ASSIGN 5
#define STMT_ARRAY_ASSIGN 6
#define STMT_FUNC_CALL 7
#define STMT_FUNC_CALL_NOARG 8
#define STMT_EXPR 9
#define STMT_DEFN 10
#define STMT_RET 11

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
#define OPR_INT 19
#define OPR_ID 20
#define OPR_INCRET 21
#define OPR_RETINC 22
#define OPR_DECRET 23
#define OPR_RETDEC 24
#define OPR_IDX 25
#define OPR_FUNC 26
#define OPR_FUNCNOARG 27

extern int lineno;
extern int tokenspos;

struct ASTNode;
struct ASTVisitor;

struct NodePrinter;

struct Goal;
struct VarDefn;
struct VarDecl;
struct FuncDefn;
struct FuncDecl;
struct FuncBody;
struct List;
struct Stmt;
struct Expr;
struct Identifier;
struct Integer;

// Abstract class
struct ASTVisitor {
    char* name;
    virtual void visit(Goal*) = 0;
    virtual void visit(VarDefn*) = 0;
    virtual void visit(VarDecl*) = 0;
    virtual void visit(FuncDefn*) = 0;
    virtual void visit(FuncDecl*) = 0;
    virtual void visit(FuncBody*) = 0;
    virtual void visit(List*) = 0;
    virtual void visit(Stmt*) = 0;
    virtual void visit(Expr*) = 0;
    virtual void visit(Identifier*) = 0;
    virtual void visit(Integer*) = 0;
};

struct ASTNode {
    static int printDepth;
    static void printTree(ASTNode* root);
    static int nodesno;

    int num_child = 0;
    int line_no;
    int token_pos;
    int type;
    int nodetype = NODE_UNDEF;
    bool has_return = false; // for return checking

    ASTNode* parent = NULL;
    ASTNode* children[MAX_CHILDREN_NODES];
    ASTNode* prev = NULL;   // prev sibling (in linked list)
    ASTNode* next = NULL;   // next sibling (in linked list)
    ASTNode* first_child = NULL; 
    ASTNode* last_child = NULL; // for conveience of inserting

    ASTNode() {
        ASTNode::nodesno++;
        this->num_child = 0;
        this->line_no = lineno;
        this->token_pos = tokenspos;
        this->type = TYPE_NONE;
        this->nodetype = NODE_UNDEF;
        this->parent = NULL;
        this->prev = NULL;
        this->next = NULL;
        this->first_child = NULL;
        this->last_child = NULL;
    }
    ~ASTNode() {ASTNode::nodesno--;}
    virtual void accept(ASTVisitor &visitor) = 0;
    virtual void setType(int type) { this->type = type; }
    void insert(ASTNode* child, bool insert_front=0);
};

struct Goal: public ASTNode {
    int num_block;
    Goal(): ASTNode() {
        this->nodetype = NODE_GOAL;
        this->num_block = 1;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("Goal visited by %s\n", visitor.name);
        visitor.visit(this);}
    void add_block() {this->num_block++;}
};

struct VarDefn: public ASTNode {
    bool isArray;
    int arraySize;
    VarDefn(bool isArray, int arraySize=0): ASTNode() {
        this->nodetype = NODE_VARDEFN;
        this->isArray = isArray;
        this->arraySize = arraySize;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("VarDefn visited by %s\n", visitor.name);
        visitor.visit(this);}
};

struct VarDecl: public ASTNode {
    bool isArray;
    int arraySize;
    VarDecl(bool isArray, int arraySize=0): ASTNode() {
        this->nodetype = NODE_VARDECL;
        this->isArray = isArray;
        this->arraySize = arraySize;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("VarDecl visited by %s\n", visitor.name);
        visitor.visit(this);}
};

struct FuncDefn: public ASTNode {
    bool isMain;
    bool hasParam;
    FuncDefn(bool isMain, bool hasParam): ASTNode() {
        this->nodetype = NODE_FUNCDEFN;
        this->isMain = isMain;
        this->hasParam = hasParam;
    }
    void accept(ASTVisitor& visitor) override {
        if (isMain) {
            dprintf_ast("MainFunc visited by %s\n", visitor.name);
        }
        else {
            dprintf_ast("FuncDefn visited by %s\n", visitor.name);
        }
        visitor.visit(this);}
};

struct FuncDecl: public ASTNode {
    bool hasParam;
    FuncDecl(bool hasParam): ASTNode() {
        this->nodetype = NODE_FUNCDECL;
        this->hasParam = hasParam;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("FuncDecl visited by %s\n", visitor.name);
        visitor.visit(this);}

};

struct FuncBody: public ASTNode {
    int num_stmt;
    FuncBody(int num_stmt): ASTNode() {
        this->nodetype = NODE_FUNCBODY;
        this->num_stmt = num_stmt;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("FuncBody visited by %s\n", visitor.name);
        visitor.visit(this);}
    void add_stmt() {this->num_stmt++;}
};

struct List: public ASTNode {
    int num_item;
    bool isArg;
    List(int num_item, bool isArg): ASTNode() {
        this->nodetype = NODE_LIST;
        this->num_item = num_item;
        this->isArg = isArg;
    }
    void accept(ASTVisitor& visitor) override {
        if (isArg) {
            dprintf_ast("ArgList visited by %s\n", visitor.name);
        }
        else {
            dprintf_ast("ParamList visited by %s\n", visitor.name);
        }
        visitor.visit(this);}
    void add_item() {this->num_item++;}
};

struct Stmt: public ASTNode {
    int stmt_type;
    Stmt(int stmt_type): ASTNode() {
        this->nodetype = NODE_STMT;
        this->stmt_type = stmt_type;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("Stmt visited by %s\n", visitor.name);
        visitor.visit(this);}
};

struct Expr: public ASTNode {
    int op;
    int num_operand;
    Expr(int op, int num_operand): ASTNode() { 
        this->nodetype = NODE_EXPR;
        this->op = op;
        this->num_operand = num_operand;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("Expr visited by %s\n", visitor.name);
        visitor.visit(this);}
};

struct Identifier: public ASTNode {
    char *name;
    Identifier(char* name): ASTNode() {
        this->nodetype = NODE_IDENTIFIER;
        this->name = strdup(name);
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("Identifier visited by %s\n", visitor.name);
        visitor.visit(this);}
};

struct Integer: public ASTNode {
    int val;
    Integer(int val): ASTNode() {
        this->nodetype = NODE_INTEGER;
        this->val = val;
    }
    void accept(ASTVisitor& visitor) override {
        dprintf_ast("Integer visited by %s\n", visitor.name);
        visitor.visit(this);}
};

struct NodePrinter: public ASTVisitor {
    NodePrinter(): ASTVisitor() {this->name = strdup("NodePrinter");}
    ~NodePrinter() {delete this->name;}
    virtual void visit(Goal* node) {
        printf_ast("Goal (num_block = %d)\n", node->num_block);
    }
    virtual void visit(VarDefn* node) {
        if (node->isArray)
            printf_ast("VarDefn (arraySize = %d)\n", node->arraySize);
        else
            printf_ast("VarDefn\n");
    }
    virtual void visit(VarDecl* node) {
        if (node->isArray)
            printf_ast("VarDecl (arraySize = %d)\n", node->arraySize);
        else
            printf_ast("VarDecl\n");
    }
    virtual void visit(FuncDefn* node) {
        if (node->isMain)
            printf_ast("MainFunc\n");
        else {
            if (node->hasParam)
                printf_ast("FuncDefn (with param)\n");
            else
                printf_ast("FuncDefn\n");
        }
    }
    virtual void visit(FuncDecl* node) {
        if (node->hasParam)
            printf_ast("FuncDecl (with param)\n");
        else
            printf_ast("FuncDecl\n");
    }
    virtual void visit(FuncBody* node) {
        printf_ast("FuncBody (num_stmt = %d)\n", node->num_stmt);
    }
    virtual void visit(List* node) {
        if (node->isArg)
            printf_ast("ArgList");
        else
            printf_ast("ParamList");
        printf_ast(" (num_item = %d)\n", node->num_item);
    }
    virtual void visit(Stmt* node) {
        switch(node->stmt_type) {
            case STMT_EMPTY: printf_ast("Empty statement\n"); break;
            case STMT_MULTI: printf_ast("Statements\n"); break;
            case STMT_MATCHED_IF: printf_ast("If statement (matched)\n"); break;
            case STMT_OPEN_IF: printf_ast("If statement (open)\n"); break;
            case STMT_WHILE: printf_ast("While statement\n"); break;
            case STMT_ASSIGN: printf_ast("Assign statement\n"); break;
            case STMT_ARRAY_ASSIGN: printf_ast("Assign statement (array)\n"); break;
            case STMT_FUNC_CALL: printf_ast("Function call\n"); break;
            case STMT_FUNC_CALL_NOARG: printf_ast("Function call (no args)\n"); break;
            case STMT_EXPR: printf_ast("Expression as statement\n"); break;
            case STMT_DEFN: printf_ast("Definition as statement\n"); break;
            case STMT_RET: printf_ast("Return statement\n"); break;
        }
    }
    virtual void visit(Expr* node) {
        switch(node->op) {
            case OPR_NONE: printf_ast("None operation\n"); break;
            case OPR_ADD: printf_ast("Addition\n"); break;
            case OPR_SUB: printf_ast("Substraction\n"); break;
            case OPR_MUL: printf_ast("Multiplication\n"); break;
            case OPR_DIV: printf_ast("Division\n"); break;
            case OPR_MOD: printf_ast("Modulo operation\n"); break;
            case OPR_ASS: printf_ast("Assignment\n"); break;
            case OPR_AND: printf_ast("Logic AND\n"); break;
            case OPR_OR: printf_ast("Logic OR\n"); break;
            case OPR_LT: printf_ast("Logic LT\n"); break;
            case OPR_GT: printf_ast("Logic GT\n"); break;
            case OPR_EQ: printf_ast("Logic EQ\n"); break;
            case OPR_NE: printf_ast("Logic NE\n"); break;
            case OPR_LE: printf_ast("Logic LE\n"); break;
            case OPR_GE: printf_ast("Logic GE\n"); break;
            case OPR_LSH: printf_ast("Left shift\n"); break;
            case OPR_RSH: printf_ast("Right shift\n"); break;
            case OPR_NOT: printf_ast("Logic NOT\n"); break;
            case OPR_NEG: printf_ast("Negative\n"); break;
            case OPR_INT: printf_ast("Integer as expression\n"); break;
            case OPR_ID: printf_ast("Identifier as expression\n"); break;
            case OPR_INCRET: printf_ast("Increment then return\n"); break;
            case OPR_RETINC: printf_ast("Return then increment\n"); break;
            case OPR_DECRET: printf_ast("Decrement then return\n"); break;
            case OPR_RETDEC: printf_ast("Return then decrement\n"); break;
            case OPR_IDX: printf_ast("Index operation\n"); break;
            case OPR_FUNC: printf_ast("Function call as expression\n"); break;
            case OPR_FUNCNOARG: printf_ast("Function call as expression (no args)\n"); break;      
        }
    }
    virtual void visit(Identifier* node)  {
        printf_ast("Identifier (name = %s", node->name);
        switch(node->type) {
            case TYPE_NONE: printf_ast(")\n"); break;
            case TYPE_INT: printf_ast(", type = INT)\n"); break;
            case TYPE_INT_ARRAY: printf_ast(", type = INT ARRAY)\n"); break;
            case TYPE_FUNC_INT: printf_ast(", type = FUNC returns INT)\n"); break;
            default: printf_ast(", type = UNKNOWN)\n");
        }
    }
    virtual void visit(Integer* node) {
        printf_ast("Integer (value = %d)\n", node->val);
    }
};

#endif