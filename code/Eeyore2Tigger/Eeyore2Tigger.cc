/* Generating Tigger code corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "Eeyore.y.tab.hh"
#include "Eeyore.typedef.hh"
#include "Eeyore.liveness.hh"
#include "Eeyore.regalloc.hh"
#include "Eeyore2Tigger.hh"

using namespace std;

TiggerGenerator tiggerGenerator;
IstrEmitter istrEmitter;
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
	print_expr();

	//return 0;

    if (out_file)
   		auto eat = freopen(argv[3], "w", stdout);

	//optimize_eeyore();
	calc_var2idx();
	calc_label2idx();

	//return 0;

	init_liveness();
	iter_liveness();
	calc_liveness_interval();
	print_liveness_line();
	print_liveness();

	calc_blocks();
	print_blocks();

	//return 0;

	generate_tigger(tiggerGenerator);

	//return 0;
	//optimize_tigger();
	emit_tigger(istrEmitter);

	fprintf(stderr, "Tigger generation finished!\n");
	return 0;
}