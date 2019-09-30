#include <stdio.h>

/*
typedef struct Pages {
	int size;
	struct Page *next;

} Page;
*/

void *start;

void memory_init(void *ptr, unsigned int size)
{
	start = ptr;
	/**((short*)start) = (short)size;
	printf("%p\n", start);
	printf("%p\n", ((short*)start));
	printf("%d\n", *((short*)start));*/
}


int main()
{

	char a[50];

	memory_init(a, 50);

	int size = 20;
	int test = 1<<31;
	int *p = &size;

	int lul = *p & ~0x1;

	printf("%d\n", sizeof(int*));
	//printf("%d\n", sizeof(test));
	printf("%d", ((21+1)>>1)<<1);

	getchar();
	getchar();
	return 0;
}