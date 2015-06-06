#include "Mapper.h"

#include "Generator.h"
int main()
{

	WorldGen::Generator generator( 4, 4, 0x845efad7 );
	generator.Print();

	return 0;
}

