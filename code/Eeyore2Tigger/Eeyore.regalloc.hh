/* Register allocation corresponding to Eeyore grammar */
/* Author = Jiajun Tang, 1500011776, CS, EECS, PKU */

#ifndef __EEYORE_REGALLOC_H__
#define __EEYORE_REGALLOC_H__


#ifdef DEBUGREG
#define dprintf_reg(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define dprintf_reg(fmt, ...)
#endif

#define REG_S 1
#define REG_T 13
#define REG_A 20
#define REG_MIN 2  // `x0` fixed, leave `s0` for Tigger2RISCV's using and temp usage
#define REG_MAX 28 // not including

#define REG_CONST 0
#define REG_VAR 1
#define REG_TEMP 2
#define REG_COPY 3 // for passing values between procedures

#include <bitset>

#include "Eeyore.typedef.hh"
#include "Eeyore.liveness.hh"

using namespace std;

// NOTE: these indexes is not in consistence with the RISC-V spec for convenience
const char idx2reg[32][8] = {
	"x0",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
}; 
// from id to ABI name
// 0: x0; 1~12: a0-a11, callee-saved; 13~20: t0-t7, caller-saved; 21~27: a0-a7, caller-saved 
struct Reg {
	bool alloced;
	bool dirty;
	int type;
	int val;
	int last_ass;
	int live_end; // (not including)

	Reg() {
		this->alloced = false;
		this->dirty = false;
		this->type = REG_CONST;
		this->val = 0;
		this->last_ass = -1;
		this->live_end = -1;
	}
};

inline void _recalc_freecnt();
void init_registers();
void expire(int reg_idx);
inline void _spill_register(int reg_idx, int expr_idx);
void spill_register(int expr_idx);
inline void _sync(int reg_idx, int expr_idx);
inline void bind_const_register(int reg_idx, int val, int expr_idx);
int alloc_const_register(int val, int expr_idx);
inline void bind_temp_register(int reg_idx, int expr_idx);
int alloc_temp_register(int expr_idx);
void bind_register(int reg_idx, Rval rval, int expr_idx, bool var_to_reg=true, bool save_old=true, bool load_new=true, bool copy_dup=false);
int alloc_register(Rval rval, int expr_idx, bool load_new=true);
void enter_block(int expr_idx);
void exit_block(int expr_idx, bool save_local=true, bool sync=true);
inline void _print_register(int reg_idx);
void print_registers();
inline void _print_variables(int var_idx);
void print_variables();


#endif