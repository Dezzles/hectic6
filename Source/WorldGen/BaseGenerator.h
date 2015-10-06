#ifndef BASEGENERATOR_H
#define BASEGENERATOR_H

#include "Data.h"
#include "Mapper.h"
#include <vector>

#include "Database.h"

namespace WorldGen
{
	class BaseGenerator
	{
	public :
		BaseGenerator() {};
		

		virtual void GenerateClues() {};
		virtual void Print() {};
		virtual void ShortPrint() {};

		Database DB_;
	};
}

#endif