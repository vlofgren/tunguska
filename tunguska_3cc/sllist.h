#ifndef sllist_h
#define sllist_h

/* Standard, run of the mill linked list, mostly used by the parser and
 * function-related classes */
class sllist {
	public:
		sllist(void* data, void* data2 = NULL) {
			this->data = data;
			this->data2 = data2;
			this->next = NULL;
		}

		sllist(sllist* next, void* data, void* data2 = NULL) {
			this->data = data;
			this->data2 = data2;
			this->next = next;
		}

		static sllist* reverse(sllist* l) {
			sllist* node = NULL;
			while(l) {
				node = new sllist(node, l->data, l->data2);
				l = l->next;
			}
			return node;
		}

		void* data;
		void* data2;
		sllist* next;
};

#endif
