/* Generating RISCV assembly corresponding to Tigger grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "Tigger.y.tab.hh"
#include "Tigger.typedef.hh"
#include "Tigger2RISCV.hh"

using namespace std;

RISCVGenerator riscvGenerator;
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

    if (out_file)
        auto eat = freopen(argv[3], "w", stdout);

    generate_riscv(riscvGenerator);

    fprintf(stderr, "RISCV assembly generation finished!\n");
    return 0;
}