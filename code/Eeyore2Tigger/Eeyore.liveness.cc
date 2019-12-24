/* Liveness analysis corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include <assert.h>
#include <string.h>

#include "Eeyore.typedef.hh"
#include "Eeyore.liveness.hh"

extern Expr* exprTab[MAX_EXPRS];
extern Var* varTab[MAX_VARS];
extern int expr_cnt;
extern int func_cnt;
extern int var_cnt;

vector<int> blockEntry;
vector<int> blockExit;
int block_cnt = 0;
bitset<MAX_VARS> livenessTab[MAX_EXPRS + 1];

int var2idx[3][MAX_VARS]; // <0 for none, >=0 in `var_index`
// int var2idx_cnt[3] = {0, 0};

int label2idx[MAX_EXPRS]; // <0 for none, >=0 in `expr_index`

LivenessAnalyzer livenessAnalyzer(exprTab, livenessTab, var2idx, label2idx);

void calc_var2idx() {
	memset(var2idx, -1, sizeof(var2idx));
	for (int i=0; i<var_cnt; i++) {
		int vtype = varTab[i]->var_type;
		int vval = varTab[i]->val;
		if (vtype > VAR_IMMEDIATE && vtype <= VAR_PARAM) {
			// a variable should not be declared twice or more in source
			assert(var2idx[vtype - VAR_NATIVE][vval] < 0);
			var2idx[vtype - VAR_NATIVE][vval] = i;
		}
	}
}

void calc_label2idx() {
	memset(label2idx, -1, sizeof(label2idx));
	for (int i=0; i<expr_cnt; i++) {
		if (exprTab[i]->expr_type == EXPR_LABEL) {
			// a label should not appear twice or more in source
			assert(label2idx[((Label*)(exprTab[i]))->val] < 0);
			label2idx[((Label*)(exprTab[i]))->val] = i;
		}
	}
}

void init_liveness() {
	for (int i=0; i<=expr_cnt; i++)
		livenessTab[i].reset();
	// initialize livenessTab[expr_cnt] = all_zero to simplify operations
}

void iter_liveness() {
	do {
		livenessAnalyzer.fixed = true;
		livenessAnalyzer.expr_cnt = expr_cnt;
		livenessAnalyzer.func_cnt_backwards = func_cnt;
		for (int i=expr_cnt-1; i>=0; i--)
			exprTab[i]->accept(livenessAnalyzer);
	}
	while (!livenessAnalyzer.fixed);
}

void calc_liveness_interval() {
	for (int i=0; i<expr_cnt; i++) {
		for (int j=0; j<var_cnt; j++) {
			if (livenessTab[i][j]) {
				if (varTab[j]->liveBegin > i)
					varTab[j]->liveBegin = i;
				if (varTab[j]->liveEnd <= i)
					varTab[j]->liveEnd = i + 1;
			}
		}
	}
}

void calc_blocks() {
	blockEntry.push_back(0);
	block_cnt++;
	for (int i=1; i<expr_cnt; i++) {
		if (exprTab[i]->isBlockEntry) {
			blockEntry.push_back(i);
			exprTab[i - 1]->isBlockExit = true;
			blockExit.push_back(i - 1);
			block_cnt++;
		}
	}
	exprTab[expr_cnt - 1]->isBlockExit = true;
	blockExit.push_back(expr_cnt - 1);
}

void print_liveness_line() {
	dprintf_liveness("\n>>>>>>>>>>>> Liveness by lines <<<<<<<<<<<<\n");
	for (int i=0; i<expr_cnt; i++) {
		dprintf_liveness("%d: ", i);
		for (int j=0; j<var_cnt; j++) {
			dprintf_liveness("%d", (livenessTab[i][j]?1:0));
		}
		dprintf_liveness("\n");
	}
}

void print_liveness() {
	dprintf_liveness("\n>>>>>>>>>>>> Liveness intervals <<<<<<<<<<<\n");
	for (int i=0; i<var_cnt; i++) {
		dprintf_liveness("var (%d: %d): liveness [%d, %d)\n",
		 varTab[i]->var_type, varTab[i]->val, varTab[i]->liveBegin, varTab[i]->liveEnd);
	}
}

void print_blocks() {
	assert(blockEntry.size() == block_cnt);
	assert(blockExit.size() == block_cnt);
	dprintf_liveness("\n>>>>>>>>>>>>>>> Basic blocks <<<<<<<<<<<<<<\n");
	for (int i=0; i<block_cnt; i++) {
		dprintf_liveness("Block #%d: [%d, %d]\n", i, blockEntry[i], blockExit[i]);
	}
}