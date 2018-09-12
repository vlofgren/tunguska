#include "type.h"
#include "variable.h"
#ifndef tables_h
#define tables_h
class symbol_table_entry {
	public:
		~symbol_table_entry() {}// delete[] ename; }
		symbol_table_entry(const char* ename, int offset, variable* v) {
			this->ename = ename;
			this->offset = offset;
			this->var = v;
			this->typ = var->get_type();
		}

		const char* effective_name() const { return ename; };
		const type* get_type() const { return typ; };
		int get_offset() const { return offset; }
		variable* get_variable() const { return var; }
	private:
		const char* ename;
		const type* typ;
		variable* var;
		int offset;
};


class symbol_table {
public:
	static const symbol_table_entry* lookup(const char* name);
private:
	
};

#endif
