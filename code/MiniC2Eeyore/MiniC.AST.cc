/* Abstract grammar tree corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MiniC.AST.hh"

NodePrinter nodePrinter;

void ASTNode::insert(ASTNode* child, bool insert_front) {
    assert(num_child < MAX_CHILDREN_NODES);
    if (num_child == 0) {
        this->first_child = child;
        this->last_child = child;
        child->next = NULL;
        child->prev = NULL;
    }
    else {
        if (insert_front) {
            this->first_child->prev = child;
            child->next = this->first_child;
            child->prev = NULL;
            this->first_child = child;
        }
        else {
            this->last_child->next = child;
            child->prev = this->last_child;
            child->next = NULL;
            this->last_child = child;
        }
    }
    children[num_child++] = child;
    child->parent = this;
}

void ASTNode::printTree(ASTNode* root) {
    if (ASTNode::printDepth > 0) {
        for (int i = 0; i < ASTNode::printDepth; i++)
            printf_ast("   |");
        printf_ast("-");
    }
    root->accept(nodePrinter); // line feed in `nodePrinter`
    ASTNode::printDepth++;
    for (ASTNode* child = root->first_child; child != NULL; child = child->next)
        ASTNode::printTree(child);
    ASTNode::printDepth--;
}
