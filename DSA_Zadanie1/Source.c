#include <stdio.h>
#include <stdlib.h>

int* start; 
//mozeme pouzivat char nie len void

void memory_init(void *ptr, unsigned int size)
{
	start = (int*)ptr;

	*start = size;	//Zapisanie velkosti do prvych 4 bytes
	
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


	while (*(start + num) != -1);
	{
		if (*(start + num - sizeof(int)) >= size && *(start + num - sizeof(int)) > 0)
		{ //Ak miesto vyhovuje
			//TODO
			
			//Vytvorenie novej particie ak zostalo viac ako 12bytes (4B hlavicka, 8B data) a nepresahujeme danu pamat
			if (*(start + num - sizeof(int)) - (size+sizeof(int)) >= 12 && (num + size + sizeof(int)) <= *start)
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

			printf("Vraciam smernik: %p\n", (start + num));
			return (void*)(start + num);
		}
		else
		{
			tmp = num;
			num = *(start + num - sizeof(int));
		}
	}

	return NULL;

}

int memory_free(void *valid_ptr){
	printf("Start: %p\n", start);
	printf("Smernik: %p\n", valid_ptr);
	printf("Rozdiel: %d\n\n", (int*)valid_ptr - start);

	int position = (int*)valid_ptr - start;
	int num = sizeof(int);
	//int tmp = num;
	//int flag = 1;

	//if(*(start+num))
	while (*(start + num) != -1 && (*(start + num-sizeof(int)) < position || num==sizeof(int)))
	{
		//tmp = num;
		num = *(start + num);
	}

	//Prepis prveho bitu na 0
	*((int*)valid_ptr - sizeof(int)) ^= 1 << 31; //*((int*)valid_ptr - sizeof(int))<<1

	//printf("Smernik pls: %d\n", start - (int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)) - sizeof(int)));
	//printf("%d\n", *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)) - sizeof(int))));

	//Pokus o merge s nasledujucim blokom - musi byt volny a nesmie to byt koniec
	if (*((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)))) > 0
		&& *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)))) < *start)
	{
		printf("Merge s nasledujucim blokom\n");
		//flag = 1;

		//Pozriem velkost uvolnovaneho bloku ( valid_ptr - sizeof(int) ) a nasledne k smerniku tuto velkost pricitam.
		//Ak je velkost nasledujuceho bloku <0, znamena to, ze je obsadeny a mergovat nebudeme.
		//Inak zvacsime velkost a prepiseme NEXT a BEFORE z nasledujuceho bloku
		*((int*)valid_ptr - sizeof(int)) += *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int))));
		*(int*)valid_ptr = *((int*)valid_ptr - sizeof(int) + (*(int*)valid_ptr)); //Prepisanie NEXT
		*((int*)valid_ptr + sizeof(int)) = *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr + sizeof(int)))); //Prepisanie BEFORE
	}
	else //Ak sa neda spravit merge
	{
		printf("Merge s nasledujucim blokom neprebehol\n");
		//Nastavenie NEXT
		*(int*)valid_ptr = *(start + num - sizeof(int));
		if (*(start + num) != -1)
		{
			//Zmena BEFORE iba ak nasleduje niaky blok
			*(start + num + sizeof(int)) = position;
			//Nastavenie NEXT okrem zaciatku
			*(int*)valid_ptr = *(start + num - sizeof(int));
		}
		else //Specialne pre prvy NEXT
		{
			*(int*)valid_ptr = -1;
		}
	}

	//Pokus o merge s predchadzajucim blokom
	if (num == sizeof(int)) //Ak sme na zaciatku a nemame tam velkost bloku ani BEFORE
	{
		printf("Nastavenie nextu na zaciatku\n");
		*(start + num) = position;
	}
	else //Ak mame pred sebou blok
	{
		//Pokus o merge
		if (start + num - sizeof(int) + *((start + num - sizeof(int))) == ((int*)valid_ptr-sizeof(int)))
		{
			printf("Merge na zaciatku prebehol\n");
			//flag = 1;
			//Zvacsenie velkosti
			*(start + num - sizeof(int)) += *((int*)valid_ptr - sizeof(int));
			//Prepisanie NEXT
			*(start + num) = *(int*)valid_ptr;
			//Prepisanie BEFORE
			if(*(start + num + sizeof(int)) != sizeof(int)) //Pokial neprepisujeme zaciatok (tam BEFORE nie je)
				*(start+*(start + num + sizeof(int))) = *((int*)valid_ptr+sizeof(int));

		}
		else //Ak sa neda spravit merge
		{
			printf("Merge na zaciatku neprebehol\n");
			//Nastavenie BEFORE
			*((int*)valid_ptr + sizeof(int)) = num;

			//Zmena NEXT
			*(start + num) = position;
		}
	}


	return 0;
}


int main()
{
	char a[50];
	char *test, *test2, *test3,*test4;

	memory_init(a, 50);

	test = (char*)memory_alloc(8);
	if (test != NULL)
	{
		printf("Alokacia test1 uspesna\n\n");
	}
	else
	{
		printf("Alokacia test1 neuspesna\n\n");
	}

	test2 = (char*)memory_alloc(8);
	if (test2 != NULL)
	{
		printf("Alokacia test2 uspesna\n\n");
	}
	else
	{
		printf("Alokacia test2 neuspesna\n\n");
	}

	test3 = (char*)memory_alloc(8);
	if (test3 != NULL)
	{
		printf("Alokacia test3 uspesna\n\n");
	}
	else
	{
		printf("Alokacia test3 neuspesna\n\n");
	}

	test4 = (char*)memory_alloc(8);
	if (test4 != NULL)
	{
		printf("Alokacia test4 uspesna\n\n");
	}
	else
	{
		printf("Alokacia test4 neuspesna\n\n");
	}

	if (!memory_free(test))
	{
		printf("Uvolnenie test sa podarilo\n\n");
	}
	else
	{
		printf("Uvolnenie test sa nepodarilo\n\n");
	}

	if (!memory_free(test2))
	{
		printf("Uvolnenie test2 sa podarilo\n\n");
	}
	else
	{
		printf("Uvolnenie test2 sa nepodarilo\n\n");
	}

	if (!memory_free(test3))
	{
		printf("Uvolnenie test2 sa podarilo\n\n");
	}
	else
	{
		printf("Uvolnenie test2 sa nepodarilo\n\n");
	}
	//memory_free(test2);
	//memory_free(test3);

	getchar();
	getchar();
	return 0;
}