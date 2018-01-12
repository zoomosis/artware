#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <msgapi.h>
#include "idx.h"
/* #include <api_sq.h> */



void PersSquish(char *name);

void main()

{
	PersSquish("Kasper Kwant");
}


void PersSquish(char *name)

{
	int index;
	struct my_idx	idxrec;


	if( (index = open("d:\\echo\\test.sqi", O_RDONLY | O_BINARY)) == -1)

			{
			printf("\n\n Error opening index!");
			return;
			}


	read(index, &idxrec, sizeof(struct my_idx));

	printf("\n\n %li	-  %li ", idxrec.hash, SquishHash(name));
	close(index);
}
