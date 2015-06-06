#include "Data.h"
#include <cstdio>

WorldGen::Data::Data( int Id, std::string ClassName )
	: Id_( Id ), ClassName_( ClassName )
{

}

std::string WorldGen::Data::ToString()
{
	char buffer[ 64 ];
	std::sprintf(buffer, "%s %d", ClassName_.c_str(), Id_ );

	return std::string(buffer);
}

WorldGen::Information::Information( int Id )
	: Data( Id, "Information" )
{

}

WorldGen::Person::Person( int Id )
	: Data( Id, "Person" )
{

}

WorldGen::Room::Room( int Id )
	: Data( Id, "Room" )
{

}

WorldGen::Time::Time( int Id )
	: Data( Id, "Time" ) 
{

}

WorldGen::Murder::Murder( int Id )
	: Data( Id, "Murder" )
{

}