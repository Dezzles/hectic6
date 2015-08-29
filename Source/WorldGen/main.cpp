#include "Mapper.h"
#include <time.h>
#include "Generator.h"
int main()
{
	for ( int i = 0; i < 20; ++i )
	{
		WorldGen::Generator generator( 4, 4, time(NULL) );
		generator.Print();
		printf("\n\n--------------------------------------------------\n\n");
	}
	return 0;
}

