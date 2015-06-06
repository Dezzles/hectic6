#pragma once
#include "Data.h"
#include "Mapper.h"
#include <vector>
namespace WorldGen
{
	class Generator
	{
	public:
		Generator( int Width, int Height, int Seed );
		void Print();

		Wrapper<Room> Rooms_;
		Wrapper<Person> People_;
		Wrapper<Time> Times_;
		Wrapper<Information> Information_;
		Wrapper<InfoForPlayer> PlayerInfo_;
		Murder Murder_;
	private:
		Mapper Mapper_;
	};


}