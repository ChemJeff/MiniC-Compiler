/* Register allocation corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#include <assert.h>

#include "Eeyore.typedef.hh"
#include "Eeyore.regalloc.hh"

extern Expr* exprTab[MAX_EXPRS];
extern Var* varTab[MAX_VARS];
extern Istr* istrTab[MAX_ISTRS];

extern bitset<MAX_VARS> livenessTab[MAX_EXPRS + 1];

extern int var2idx[3][MAX_VARS];
extern int expr_cnt;
extern int var_cnt;
extern int istr_cnt;

Reg registers[32];
int free_reg_cnt = 26;
int func_stack_size = 0;

using namespace std;

inline void _recalc_freecnt() {
	dprintf_reg("in function: _recalc_freecnt\n");
	free_reg_cnt = 0;
	// print_registers();
	for (int i=REG_MIN; i<REG_MAX; i++) {
		if (!registers[i].alloced) {
			free_reg_cnt++;
		}
	}
	dprintf_reg("    free_reg_cnt = %d\n",free_reg_cnt);
}

// initialize all registers but x0 as `clean` registers
void init_registers() {
	dprintf_reg("in function: init_registers\n");
	for (int i=0; i<REG_MAX; i++) {
		registers[i].alloced = false;
		registers[i].dirty = false;
		// registers[i].assigned_cnt = 0;
		registers[i].type = REG_CONST;
		registers[i].val = 0;
		registers[i].last_ass = -1;
		registers[i].live_end = -1;
	}
	registers[0].alloced = true; // x0=0 is fixed
	registers[REG_S].alloced = true; // s0 is reserved for Tigger2RISCV and temp
	dprintf_reg("\n>>>>>>>>>> Registers initialized <<<<<<<<<<\n");
	_recalc_freecnt();
}

// expire a reg into a `clean` state WITHOUT any sync
void expire(int reg_idx) {
	dprintf_reg("in function: expire\n");
	assert(reg_idx >= REG_MIN && reg_idx < REG_MAX);
	if (registers[reg_idx].alloced) free_reg_cnt++;
	if (registers[reg_idx].type == REG_VAR) {
		Var* varEntry = varTab[registers[reg_idx].val];
		dprintf_reg("    expiring var{%d, %d}, isGlobal = %d, isArray = %d, allocedStatus = %d, allocedStk = %d\n",
				varEntry->var_type, varEntry->val, varEntry->isGlobal, varEntry->isArray, varEntry->allocedStatus, varEntry->allocedStk);
		if (varEntry->isGlobal) {
			varEntry->allocedStatus = ALLOC_STK;
		}
		else {
			varEntry->allocedStatus = ALLOC_NONE;
		}
	}
	registers[reg_idx].alloced = false;
	registers[reg_idx].dirty = false;
	registers[reg_idx].type = REG_CONST;
	registers[reg_idx].val = 0;
}

inline void _spill_register(int reg_idx, int expr_idx) {
	// spill a specified reg
	// NOTE: this func is modified that can be called externally
	dprintf_reg("in function: _spill_register\n");
	// spill a live local variable needs extra operations
	assert(reg_idx >= REG_MIN && reg_idx < REG_MAX);
	assert(registers[reg_idx].alloced == true);
	if (registers[reg_idx].type == REG_VAR) {
		Var* varEntry = varTab[registers[reg_idx].val];
		// NOTE: always sync global variables when spilled
		if (registers[reg_idx].live_end <= expr_idx && !varEntry->isGlobal) {
			expire(reg_idx);
			return;
		}
		dprintf_reg("    spilling var{%d, %d}, isGlobal = %d, isArray = %d, allocedStatus = %d, allocedStk = %d\n",
				varEntry->var_type, varEntry->val, varEntry->isGlobal, varEntry->isArray, varEntry->allocedStatus, varEntry->allocedStk);
		assert(varEntry->allocedStatus == ALLOC_REG);
		varEntry->allocedStatus = ALLOC_NONE;
		varEntry->assignedReg = -1;
		if (varEntry->isGlobal && !varEntry->isArray) {
			// NOTE: only if dirty
			// save global variables: loadaddr vx addr_reg; addr_reg[0] = reg[reg_idx]
			if (registers[reg_idx].dirty) {
				// NOTE: should not alloc the same reg to be spilled as temp reg
				istrTab[istr_cnt++] = new Istr_LoadAddr(REG_S, true, varEntry->globalID);
				istrTab[istr_cnt++] = new Istr_Assign(REG_S, true, reg_idx, 0, ASS_ARRAY_LEFT);
			}
			varEntry->allocedStatus = ALLOC_NONE;
		}
		else if (!varEntry->isGlobal && !varEntry->isArray) {
			// NOTE: only if dirty
			// save live local variables: push into stack
			if (registers[reg_idx].dirty) {
				istrTab[istr_cnt++] = new Istr_Store(reg_idx, varEntry->allocedStk);
			}
			varEntry->allocedStatus = ALLOC_STK;
		}
		free_reg_cnt++;
		registers[reg_idx].alloced = false;
		registers[reg_idx].dirty = false;
		registers[reg_idx].type = REG_CONST;
		registers[reg_idx].val = 0;
	}
	else { // VAR_CONST , VAR_TEMP or (unused) VAR_COPY
		expire(reg_idx);
		return;
	}
}

void spill_register(int expr_idx) {
	dprintf_reg("in function: spill_register\n");
	int longestEnd = -1;
	int longest_idx = -1;
	// not spilling ax regs as possible as can be done
	for (int i=REG_T; i<REG_T+7; i++) {
		if (!registers[i].alloced) {
			continue;
		}
		if (registers[i].last_ass == expr_idx) {
			continue;
		}
		else if (registers[i].live_end <= expr_idx) {
			_spill_register(i, expr_idx);
			return;
		}
		else {
			if (registers[i].live_end > longestEnd) {
				longestEnd = registers[i].live_end;
				longest_idx = i;
			}
		}
	}
	for (int i=REG_S+1; i<REG_S+12; i++) {
		if (!registers[i].alloced) {
			continue;
		}
		if (registers[i].last_ass == expr_idx) {
			continue;
		}
		else if (registers[i].live_end <= expr_idx) {
			_spill_register(i, expr_idx);
			return;
		}
		else {
			if (registers[i].live_end > longestEnd) {
				longestEnd = registers[i].live_end;
				longest_idx = i;
			}
		}
	}
	for (int i=REG_A; i<REG_A+8; i++) {
		if (!registers[i].alloced) {
			continue;
		}
		if (registers[i].last_ass == expr_idx) {
			continue;
		}
		else if (registers[i].live_end <= expr_idx) {
			_spill_register(i, expr_idx);
			return;
		}
		else {
			if (registers[i].live_end > longestEnd) {
				longestEnd = registers[i].live_end;
				longest_idx = i;
			}
		}
	}
	if (!(longest_idx >= REG_MIN && longest_idx < REG_MAX && longestEnd >=0))
		print_registers();
	assert(longest_idx >= REG_MIN && longest_idx < REG_MAX && longestEnd >=0);
	_spill_register(longest_idx, expr_idx);
}

inline void _sync(int reg_idx, int expr_idx) {
	dprintf_reg("in function: _sync\n");
	assert(reg_idx >= REG_MIN && reg_idx < REG_MAX);
	assert(registers[reg_idx].alloced == true);
	assert(registers[reg_idx].type == REG_VAR);
	if (registers[reg_idx].live_end <= expr_idx) {
		expire(reg_idx);
		return;
	}
	Var* varEntry = varTab[registers[reg_idx].val];
	dprintf_reg("    syncing var{%d, %d}, isGlobal = %d, isArray = %d, allocedStatus = %d, allocedStk = %d\n",
		varEntry->var_type, varEntry->val, varEntry->isGlobal, varEntry->isArray, varEntry->allocedStatus, varEntry->allocedStk);
	assert(varEntry->allocedStatus == ALLOC_REG);
	if (varEntry->isGlobal && !varEntry->isArray) {
		// NOTE: only if dirty
		// save global variables: loadaddr vx addr_reg; addr_reg[0] = reg[reg_idx]
		if (registers[reg_idx].dirty) {
			// NOTE: should not alloc the same reg to be spilled as temp reg
			istrTab[istr_cnt++] = new Istr_LoadAddr(REG_S, true, varEntry->globalID);
			istrTab[istr_cnt++] = new Istr_Assign(REG_S, true, reg_idx, 0, ASS_ARRAY_LEFT);
		}
	}
	else if (!varEntry->isGlobal && !varEntry->isArray) {
		// NOTE: only if dirty
		// save live local variables: push into stack
		if (registers[reg_idx].dirty) {
			istrTab[istr_cnt++] = new Istr_Store(reg_idx, varEntry->allocedStk);
		}
	}
}

inline void bind_const_register(int reg_idx, int val, int expr_idx) {
	// load immediate into regs
	// NOTE: this should never be called externally
	assert(reg_idx >= REG_MIN && reg_idx < REG_MAX);
	if (!registers[reg_idx].alloced) free_reg_cnt--;
	istrTab[istr_cnt++] = new Istr_Assign(reg_idx, false, val, 0, ASS_NO_ARRAY);
	registers[reg_idx].alloced = true;
	registers[reg_idx].dirty = false;
	registers[reg_idx].type = REG_CONST;
	registers[reg_idx].val = val;
	registers[reg_idx].last_ass = expr_idx;
	registers[reg_idx].live_end = expr_idx + 1;
}

int alloc_const_register(int val, int expr_idx) {
	dprintf_reg("in function: alloc_const_register\n");
	if (val == 0) return 0; // x0===0
	if (free_reg_cnt < 0 || free_reg_cnt > 26) _recalc_freecnt();
	dprintf_reg("    free_reg_cnt=%d\n", free_reg_cnt);

	if (free_reg_cnt == 0) {
		spill_register(expr_idx);
	}
	// never reuse even if the value is already in some register
	// not assigning ax regs as possible as can be done
	for (int i=REG_T; i<REG_T+7; i++) {
		if (!registers[i].alloced) {
			bind_const_register(i, val, expr_idx);
			return i;
		}
	}
	for (int i=REG_S+1; i<REG_S+12; i++) {
		if (!registers[i].alloced) {
			bind_const_register(i, val, expr_idx);
			return i;
		}
	}
	for (int i=REG_A; i<REG_A+8; i++) {
		if (!registers[i].alloced) {
			bind_const_register(i, val, expr_idx);
			return i;
		}
	}
	print_registers();
	assert(0); // should never run into this line
	return -1; // assignment fail
}

inline void bind_temp_register(int reg_idx, int expr_idx) {
	// NOTE: this should never be called externally
	assert(reg_idx >= REG_MIN && reg_idx < REG_MAX);
	if (!registers[reg_idx].alloced) free_reg_cnt--;
	registers[reg_idx].alloced = true;
	registers[reg_idx].dirty = false;
	registers[reg_idx].type = REG_TEMP;
	registers[reg_idx].last_ass = expr_idx;
	registers[reg_idx].live_end = expr_idx + 1;
}

int alloc_temp_register(int expr_idx) {
	dprintf_reg("in function: alloc_temp_register\n");
	if (free_reg_cnt < 0 || free_reg_cnt > 26) _recalc_freecnt();
	dprintf_reg("    free_reg_cnt=%d\n", free_reg_cnt);
	if (free_reg_cnt == 0) {
		spill_register(expr_idx);
	}
	// never reuse even if the value is already in some register (difficult to check)
	// not assigning ax regs as possible as can be done
	for (int i=REG_T; i<REG_T+7; i++) {
		if (!registers[i].alloced) {
			bind_temp_register(i, expr_idx);
			return i;
		}
	}
	for (int i=REG_S+1; i<REG_S+12; i++) {
		if (!registers[i].alloced) {
			bind_temp_register(i, expr_idx);
			return i;
		}
	}
	for (int i=REG_A; i<REG_A+8; i++) {
		if (!registers[i].alloced) {
			bind_temp_register(i, expr_idx);
			return i;
		}
	}
	return REG_S; // anyway, s0 is reserved for temp usage
}

void bind_register(int reg_idx, Rval rval, int expr_idx
	, bool var_to_reg, bool save_old, bool load_new, bool copy_dup) {
	dprintf_reg("in function: bind_register\n");
	dprintf_reg("    var_to_reg=%d, save_old=%d, load_new=%d, copy_dup=%d\n", var_to_reg, save_old, load_new, copy_dup);
	// default values: var_to_reg=true, save_old=true, load_new=true
	// NOTE: this may spill previously assigned variable in the specified register
	assert(reg_idx >= REG_MIN && reg_idx < REG_MAX);
	int _type = (rval.var_type == VAR_IMMEDIATE)?0:1;
	int _val = (_type == 0)?rval.val:(var2idx[rval.var_type - VAR_NATIVE][rval.val]);
	if (registers[reg_idx].alloced) {
		if (save_old) _spill_register(reg_idx, expr_idx);
		else expire(reg_idx);
	}
	free_reg_cnt--;
	registers[reg_idx].alloced = true;
	if (!copy_dup) {
		registers[reg_idx].type = _type;
		registers[reg_idx].val = _val;
	}
	else {
		registers[reg_idx].type = REG_COPY;
		// registers[reg_idx].val = expr_idx;
	}
	registers[reg_idx].last_ass = expr_idx;
	if (var_to_reg) {
		// inserting load/loadaddr istrs
		if (_type == REG_VAR) {
			Var* varEntry = varTab[_val];
			dprintf_reg("    binding var{%d, %d}, isGlobal = %d, isArray = %d, allocedStatus = %d, allocedStk = %d\n",
				varEntry->var_type, varEntry->val, varEntry->isGlobal, varEntry->isArray, varEntry->allocedStatus, varEntry->allocedStk);
			if (varEntry->allocedStatus == ALLOC_REG) { // copy to another register
				dprintf_reg("    already in reg %d, assign to reg %d\n", varEntry->assignedReg, reg_idx);
				if (copy_dup && reg_idx != varEntry->assignedReg) 
					istrTab[istr_cnt++] = new Istr_Assign(reg_idx, true, varEntry->assignedReg, 0, ASS_NO_ARRAY);
				// one variable can only be mapped to one register
				else return; // erronous combination
			}
			else if (varEntry->isGlobal) {
				if (varEntry->isArray) { // global array
					istrTab[istr_cnt++] = new Istr_LoadAddr(reg_idx, true, varEntry->globalID);
				}	
				else { // global variable
					if (load_new) istrTab[istr_cnt++] = new Istr_Load(reg_idx, true, varEntry->globalID);
				}
			}
			else {
				if (varEntry->isArray) { // local array
					if (varEntry->allocedStk < 0) {
						// alloc stk space for loacl array the first time
						istrTab[istr_cnt++] = new Istr_LoadAddr(reg_idx, false, func_stack_size);
						varEntry->allocedStk = func_stack_size;
						func_stack_size += varEntry->arraySize / 4;
					}
					else {
						istrTab[istr_cnt++] = new Istr_LoadAddr(reg_idx, false, varEntry->allocedStk);
					}
				}
				else { // local variable
					if (load_new && varEntry->allocedStatus == ALLOC_STK) {
						istrTab[istr_cnt++] = new Istr_Load(reg_idx, false, varEntry->allocedStk);
					}
					// no need to add explicit istr for uninitialized/unused local variables 
				}
			}
			if (!copy_dup) {
				varEntry->allocedStatus = ALLOC_REG;
				varEntry->assignedReg = reg_idx;
				registers[reg_idx].live_end = varEntry->liveEnd;
			}
			else {
				registers[reg_idx].live_end = expr_idx + 1;
			}
		}
		else {
			// load immediate into regs
			istrTab[istr_cnt++] = new Istr_Assign(reg_idx, false, _val, 0, ASS_NO_ARRAY);
			registers[reg_idx].live_end = expr_idx + 1;
		}
	}
	else {
		// directly bind some value already in reg to some variable
		// cases such as return value to a0
		assert(_type == REG_VAR);
		Var* varEntry = varTab[_val];
		if (varEntry->allocedStatus == ALLOC_REG) { // already assigned a reg
			// discard old value in reg
			expire(varEntry->assignedReg);
		}
		varEntry->allocedStatus = ALLOC_REG;
		varEntry->assignedReg = reg_idx;
		registers[reg_idx].dirty = true;
		registers[reg_idx].live_end = varEntry->liveEnd;
	}
}

int alloc_register(Rval rval, int expr_idx, bool load_new) {
	dprintf_reg("in function: alloc_register\n");
	if (free_reg_cnt < 0 || free_reg_cnt > 26) _recalc_freecnt();
	dprintf_reg("    free_reg_cnt=%d\n", free_reg_cnt);

	int _type = (rval.var_type == VAR_IMMEDIATE)?0:1;
	int _val = (_type == 0)?rval.val:(var2idx[rval.var_type - VAR_NATIVE][rval.val]);
	// allocate const to register
	if (_type == VAR_IMMEDIATE) return alloc_const_register(_val, expr_idx);

	Var* varEntry = varTab[_val];
	// already in some registers
	if ((varEntry->allocedStatus == ALLOC_REG) 
		&& (varEntry->assignedReg > 0)) return varEntry->assignedReg;

	// need to assign a new register to this variable
	if (free_reg_cnt == 0) {
		spill_register(expr_idx);
	}
	// not assigning ax regs as possible as can be done
	for (int i=REG_T; i<REG_T+7; i++) {
		if (!registers[i].alloced) {
			bind_register(i, rval, expr_idx, true, false, load_new);
			return i;
		}
	}
	for (int i=REG_S+1; i<REG_S+12; i++) {
		if (!registers[i].alloced) {
			bind_register(i, rval, expr_idx, true, false, load_new);
			return i;
		}
	}
	for (int i=REG_A; i<REG_A+8; i++) {
		if (!registers[i].alloced) {
			bind_register(i, rval, expr_idx, true, false, load_new);
			return i;
		}
	}
	print_registers();
	assert(0); // should never run into this line
	return -1; // assignment fail
}

// actions taken at the entry of basic blocks
void enter_block(int expr_idx) {
	dprintf_reg("in function: enter_block\n");
	print_registers();
	print_variables();
	// NOTE: for now, clean `all memory` when entering a new block
	init_registers();
	for (int i=0; i<var_cnt; i++) {
		if (varTab[i]->liveEnd <= expr_idx && !varTab[i]->isGlobal) continue;
		if (varTab[i]->allocedStatus == ALLOC_REG && (varTab[i]->isGlobal || varTab[i]->isArray))
			varTab[i]->allocedStatus = ALLOC_NONE;
		else if (varTab[i]->allocedStatus == ALLOC_REG && !varTab[i]->isGlobal)
			varTab[i]->allocedStatus = ALLOC_STK;
		varTab[i]->assignedReg = -1;
	}
	print_registers();
	print_variables();
}

// actions taken at the exit of basic blocks
void exit_block(int expr_idx, bool save_local, bool sync) {
	dprintf_reg("in function: exit_block\n");
	print_registers();
	print_variables();
	for (int i=REG_MIN; i<REG_MAX; i++) {
		if (registers[i].alloced == true 
			&& (registers[i].type == REG_VAR)) {
			if (!save_local && !varTab[registers[i].val]->isGlobal) {
				continue;
			}
			if (sync) {
				_sync(i, expr_idx);
			}
			else /*if (registers[i].type != REG_COPY)*/ {
				_spill_register(i, expr_idx);
			}
		}
	}
	print_registers();
	print_variables();
}

inline void _print_register(int reg_idx) {
	int i = reg_idx;
	dprintf_reg("%s: alloced = %d, type = %d, val = %d, dirty = %d, last_ass = %d, live_end = %d\n",
			idx2reg[i], registers[i].alloced, registers[i].type, registers[i].val, registers[i].dirty, registers[i].last_ass, registers[i].live_end);
}

void print_registers() {
	for (int i=0; i<REG_MAX; i++) {
		_print_register(i);
	}
}

inline void _print_variables(int var_idx) {
	int i = var_idx;
	dprintf_reg("%d: var{%d, %d}, isGlobal = %d, isArray = %d, allocedStatus = %d, assignedReg = %d, allocedStk = %d\n",
		i, varTab[i]->var_type, varTab[i]->val, varTab[i]->isGlobal, varTab[i]->isArray, varTab[i]->allocedStatus, varTab[i]->assignedReg, varTab[i]->allocedStk);
}

void print_variables() {
	for (int i=0; i<var_cnt; i++) {
		_print_variables(i);
	}
}