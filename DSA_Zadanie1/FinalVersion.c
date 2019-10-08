// zadanie1.c -- Dominik Mikuška, 7.10.2019 07:14
#include <stdio.h>
#include <stdlib.h>

void *start;

void memory_initChar(void *ptr, unsigned int size)
{
	start = ptr;

	*(int*)start = size;	//Zapisanie velkosti do prvych 4 bytes

	*(char*)((char*)start + sizeof(int)) = 2 + sizeof(int);	//Smernik na prvu volnu velkost

	*((char*)start + 1 + sizeof(int)) = size - 1 - sizeof(int);	//Zapisanie velkosti prvej volnej pamati

	*((char*)start + 2 + sizeof(int)) = -1;	//Smernik na druhu volnu velkost
	//POUZIVANIE -1 NAMIESTO NULL

	*(int*)start |= (1 << 31);

}

int memory_check(void *ptr)
{
	if (ptr == NULL)
		return 0;

	if (*(int*)start < 0)
	{
		if (*((char*)ptr - 1) < 0)
			return 0;
	}
	else
	{
		if (*(int*)((char*)ptr - sizeof(int)) < 0)
			return 0;
	}

	return 1;
}

void *memory_allocChar(unsigned int size)
{
	*(int*)start ^= 1 << 31;

	//Ak nemame ziadny volny blok - return
	if (*((char*)start + sizeof(int)) == -1)
	{
		*(int*)start |= 1 << 31;
		return NULL;
	}

	int num = *((char*)start + sizeof(int));
	int tmp = sizeof(int); //ukazovatel na posledny odkaz NEXT


	do
	{
		if (*((char*)start + num - 1) >= (size + 1) && *((char*)start + num - 1) > 0)
		{ //Ak miesto vyhovuje

			//Vytvorenie novej particie ak zostalo viac ako 8bytes (4B hlavicka, 4B NEXT) a nepresahujeme danu pamat
			if (*((char*)start + num - 1) - (size + 1) >= 8 && (num + size + 1) <= *(int*)start)
			{
				//Nastavenie velkosti noveho bloku
				*((char*)start + num + size) = *((char*)start + num - 1) - (size + 1);

				//Zmensenie aktualnej velkosti
				*((char*)start + num - 1) = size + 1;

				//Zapisanie NEXT
				*((char*)start + num + size + 1) = *((char*)start + num);

				//Prepisanie NEXT minuleho bloku
				*((char*)start + tmp) = num + size + 1;

			}
			else
			{
				//Prepisanie NEXT na dalsi volny blok;
				*((char*)start + tmp) = *((char*)start + num);

			}

			//Nastavenie bitu na 1 - OBSADENY
			*((char*)start + num - 1) |= 1 << 7;
			*(int*)start |= 1 << 31;
			return (void*)((char*)start + num);
		}
		else
		{
			if (*((char*)start + num) == -1)
				break; //Niekedy sa moze stat, ze vyjdeme von z pamati a to nechceme

			//Pokracovanie v prehladavani
			tmp = num;
			num = *((char*)start + num);
		}
	} while (*((char*)start + num) != -1);

	*(int*)start |= 1 << 31;
	return NULL;

}

int memory_freeChar(void *valid_ptr)
{
	*(int*)start ^= 1 << 31;

	int position = (char*)valid_ptr - (char*)start;// - sizeof(int);
	int num = sizeof(int);
	int tmp = num;

	//Ak sa snazime uvolnit uvolnene miesto
	if (*((char*)valid_ptr - 1) >= 0)
	{
		*(int*)start |= 1 << 31;
		return 1;
	}

	//Dostanem sa na pozadovane miesto
	while (*((char*)start + num) != -1 && (num < position || num == sizeof(int)))
	{
		tmp = num;
		num = *((char*)start + num);
	}

	//zistit, ci vkladam medze dva bloky alebo som na konci
	if (*((char*)start + num) == -1 && num < position)
		tmp = num;

	//Prepis prveho bitu na 0 - "uvolnenie"
	*((char*)valid_ptr - 1) ^= 1 << 7;


	//Pokus o merge s nasledujucim blokom - musi byt volny a nesmie to byt koniec pamati
	if (*((char*)valid_ptr - 1 + (*((char*)valid_ptr - 1))) > 0
		&& *((char*)valid_ptr - 1 + (*((char*)valid_ptr - 1))) < *(int*)start)
	{
		//Pozriem velkost uvolnovaneho bloku ( valid_ptr - sizeof(int) ) a nasledne k smerniku tuto velkost pricitam.
		//Ak je velkost nasledujuceho bloku <0, znamena to, ze je obsadeny a mergovat nebudeme.
		//Inak zvacsime velkost a prepiseme NEXT a PREVIOUS z nasledujuceho bloku
		*(char*)valid_ptr = *((char*)valid_ptr - 1 + (*((char*)valid_ptr - 1) + 1)); //Prepisanie NEXT
		*((char*)valid_ptr - 1) += *((char*)valid_ptr - 1 + (*((char*)valid_ptr - 1))); //Prepisanie velkosti
	}
	else //Ak sa neda spravit merge
	{
		//Nastavenie NEXT
		*((char*)valid_ptr) = *((char*)start + tmp);
	}

	//Pokus o merge s predchadzajucim blokom
	if (tmp == sizeof(int)) //Ak sme na zaciatku a nemame tam velkost bloku ani PREVIOUS
	{
		*((char*)start + tmp) = position;
	}
	else //Ak mame pred sebou blok
	{
		//Pokus o merge
		if ((char*)start + tmp - 1 + *(((char*)start + tmp - 1)) == ((char*)valid_ptr - 1))
		{
			//Zvacsenie velkosti
			*((char*)(char*)start + tmp - 1) += *((char*)valid_ptr - 1);
			//Prepisanie NEXT
			*((char*)start + tmp) = *(char*)valid_ptr;
		}
		else //Ak sa neda spravit merge
		{
			//Zmena NEXT
			*((char*)start + tmp) = position;
		}
	}

	*(int*)start |= 1 << 31;
	return 0;
}

void memory_init(void *ptr, unsigned int size)
{
	if (size < 126)
	{
		memory_initChar(ptr, size);
		return;
	}

	start = ptr;

	*(int*)start = size;	//Zapisanie velkosti do prvych 4 bytes

	*(int*)((char*)start + sizeof(int)) = 3 * sizeof(int);	//Smernik na prvu volnu velkost

	*(int*)((char*)start + 2 * sizeof(int)) = size - 2 * sizeof(int);	//Zapisanie velkosti prvej volnej pamati

	*(int*)((char*)start + 3 * sizeof(int)) = -1;	//Smernik na druhu volnu velkost
	//POUZIVANIE -1 NAMIESTO NULL

}

void *memory_alloc(unsigned int size)
{
	if (*(int*)start < 0)
		return memory_allocChar(size);

	if (size < 4) size = 4; //Zarovnanie najmenej na 4 bytes;

	//Ak nemame ziadny volny blok - return
	if (*(int*)((char*)start + sizeof(int)) == -1)
		return NULL;

	int num = *(int*)((char*)start + sizeof(int));
	int tmp = sizeof(int); //ukazovatel na posledny odkaz NEXT


	do
	{
		if (*(int*)((char*)start + num - sizeof(int)) >= (size + sizeof(int)) && *(int*)((char*)start + num - sizeof(int)) > 0)
		{ //Ak miesto vyhovuje

			//Vytvorenie novej particie ak zostalo viac ako 8bytes (4B hlavicka, 4B NEXT) a nepresahujeme danu pamat
			if (*(int*)((char*)start + num - sizeof(int)) - (size + sizeof(int)) >= 8 && (num + size + sizeof(int)) <= *(int*)start)
			{
				//Nastavenie velkosti noveho bloku
				*(int*)((char*)start + num + size) = *(int*)((char*)start + num - sizeof(int)) - (size + sizeof(int));

				//Zmensenie aktualnej velkosti
				*(int*)((char*)start + num - sizeof(int)) = size + sizeof(int);

				//Zapisanie NEXT
				*(int*)((char*)start + num + size + sizeof(int)) = *(int*)((char*)start + num);

				//Prepisanie NEXT minuleho bloku
				*(int*)((char*)start + tmp) = num + size + sizeof(int);

			}
			else
			{
				//Prepisanie NEXT na dalsi volny blok;
				*(int*)((char*)start + tmp) = *(int*)((char*)start + num);

			}

			//Nastavenie bitu na 1 - OBSADENY
			*(int*)((char*)start + num - sizeof(int)) |= 1 << 31;

			return (void*)((char*)start + num);
		}
		else
		{
			if (*(int*)((char*)start + num) == -1)
				break; //Niekedy sa moze stat, ze vyjdeme von z pamati a to nechceme

			//Pokracovanie v prehladavani
			tmp = num;
			num = *(int*)((char*)start + num);
		}
	} while (*(int*)((char*)start + num) != -1);

	return NULL;

}

int memory_free(void *valid_ptr)
{
	if (*(int*)start < 0)
		return memory_freeChar(valid_ptr);

	int position = (char*)valid_ptr - (char*)start;
	int num = sizeof(int);
	int tmp = num;

	//Ak sa snazime uvolnit uvolnene miesto
	if (*(int*)((char*)valid_ptr - sizeof(int)) >= 0)
	{
		return 1;
	}

	//Dostanem sa na pozadovane miesto
	while (*(int*)((char*)start + num) != -1 && (num < position || num == sizeof(int)))
	{
		tmp = num;
		num = *(int*)((char*)start + num);
	}

	//zistit, ci vkladam medze dva bloky alebo som na konci
	if (*(int*)((char*)start + num) == -1 && num < position)
		tmp = num;

	//Prepis prveho bitu na 0 - "uvolnenie"
	*(int*)((char*)valid_ptr - sizeof(int)) ^= 1 << 31;


	//Pokus o merge s nasledujucim blokom - musi byt volny a nesmie to byt koniec pamati
	if (*(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)))) > 0
		&& *(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)))) < *(int*)start)
	{
		//Pozriem velkost uvolnovaneho bloku ( valid_ptr - sizeof(int) ) a nasledne k smerniku tuto velkost pricitam.
		//Ak je velkost nasledujuceho bloku <0, znamena to, ze je obsadeny a mergovat nebudeme.
		//Inak zvacsime velkost a prepiseme NEXT a PREVIOUS z nasledujuceho bloku
		*(int*)(char*)valid_ptr = *(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)) + sizeof(int))); //Prepisanie NEXT
		*(int*)((char*)valid_ptr - sizeof(int)) += *(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)))); //Prepisanie velkosti
	}
	else //Ak sa neda spravit merge
	{
		//Nastavenie NEXT
		*(int*)((char*)valid_ptr) = *(int*)((char*)start + tmp);
	}

	//Pokus o merge s predchadzajucim blokom
	if (tmp == sizeof(int)) //Ak sme na zaciatku a nemame tam velkost bloku ani PREVIOUS
	{
		*(int*)((char*)start + tmp) = position;
	}
	else //Ak mame pred sebou blok
	{
		//Pokus o merge
		if ((char*)start + tmp - sizeof(int) + *(int*)(((char*)start + tmp - sizeof(int))) == (int*)((char*)valid_ptr - sizeof(int)))
		{
			//Zvacsenie velkosti
			*(int*)((char*)(char*)start + tmp - sizeof(int)) += *(int*)((char*)valid_ptr - sizeof(int));
			//Prepisanie NEXT
			*(int*)((char*)start + tmp) = *(int*)(char*)valid_ptr;
		}
		else //Ak sa neda spravit merge
		{
			//Zmena NEXT
			*(int*)((char*)start + tmp) = position;
		}
	}

	return 0;
}


int main()
{
	char region[50];
	memory_init(region, 50);
	char* pointer = (char*)memory_alloc(10);
	if (pointer)
		memset(pointer, 0, 10);
	if (pointer)
		memory_free(pointer);
	return 0;
}