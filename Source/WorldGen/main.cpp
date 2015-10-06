#include "Mapper.h"
#include <time.h>
#include "GeneratorPeopleRooms.h"
#include "GeneratorPeopleRoomsItems.h"
int main(int argc, char ** argv)
{
	if ( argc > 1 )
	{
		int r = atoi(argv[1]);
		WorldGen::GeneratorPeopleRoomsItems generator( 5, 4, r );
		printf( "Seed: %d\n", r );
		if ( argc > 2 && ( argv[ 2 ][ 0 ] == 's' ) )
			generator.ShortPrint();
		else
		{
			generator.Print();
		}
		return 0;
	}
	srand(time(0));
	for ( int i = 0; i < 20; ++i )
	{
		int r = rand();
		WorldGen::GeneratorPeopleRooms generator( 5, 4, r );
		printf( "Seed: %d", r );
		// generator.Print();
		generator.ShortPrint();
		printf("\n\n--------------------------------------------------\n\n");
	}
	return 0;
}
