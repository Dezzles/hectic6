#pragma once
namespace WorldGen
{
	class Generator;
	class Mapper
	{
		friend Generator;
		struct Pair;
	public:
		Mapper( int Width, int Height, int Seed );

		void Print();
		void NormaliseRooms();
		Pair GetMurder();
	private:
		void ReplaceRoom( int RoomId, int NewId );
		bool IdInOtherRow( int RoomId, int CurrentRow );
		int GetRoomIdNotInRow( int CurrentRow );

		void SetLocation( int X, int Y, int Val );
		int GetLocation( int X, int Y );

		void Generate();
		Pair GetUnsetPair();
		bool UnsetLocationExists();
		int UnusedColumn( int Row );
		int UnusedRow( int Col );
		int GetRandomSetColumn( int Y );
		int Width_;
		int Height_;
		int Seed_;
		static const int MemSize = 8;

		int Data[ MemSize ][ MemSize ];
		struct Pair
		{
			Pair( int X, int Y )
				: X_( X ), Y_( Y )
			{

			}
			int X_;
			int Y_;
		};

		static const int UNUSED = -1;
		static const int LOCKED = -2;
		static const int GUILTY = '!' - 'a';
	};
}