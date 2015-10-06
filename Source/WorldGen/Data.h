#pragma once
#include "Database.h"
#include <string>
#include <vector>
namespace WorldGen
{
	class Database;
	class Murder;
	class InfoForPlayer;
	class Item;

	class Data
	{
	public:
		Data( int Id , std::string ClassName );

		int Id_;

		std::string ToString();
		Database* Database_;
	private: 
		std::string ClassName_;
	};

	class Room
		: public Data
	{
	public:
		Room( int Id );
		bool MurderRoom_;
		int NormalRoomId_;
	};

	class Person
		: public Data
	{
	public:
		Person( int Id );
		std::vector<int> InformationIds_;
		MAP_MANY(Information_, PlayerInfo_, InfoForPlayer, InformationIds_, Person);
	};

	class Time
		: public Data
	{
	public:
		Time( int Id );
	};

	class Information
		: public Data
	{
	public:
		Information( int Id );
		int StartTimeId_;
		int EndTimeId_;
		int PersonId_;
		int RoomId_;

		MAP_ONE(Person, People_, Person, PersonId_);
		MAP_ONE(Room, Rooms_, Room, RoomId_);
	};

	class Murder
		: public Data
	{
	public:
		Murder( int Id );
		int RoomId_;
		int TimeId_;
		int PersonId_;
		int ItemId_;

		MAP_ONE(Room, Rooms_, Room, RoomId_);
		MAP_ONE(Person, People_, Person, PersonId_);
		MAP_ONE(Item, Items_, Item, ItemId_);
	};

	class InfoForPlayer
		: public Data
	{
	public:
		InfoForPlayer( int Id );

		int RoomId_;
		int StartTimeId_;
		int EndTimeId_;
		int PersonId_;
		int TargetId_;

		MAP_ONE( Room, Rooms_, Room, RoomId_ );
		MAP_ONE( Person, People_, Person, PersonId_ );
		MAP_ONE( Target, People_, Person, TargetId_ );
	};

	class Item
		: public Data
	{
	public:
		Item( int Id );

	};
}