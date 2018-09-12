/* Tunguska, ternary virtual machine
 *
 * Copyright (C) 2007-2009 Viktor Lofgren
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef trit_h
#define trit_h

typedef int tval_t;
class trit {
public:
	enum {
		TRFALSE = -1, TRMU = 0, TRTRUE = 1, TRINVALID = -2
	}; 


	trit(register int data) { this->data = data; }
	trit(register const trit& t) { data = t.data; }
	trit() { this->data = TRMU; };

	trit& operator=(const trit& t);
	trit& operator=(int i);
	int operator&(const trit& t) const;
	int operator|(const trit& t) const;
	int operator^(const trit& t) const;
	int operator!() const;

	trit& operator&=(const trit& t);
	trit& operator|=(const trit& t);
	trit& operator^=(const trit& t);

	int operator>(const trit& t) const;
	int operator>>(const trit& t) const;
	int but(const trit& t) const;
	trit& but_this(const trit& t);
	int comm_op(int op, const trit& t) const;
	trit& comm_op_this(int op, const trit& t);
	int operator==(const trit& t) const;
	int operator!=(const trit& t) const;

	int to_int() const { return data; }

	class trit_strategy;
	static void set_strategy(const trit_strategy* t) { delete strategy; strategy = t; }


	/* Trit strategy, this basically determines what operations
	 * do. While Tunguska only uses True-Unknown-False logic,
	 * it is theoretically possible to add other ternary logic 
	 * systems, say True-Neither-False by adding another strategy. */
	class trit_strategy {
		public:
			virtual ~trit_strategy() {}
			virtual tval_t op_inv(tval_t a) const = 0;
			virtual tval_t op_and(tval_t a, tval_t b) const = 0;
			virtual tval_t op_or(tval_t a, tval_t b) const = 0;
			virtual tval_t op_xor(tval_t a, tval_t b) const = 0;
			virtual tval_t op_tsh(tval_t a, tval_t b) const = 0;
			virtual tval_t op_rol(tval_t a, tval_t b) const = 0;
			virtual tval_t op_but(tval_t a, tval_t b) const = 0;
	};

	/* Calculate on the fly */
	class trit_strategy_tuf_calculate : public trit_strategy {
		public:
			virtual tval_t op_inv(tval_t a) const;
			virtual tval_t op_and(tval_t a, tval_t b) const;
			virtual tval_t op_or(tval_t a, tval_t b) const;
			virtual tval_t op_xor(tval_t a, tval_t b) const;
			virtual tval_t op_tsh(tval_t a, tval_t b) const;
			virtual tval_t op_rol(tval_t a, tval_t b) const;
			virtual tval_t op_but(tval_t a, tval_t b) const;
	};

	/* Use cached tuf_calculate values */
	class trit_strategy_tuf_table : public trit_strategy {
		public:
			trit_strategy_tuf_table() {
				trit_strategy_tuf_calculate cal;
				for(int a = -1; a < 2; a++) {
					inv_table[a+1] = cal.op_inv(a);
	
					for(int b = -1; b < 2; b++) {
						and_table[a+1][b+1] = cal.op_and(a,b);
						or_table[a+1][b+1] = cal.op_or(a,b);
						xor_table[a+1][b+1] = cal.op_xor(a,b);
						tsh_table[a+1][b+1] = cal.op_tsh(a,b);
						rol_table[a+1][b+1] = cal.op_rol(a,b);
						but_table[a+1][b+1] = cal.op_but(a,b);
					}
				}
			}
			virtual tval_t op_inv(tval_t a) const { return inv_table[a+1]; }
			virtual tval_t op_and(tval_t a, tval_t b) const { return and_table[a+1][b+1]; }
			virtual tval_t op_or(tval_t a, tval_t b) const { return or_table[a+1][b+1]; }
			virtual tval_t op_xor(tval_t a, tval_t b) const { return xor_table[a+1][b+1]; }
			virtual tval_t op_tsh(tval_t a, tval_t b) const { return tsh_table[a+1][b+1]; }
			virtual tval_t op_rol(tval_t a, tval_t b) const { return rol_table[a+1][b+1]; }
			virtual tval_t op_but(tval_t a, tval_t b) const { return but_table[a+1][b+1]; }
		private:
			tval_t inv_table[3];
			tval_t and_table[3][3];
			tval_t or_table[3][3];
			tval_t xor_table[3][3];
			tval_t tsh_table[3][3];
			tval_t rol_table[3][3];
			tval_t but_table[3][3];
	};
protected:
	int data : 2;
	static const trit_strategy* strategy;
};

#endif
