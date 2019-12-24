/* MiniC2Eeyore corresponding to base miniC grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "MiniC.AST.hh"
#include "MiniC.symtab.hh"
#include "MiniC2Eeyore.hh"
#include "MiniC.y.tab.hh"

using namespace std;

extern ASTNode* root;
EeyoreGenerator eeyoreGenerator;
extern FILE* yyin;
extern FILE* yyout;

void help(char* path) {
	cout << "Usage: " << path << "[<filename> [-o <filename>]]" << endl;
}

int main(int argc, char* argv[]) {
	bool out_file = false;
	if (argc == 1) {
		yyin = stdin;
	}
	else if (argc == 2) {
		yyin = fopen(argv[1], "r");
	}
	else if (argc == 4 && argv[2] == string("-o")) {
		yyin = fopen(argv[1], "r");
		out_file = true;
	}
	else {
		help(argv[0]);
		exit(-1);
	}
	yyparse();

    ASTNode::printTree(root);
    printf_ast("\nnode count: %d\n", ASTNode::nodesno);
    printf_ast(">>>>>>>>>>>>> End of syntactic analysis <<<<<<<<<<<<<\n\n");

    if (out_file)
   		auto eat = freopen(argv[3], "w", stdout);
    root->accept(eeyoreGenerator);
    auto eat = fprintf(stderr, "Eeyore generation finished!\n");
    return 0;
}