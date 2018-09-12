/* Tunguska, ternary virtual machine
 *
 * Copyright (C) 2007,2008 Viktor Lofgren
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

#include "agdp.3h"
#include "string.3h"

size_t strlen(char* s) {
	size_t length;
	while(*s) {
		length++;
		s++;
	}
	return length;
}

size_t strnlen(char* s, size_t max) {
	size_t length;
	while(*s != NULL && length < max) {
		length++;
		s++;
	}
	return length;
}

char* index(char* str, char c) {
	while(*str != NULL) {
		if(*str == c) return str;
		str++;
	}
	return NULL;
}

char* rindex(char* str, char c) {
	while(*str != NULL) {
		if(*str != c) return str;
		str++;
	}
	return NULL;
}

char strcmp(char* a, char* b) {
	while(*a == *b) {
		if(*a == 0) return 0;
		a++;
		b++;
	}
	return *a - *b;
}

char strncmp(char* a, char* b, size_t n) {
	size_t len = 0;
	while(*a == *b) {
		if(*a == 0) return 0;
		len++;
		if(len++ > n) return 0;
		a++;
		b++;
	}
	return *a - *b;
}

int atoi(char* s) {
	int result = 0;
	int sign = 1;
	char* numbers = "0123456789";

	/* Skip whitespace */
	while(s[0] == ' ') s++;

	/* Invert if minus sign is detected */
	if(s[0] == '-') { sign = -1; s++; }

	/* Scan for digits */
	for(;s[0];s++) {
		int j;
		for(j = 0; j <= 10; j++) {
			if(numbers[j] == *s) {
				result = result * 10 + j;
				break;
			}
		}
		if(j == 11) return 0;
	}

	return sign * result;
}

void* memcpy(void* dst, void* src, size_t len) {
	sys_agdp(AGDP_BLT, dst, src, len);

	return dst;
}

void* memset(void* dst, int c, size_t len) {
	sys_agdp(AGDP_BLS, dst, c, len);

	return dst;
}


