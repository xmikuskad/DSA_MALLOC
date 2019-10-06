#include <stdio.h>
#include <stdlib.h>

int* start; 
//mozeme pouzivat aj ine typy ako void

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
			
			//Vytvorenie novej particie ak zostalo viac ako 12bytes (4B hlavicka, 8B data) a nepresahujeme danu pamat
			if (*(start + num - sizeof(int)) - (size+sizeof(int)) >= 12 && (num + size + sizeof(int)) <= *start)
			{
				//Nastavenie velkosti noveho bloku
				*(start + num + size) = *(start + num - sizeof(int)) - (size + sizeof(int));

				//Zmensenie aktualnej velkosti
				*(start + num - sizeof(int)) = size + sizeof(int);

				//Zapisanie NEXT
				*(start + num + size + sizeof(int)) = *(start + num);

				//Zapisanie PREVIOUS
				*(start + num + size + 2 * sizeof(int)) = tmp;

				//Prepisanie NEXT minuleho bloku
				*(start + tmp) = num + size + sizeof(int);

			}
			else
			{
				//Prepisanie NEXT na dalsi volny blok;
  				*(start + tmp) = *(start + num); 

				if (tmp != sizeof(int)) //Pokial nie sme na zaciatku (tam nie je PREVIOUS)
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

int memory_free(void *valid_ptr)
{
	printf("Start: %p\n", start);
	printf("Smernik: %p\n", valid_ptr);
	printf("Rozdiel: %d\n\n", (int*)valid_ptr - start);

	int position = (int*)valid_ptr - start;
	int num = sizeof(int); 
	int tmp = num;
	//int flag = 0; //flag sluzi na rozpoznanie, ci prebehol merge s nasledujucim blokom

	//Ak sa snazime uvolnit uvolnene miesto
	if (*((int*)valid_ptr - sizeof(int)) >= 0)
	{
		printf("Pokus o uvolnenie uvolnenej pamati - chyba\n");
		return 1;
	}

	//Dostanem sa na pozadovane miesto
	while (*(start + num) != -1 && (num < position || num==sizeof(int)))
	{
		tmp = num;
		num = *(start + num);
	}

	//zistit, ci vkladam medze dva bloky alebo som na konci
	if (*(start + num) == -1 && num < position)
		tmp = num;

	//Prepis prveho bitu na 0 - "uvolnenie"
	*((int*)valid_ptr - sizeof(int)) ^= 1 << 31; //*((int*)valid_ptr - sizeof(int))<<1


	//Pokus o merge s nasledujucim blokom - musi byt volny a nesmie to byt koniec pamati
	if (*((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)))) > 0
		&& *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)))) < *start)
	{
		printf("Merge s nasledujucim blokom\n");

		//Pozriem velkost uvolnovaneho bloku ( valid_ptr - sizeof(int) ) a nasledne k smerniku tuto velkost pricitam.
		//Ak je velkost nasledujuceho bloku <0, znamena to, ze je obsadeny a mergovat nebudeme.
		//Inak zvacsime velkost a prepiseme NEXT a PREVIOUS z nasledujuceho bloku
		//*((int*)valid_ptr - sizeof(int)) += *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int))));
		*(int*)valid_ptr = *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int))+sizeof(int))); //Prepisanie NEXT
		*((int*)valid_ptr + sizeof(int)) = *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)) + 2*sizeof(int))); //Prepisanie PREVIOUS
		*((int*)valid_ptr - sizeof(int)) += *((int*)valid_ptr - sizeof(int) + (*((int*)valid_ptr - sizeof(int)))); //Prepisanie velkosti
	}
	else //Ak sa neda spravit merge
	{
		printf("Merge s nasledujucim blokom neprebehol\n");
		//Nastavenie NEXT
		//*(int*)valid_ptr = *(start + tmp);
		/*
		int tmp2 = sizeof(int);

		//Prehladanie miesta, kam tento blok zaradime
		while (*(start + tmp2) > 0 && tmp2 < position)
		{
			tmp2 = *(start + tmp2);
		}*/

		//Nastavenie NEXT
		*((int*)valid_ptr) = *(start + tmp);

		//Pokial nenastavujeme PREVIOUS na zaciatku
		if (num != sizeof(int))
		{
			*(start + num + sizeof(int)) = position;
		}

		//*(start + tmp2) = position;

		/*if (tmp != sizeof(int))
		{
			//Nastavenie NEXT (okrem zaciatku)
			//*(int*)valid_ptr = num;


			//Zmena PREVIOUS iba ak nasleduje niaky blok
			*(start + tmp + sizeof(int)) = position;

			//Nastavenie NEXT (okrem zaciatku)
			//*(int*)valid_ptr = *(start + num - sizeof(int));
		}
		/*else //Specialne pre prvy NEXT, pretoze ten nema PREVIOUS
		{
			*(int*)valid_ptr = -1;
		}*/
	}

	//Pokus o merge s predchadzajucim blokom
	if (tmp == sizeof(int)) //Ak sme na zaciatku a nemame tam velkost bloku ani PREVIOUS
	{
		printf("Nastavenie nextu na zaciatku\n");
		*(start + tmp) = position;
	}
	else //Ak mame pred sebou blok
	{
		//Pokus o merge
		if (start + tmp - sizeof(int) + *((start + tmp - sizeof(int))) == ((int*)valid_ptr-sizeof(int)))
		{
			printf("Merge na zaciatku prebehol\n");

			//Zvacsenie velkosti
			*(start + tmp - sizeof(int)) += *((int*)valid_ptr - sizeof(int));
			//Prepisanie NEXT
			*(start + tmp) = *(int*)valid_ptr;
			//Prepisanie PREVIOUS - WTF NETREBA
			/*if(*(start + tmp + sizeof(int)) != sizeof(int)) //Pokial neprepisujeme zaciatok (tam PREVIOUS nie je)
				*(start+*(start + tmp + sizeof(int))) = *((int*)valid_ptr+sizeof(int));
				*/

		}
		else //Ak sa neda spravit merge
		{
			printf("Merge na zaciatku neprebehol\n");
			//Nastavenie PREVIOUS
			*((int*)valid_ptr + sizeof(int)) = tmp;

			//Zmena NEXT
			*(start + tmp) = position;
		}
	}


	return 0;
}


int main()
{
	char a[50];
	char *test1, *test2, *test3,*test4;

	memory_init(a, 60);

	test1 = (char*)memory_alloc(8);
	if (test1 != NULL)
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

	if (!memory_free(test4))
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

	if (!memory_free(test1))
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