#pragma once
#include "Data.h"
#include "Mapper.h"
#include <vector>
#include "Database.h"
namespace WorldGen
{
	class GeneratorPeopleRooms
	{
	public:
		GeneratorPeopleRooms( int People, int Hours, int Seed );
		void GenerateClues();
		void Print();
		void ShortPrint();

		Database DB_;
	private:
		Mapper Mapper_;
		int Seed_;
	};


}