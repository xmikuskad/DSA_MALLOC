#include <stdio.h>
#include <stdlib.h>

void *start;

void memory_init(void *ptr, unsigned int size)
{
	start = ptr;

	*(int*)start = size;	//Zapisanie velkosti do prvych 4 bytes

	*(int*)((char*)start + sizeof(int)) = 3 * sizeof(int);	//Smernik na prvu volnu velkost

	*(int*)((char*)start + 2 * sizeof(int)) = size - 2 * sizeof(int);	//Zapisanie velkosti prvej volnej pamati

	*(int*)((char*)start + 3 * sizeof(int)) = -1;	//Smernik na druhu volnu velkost
	//POUZIVANIE -1 NAMIESTO NULL

}

int memory_check(void *ptr)
{
	if (ptr == NULL)
		return 0;

	if (*(int*)((char*)ptr - sizeof(int)) < 0)
		return 0;

	return 1;
}

void *memory_alloc(unsigned int size)
{
	if (size < 4) size = 4; //Zarovnanie najmenej na 8 bytes;

	//Ak nemame ziadny volny blok - return
	if (*(int*)((char*)start + sizeof(int)) == -1)
		return NULL;

	int num = *(int*)((char*)start + sizeof(int));
	int tmp = sizeof(int); //ukazovatel na posledny odkaz NEXT


	do
	{
		if (*(int*)((char*)start + num - sizeof(int)) >= size && *(int*)((char*)start + num - sizeof(int)) > 0)
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
			printf("Velkost %d\n", *(int*)((char*)start + num - sizeof(int)));
			*(int*)((char*)start + num - sizeof(int)) |= 1 << 31;
			//printf("New value %d\n", *(int*)((char*)start + num - sizeof(int)));
			/**(int*)((char*)start + num - sizeof(int)) |= 1 << 8;
			printf("New value %d\n", *(int*)(start + num - sizeof(int)));*/

			//printf("Position of pointer: %d\n", (int*)((int*)((char*)start + num) - (int*)start));
			return (void*)((char*)start + num);
		}
		else
		{
			tmp = num;
			num = *(int*)((char*)start + num);
		}
	} while (*(int*)((char*)start + num) != -1);

	return NULL;

}

int memory_free(void *valid_ptr)
{
	int position = (char*)valid_ptr - (char*)start;
	printf("Position: %d\n", position);
	int num = sizeof(int);
	int tmp = num;

	//Ak sa snazime uvolnit uvolnene miesto
	if (*(int*)((char*)valid_ptr - sizeof(int)) >= 0)
	{
		printf("Pokus o uvolnenie uvolnenej pamati - chyba\n");
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
	*(int*)((char*)valid_ptr - sizeof(int)) ^= 1 << 31; //*(int*)((int*)valid_ptr - sizeof(int))<<1


	//Pokus o merge s nasledujucim blokom - musi byt volny a nesmie to byt koniec pamati
	if (*(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)))) > 0
		&& *(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)))) < *(int*)start)
	{
		printf("Merge s nasledujucim blokom\n");

		//Pozriem velkost uvolnovaneho bloku ( valid_ptr - sizeof(int) ) a nasledne k smerniku tuto velkost pricitam.
		//Ak je velkost nasledujuceho bloku <0, znamena to, ze je obsadeny a mergovat nebudeme.
		//Inak zvacsime velkost a prepiseme NEXT a PREVIOUS z nasledujuceho bloku
		//*(int*)((int*)valid_ptr - sizeof(int)) += *(int*)((int*)valid_ptr - sizeof(int) + (*(int*)((int*)valid_ptr - sizeof(int))));
		*(int*)(char*)valid_ptr = *(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)) + sizeof(int))); //Prepisanie NEXT
		*(int*)((char*)valid_ptr - sizeof(int)) += *(int*)((char*)valid_ptr - sizeof(int) + (*(int*)((char*)valid_ptr - sizeof(int)))); //Prepisanie velkosti
	}
	else //Ak sa neda spravit merge
	{
		printf("Merge s nasledujucim blokom neprebehol\n");

		//Nastavenie NEXT
		*(int*)((char*)valid_ptr) = *(int*)((char*)start + tmp);

	}

	//Pokus o merge s predchadzajucim blokom
	if (tmp == sizeof(int)) //Ak sme na zaciatku a nemame tam velkost bloku ani PREVIOUS
	{
		printf("Nastavenie nextu na zaciatku\n");
		*(int*)((char*)start + tmp) = position;
	}
	else //Ak mame pred sebou blok
	{
		//Pokus o merge
		if ((char*)start + tmp - sizeof(int) + *(int*)(((char*)start + tmp - sizeof(int))) == (int*)((char*)valid_ptr - sizeof(int)))
		{
			printf("Merge na zaciatku prebehol\n");

			//Zvacsenie velkosti
			*(int*)((char*)(char*)start + tmp - sizeof(int)) += *(int*)((char*)valid_ptr - sizeof(int));
			//Prepisanie NEXT
			*(int*)((char*)start + tmp) = *(int*)(char*)valid_ptr;


		}
		else //Ak sa neda spravit merge
		{
			printf("Merge na zaciatku neprebehol\n");

			//Zmena NEXT
			*(int*)((char*)start + tmp) = position;
		}
	}


	return 0;
}

void Tester()
{
	int count = 0, tmp = 0,tmp2=0;
	char* testik = NULL;
	char** pole = malloc(30 * sizeof(void*));

	while (1)
	{
		testik = memory_alloc(8);
		pole[count] = testik;
		if (testik == NULL)
			break;
		count++;
	}
	/*
	for (int i = 0; i < 5; i++)
	{
		testik = memory_alloc(8);
		printf("Testik: %p\n", testik);
		//memory_free(testik);
	}

	//printf("Uspesne alokovany %d x 8bytes\n", count);
	*/
	for (int i = 0; i < count; i++)
	{
		if (i % 2 == 0 || i%3==0)
			memory_free(pole[i]);
	}

	//memory_free(pole[2]);
	//memory_free(pole[3]);
	//memory_free(pole[1]);

	printf("DONE\n");
}

void TuringCorrecter()
{
	char* testik = NULL;
	char** pole = malloc(30 * sizeof(void*));

	printf("Velkost: %d\n", -2147479345 ^ (1 << 31));
	testik = memory_alloc(3583);	//	alloc(3583) -> 94754850078748
	pole[15] = testik;
	testik = memory_alloc(2326);	//	alloc(2326) -> 94754850082335
	pole[1] = testik;
	testik = memory_alloc(3537);//	alloc(3537) -> 94754850084665
	testik = memory_alloc(1863);//	alloc(1863) -> 94754850088206
	pole[20] = testik;
	testik = memory_alloc(4885);//	alloc(4885) -> 94754850090073
	testik = memory_alloc(4454);//	alloc(4454) -> 94754850094962
	pole[29] = testik;
	testik = memory_alloc(3964);//	alloc(3964) -> 94754850099420
	pole[23] = testik;
	testik = memory_alloc(2414);//	alloc(2414) -> 94754850103388
	pole[16] = testik;
	testik = memory_alloc(3441);//	alloc(3441) -> 94754850105806
	pole[10] = testik;
	testik = memory_alloc(2149);//	alloc(2149) -> 94754850109251
	pole[2] = testik;
	testik = memory_alloc(2104);//	alloc(2104) -> 94754850111404
	pole[7] = testik;
	testik = memory_alloc(779);//		alloc(779) -> 94754850113512
	pole[0] = testik;
	testik = memory_alloc(1218);//	alloc(1218) -> 94754850114295
	pole[5] = testik;
	testik = memory_alloc(2382);//	alloc(2382) -> 94754850115517
	pole[24] = testik;
	testik = memory_alloc(1320);//	alloc(1320) -> 94754850117903
	testik = memory_alloc(4873);//	alloc(4873) -> 94754850119227
	pole[9] = testik;
	testik = memory_alloc(4756);//	alloc(4756) -> 94754850124104
	pole[8] = testik;
	testik = memory_alloc(1619);//	alloc(1619) -> 94754850128864
	pole[19] = testik;
	testik = memory_alloc(1224);//	alloc(1224) -> 94754850130487
	pole[3] = testik;
	testik = memory_alloc(2822);//	alloc(2822) -> 94754850131715
	pole[4] = testik;
	testik = memory_alloc(4879);//	alloc(4879) -> 94754850134541
	pole[13] = testik;
	testik = memory_alloc(4675);//	alloc(4675) -> 94754850139424
	pole[27] = testik;
	testik = memory_alloc(1531);//	alloc(1531) -> 94754850144103
	pole[28] = testik;
	testik = memory_alloc(1098);//	alloc(1098) -> 94754850145638
	pole[17] = testik;
	testik = memory_alloc(843);//		alloc(843) -> 94754850146740
	testik = memory_alloc(4189);//	alloc(4189) -> 94754850147587
	pole[18] = testik;
	testik = memory_alloc(2067);//	alloc(2067) -> 94754850151780
	pole[14] = testik;
	testik = memory_alloc(3372);//	alloc(3372) -> 94754850153851
	pole[26] = testik;
	testik = memory_alloc(1495);//	alloc(1495) -> 94754850157227
	pole[25] = testik;
	testik = memory_alloc(2006);//	alloc(2006) -> 94754850158726
	pole[12] = testik;
	testik = memory_alloc(3237);//	alloc(3237) -> 94754850160736
	pole[21] = testik;
	testik = memory_alloc(3572);//	alloc(3572) -> 94754850163977
	pole[6] = testik;
	testik = memory_alloc(931);//		alloc(931) -> 94754850167553
	testik = memory_alloc(3139);//	alloc(3139) -> 94754850168488
	pole[22] = testik;
	testik = memory_alloc(4526);//	alloc(4526) -> 94754850171631
	pole[11] = testik;
		//free(94754850113512) OK(length 779). 0
		//free(94754850082335) OK(length 2326).1
		//free(94754850109251) OK(length 2149).2
		//free(94754850130487) OK(length 1224).3
		//free(94754850131715) OK(length 2822).4
		//free(94754850114295) OK(length 1218).5
		//free(94754850163977) OK(length 3572).6
		//free(94754850111404) OK(length 2104).7
		//free(94754850124104) OK(length 4756).8
		//free(94754850157227) OK(length 1495).9
		//free(94754850105806) OK(length 3441).10
		//free(94754850171631) OK(length 4526).11
		//free(94754850158726) OK(length 2006).12
		//free(94754850134541) OK(length 4879).13
		//free(94754850151780) OK(length 2067).14
		//free(94754850078748) OK(length 3583).15
		//free(94754850103388) OK(length 2414).16
		//free(94754850145638) OK(length 1098).17
		//free(94754850147587) OK(length 4189).18
		//free(94754850128864) OK(length 1619).19
		//free(94754850088206) OK(length 1863).20
		//free(94754850160736) OK(length 3237).21
		//free(94754850168488) OK(length 3139).22
		//free(94754850099420) OK(length 3964).23
		//free(94754850115517) OK(length 2382).24
		//free(94754850119227) OK(length 4873).25
		//free(94754850153851) OK(length 3372).26
		//free(94754850139424) OK(length 4675).27
		//free(94754850144103) OK(length 1531).28
		//free(94754850094962) OK(length 4454).29

	for (int i = 0; i < 30; i++)
	{
		memory_free(pole[i]);
	}

			testik = memory_alloc(4299);//alloc(4299) -> 94754850078748
			testik = memory_alloc(3224);
		
}

int main()
{
	char a[100000];
	char *test1, *test2, *test3, *test4;

	memory_init(a, 100000);


	//Tester();
	TuringCorrecter();
	
	getchar();
	getchar();
	return 0;
}