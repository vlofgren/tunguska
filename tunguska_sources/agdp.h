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

#include "machine.h"
#include "tryte.h"


#ifndef agdp_h
#define agdp_h

#define TFP_SMAX 3280
#define TFP_EMAX 40
#define TFP_INF 40

/* Ternary floating point
 * 
 * MST	   LST  |  MST     LST
 * E E E S S S  |  S S S S S S
 *
 * E: Exponent [-40, 40]
 * S: Significand [-3280, 3280]
 *
 * X = S * 10^E
 * */

enum {
	COP_NOOP = 0,

	COP_ITOF,	/* Integer -> float */
	COP_FTOI,	/* Float   -> integer */

	COP_FADD,	/* Float add */
	COP_FMUL,	/* Float mul */
	COP_FDIV,	/* Float div */
	COP_FEXP,	/* Float exponential function */
	COP_FLOG,	/* Float logarithm */
	COP_FCOS,	/* Float cos */
	COP_FSIN,	/* Float sin */ 

	COP_BLT,	/* Block transfer (not page bound) */
	COP_BLS,	/* Block set      (not page bound) */
	COP_BLA,	/* Block AND */
	COP_BLX,	/* Block EOR */
	COP_BLO,	/* Block OR  */
	COP_BSH,	/* Block SHIFT */
	COP_BLP,	/* Block PERMUTE */

	COP_WHEN,
	COP_IDIVW,	/* 12 trit division */
	COP_IMODW,	/* 12 trit modulus */

};

class agdp{
	public: 
		agdp();
		virtual ~agdp() {};

		void heartbeat(machine& m);

		/* Convert binary float to ternary float */
		static void float2_to_float3(float f, tryte& a, 
				tryte& b);
		/* Convert ternary float to binary float */
		static float float3_to_float2(const tryte& a, const tryte& b);
	private:
		void itof(machine& m);
		void ftoi(machine& m);
		void fadd(machine& m);
		void fmul(machine& m);
		void fdiv(machine& m);
		void fexp(machine& m);
		void flog(machine& m);
		void fcos(machine& m);
		void fsin(machine& m); 

		void blt(machine& m);
		void bls(machine& m);
		void bla(machine& m);
		void blx(machine& m);
		void blo(machine& m);
		void bsh(machine& m);
		void blp(machine& m);

		void when(machine& m);

		void divw(machine& m);
		void modw(machine& m);
		int M_OP, M_R1, M_R2, M_R3;
};

class agdp_change_hook : public tryte::change_hook {
	public:
		agdp_change_hook(agdp& d, machine& m) : agd(d), mach(m), recursion_lock(false) {}
		virtual ~agdp_change_hook() {}

		virtual const tryte& change(tryte& self, const tryte& value) { 
			if(recursion_lock) return value;
			recursion_lock = true;
			agd.heartbeat(mach);
			recursion_lock = false;
			return value;
		}
	private:
		agdp& agd;
		machine& mach;
		double recursion_lock;

};


#endif
