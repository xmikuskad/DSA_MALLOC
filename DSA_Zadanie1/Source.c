#include <stdio.h>
#include <stdlib.h>

int* start; 
//mozeme pouzivat char nie len void

void memory_init(void *ptr, unsigned int size)
{
	start = (int*)ptr;

	*start = size;	//Zapisanie velkosti do prvy 4 bytes
	
	*(start + sizeof(int)) = 3*sizeof(int);	//Smernik na prvu volnu velkost
	
	*(start + 2*sizeof(int)) = size-2*sizeof(int);	//Zapisanie velkosti prvej volnej pamati
	
	*(start + 3*sizeof(int)) = -1;	//Smernik na druhu volnu velkost

}

void* FindPlace(int num, int size)
{	
	if (*(start + num - sizeof(int)) >= size)
	{ //Ak miesto vyhovuje
		printf("CISLO: %d\n", *(start + num-4));
		return (void*)(start + num);
	}
	else //Ak je miesto moc male
	{
		return FindPlace(num + *(start + num - sizeof(int)), size);
	}

	//Pokial nenajde volne miesto
	if (*(start + num) == -1)
		return NULL;


}

void *memory_alloc(unsigned int size)
{

	return FindPlace(*(start+sizeof(int)), size);

}

int main()
{
	int num = 515;
	char a[50];

	memory_init(a, 50);

	int *test = (char*)memory_alloc(20);
	test -= sizeof(int);

	printf("NAJDENY ALLOC: %d\n",*test);

	//printf("%d", ((num+1)>>1)<<1);

	getchar();
	getchar();
	return 0;
}