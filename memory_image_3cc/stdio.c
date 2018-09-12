#include "stdio.3h"
#include "string.3h"
#include "system.3h"

/* Scroll the screen down one line
 *
 */
void scroll() {
	memcpy(SYS_SCREEN, 0nDDBD2D, SYS_SCREEN_END - 0nDDBD2D);
	memset(SYS_SCREEN_END - 54, 0, 54);
}

void putc(char c) {
	static char col = 0;
	static char row = 0;
	if(c == '\n') {
		col = 0;
		if(++row > 26) {
			row = 26; 
			scroll();
		}
		return;
	} else if(col >= 54) {
		col = 0;
		if(++row > 26) {
			row = 26;
			scroll();
		}
	}

	sys_text_buffer[row][col] = c;
	col++;
}

void puts(char* s) {
	for(;(*s) != 0; s++) putc(*s);
}

/* Print a decimal integer */
void putnum(int i) {
	char* numbers = "0123456789";
	int reverse = 0;
	int length = 0;

	if(i == 0) {
		putc('0');
		return;
	} else if(i < 0) {
		putc('-');
		i = -i;
	}

	/* Reverse the number (123 becomes 321) */
	while(i) {
		int j = i % 10;
		reverse = 10 * reverse + j;
		length++; // Keep a track of length, since 100 reversed is 0*100 + 0*10 + 1 = 1
		i = i / 10;
	}

	/* Print the number */
	while(length--) {
		int j = reverse % 10;
		putc(numbers[j]);
		reverse = reverse / 10;
	}
}

/* Print a decimal integer stored in char (significatly faster) */
void putsmallnum(char i) {
	char* numbers = "0123456789";
	char reverse = 0;
	char length = 0;

	if(i == 0) {
		putc('0');
		return;
	} else if(i < 0) {
		putc('-');
		i = -i;
	}

	/* Reverse the number (123 becomes 321) */
	while(i) {
		char j = i % 10;
		reverse = 10 * reverse + j;
		length++; // Keep a track of length, since 100 reversed is 0*100 + 0*10 + 1 = 1
		i = i / 10;
	}

	/* Print the number */
	while(length--) {
		char j = reverse % 10;
		putc(numbers[j]);
		reverse = reverse / 10;
	}
}
char getc_noblock() {
	struct machine_status* ms = get_ms();
	char c;
	if(ms->key == 0) return 0;

	c = ms->key;
	ms->key = 0;

	if(ms->echo_input) putc(c);

	return c;
}
char getc() {
	struct machine_status* ms = get_ms();
	char c;
	while(ms->key == 0);

	c = ms->key;
	ms->key = 0;

	if(ms->echo_input) putc(c);

	return c;
}

int gets(char* buffer, int bufsiz) {
	int index;
	for(index = 0; index < bufsiz; index++) {
		char c = getc();
		buffer[index] = c;
		if(c == 0) return index;
		if(c == 2) {
			buffer[index] = 0;
			return index;
		}
	}
	buffer[index-1] = 0;
	return index;
}


