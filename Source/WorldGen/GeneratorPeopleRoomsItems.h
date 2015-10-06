#pragma once
#include "Data.h"
#include "Database.h"
#include "Mapper.h"
#include <vector>
namespace WorldGen
{
	class GeneratorPeopleRoomsItems
	{
	public:
		GeneratorPeopleRoomsItems( int People, int Hours, int Seed );
		void GenerateClues();
		void Print();
		void ShortPrint();

		Database DB_;
	private:
		Mapper Mapper_;
		int Seed_;
	};


}