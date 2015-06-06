#include "Mapper.h"

#include "Generator.h"
int main()
{

	WorldGen::Generator generator( 5, 4, 0x845efad7 );
	generator.Print();

	return 0;
}

