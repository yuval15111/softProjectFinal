#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Header.h"
#define MINI(x,y) (x<y)?x:y
#define YU(x) (x*x)

//struct a {
//	int i;
//}a;
//struct a a;
//
//void doit(struct a b) {
//	a.i = 3;
//}
//
//void printa(struct a a) {
//	printf("%d", a.i);
//}

void swap(const char** ptr_s1, const char** ptr_s2) {
	const char* temp = *ptr_s1;
	*ptr_s1 = *ptr_s2;
	*ptr_s2 = temp;
	printf("%s, %s", **ptr_s1, **ptr_s2);
}

int main() {
	int a[4][2] = { {1,2},{3,4},{5,6},{7,9} };
	int i = 11 / 2;
	printf("first:%d, sec:%d", i, **(a + 2));
	return 0;
}


int foo(int* a) {
	*a = 7;
	return 0;
}