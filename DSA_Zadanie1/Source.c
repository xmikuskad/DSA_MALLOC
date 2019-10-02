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
	//POUZIVANIE -1 NAMIESTO NULL

	*(start + 4 * sizeof(int)) = sizeof(int);	//Smernik na predchadzajucu volnu pamat
	//zatial nepotrebne
}


void *memory_alloc(unsigned int size)
{
	if (size < 8) size = 8; //Zarovnanie najmenej na 8 bytes;

	//Ak nemame ziadny volny blok - return
	if(*(start + sizeof(int)) == -1)
		return NULL;

	int num = *(start + sizeof(int));
	int tmp = sizeof(int); //ukazovatel na posledny odkaz NEXT


	do
	{
		if (*(start + num - sizeof(int)) >= size && *(start + num - sizeof(int)) > 0)
		{ //Ak miesto vyhovuje
			//TODO
			
			//Vytvorenie novej particie ak zostalo viac ako 12bytes (4B hlavicka, 8B data) a nepresahujeme danu pamat
			if (*(start + num - sizeof(int)) - size >= 12 && (num + size + sizeof(int)) <=50)
			{
				//Nastavenie velkosti noveho bloku
				*(start + num + size) = *(start + num - sizeof(int)) - (size + sizeof(int));

				//Zmensenie aktualnej velkosti
				*(start + num - sizeof(int)) = size + sizeof(int);

				//Zapisanie NEXT
				*(start + num + size + sizeof(int)) = *(start + num);
				//Zapisanie BEFORE
				*(start + num + size + 2 * sizeof(int)) = tmp;

				//Prepisanie NEXT minuleho bloku
				*(start + tmp) = num + size + sizeof(int);

			}
			else
			{
  				*(start + tmp) = *(start + num); //Prepisanie NEXT na dalsi volny blok;
				if (tmp != sizeof(int)) //Pokial nie sme na zaciatku (tam nie je BEFORE)
				{
					*(start + num + sizeof(int)) = *(start + tmp + sizeof(int));
				}
			}

			//Nastavenie bitu na 1 - OBSADENY
			printf("Old value %d\n", *(start + num - sizeof(int)));
			*(start + num - sizeof(int)) |= 1 << 31;
			printf("New value %d\n", *(start + num - sizeof(int)));


			return (void*)(start + num);
		}
		else
		{
			tmp = num;
			num += *(start + num - sizeof(int));
		}
	}
	while (*(start + num) != -1);

	return NULL;

}

void testik(char* p)
{
	printf("Vzdialenost %d\n", p-(char*)start);
}

int main()
{
	char a[50];
	char* test, test2, test3,test4;

	memory_init(a, 50);

	test = (char*)memory_alloc(8);

	if (test != NULL)
	{
		printf("Alokacia test1 uspesna\n\n");
		//testik(test);
	}
	else
	{
		printf("Alokacia test1 neuspesna\n\n");
	}
	//test -= sizeof(int);

	/*int pokus = 1;
	printf("Pokus %d\n", pokus << 30);
	printf("Pokus %d\n", pokus << 31);*/

	//printf("NAJDENY ALLOC: %d\n",*test);
	//testik(test);

	test2 = (char*)memory_alloc(8);
	if (test2 != NULL)
	{
		printf("Alokacia test2 uspesna\n\n");
		//testik(test2);
	}
	else
	{
		printf("Alokacia test2 neuspesna\n\n");
	}

	test3 = (char*)memory_alloc(8);
	if (test3 != NULL)
	{
		printf("Alokacia test3 uspesna\n\n");
		//testik(test3);
	}
	else
	{
		printf("Alokacia test3 neuspesna\n\n");
	}

	test4 = (char*)memory_alloc(8);
	if (test4 != NULL)
	{
		printf("Alokacia test4 uspesna\n\n");
		//testik(test3);
	}
	else
	{
		printf("Alokacia test4 neuspesna\n\n");
	}

	//printf("%d", ((num+1)>>1)<<1);

	getchar();
	getchar();
	return 0;
}