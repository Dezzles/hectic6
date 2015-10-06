#ifndef DATABASE_H
#define DATABASE_H

#include "DataSet.h"

namespace WorldGen
{
	class Room;
	class Person;
	class Murder;
	class Time;
	class Information;
	class InfoForPlayer;
	class Murder;
	class Item;
	class Database
	{
	public:
		Database();

		DataSet<Room> Rooms_;
		DataSet<Person> People_;
		DataSet<Time> Times_;
		DataSet<Information> Information_;
		DataSet<InfoForPlayer> PlayerInfo_;
		DataSet<Murder> Murders_;
		DataSet<Item> Items_;

	};
}


#endif