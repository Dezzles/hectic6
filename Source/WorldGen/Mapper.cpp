#include "Mapper.h"

#include <memory>
#include <string>
#include <vector>

WorldGen::Mapper::Mapper( int Width, int Height, int Seed )
	:Width_( Width ), Height_( Height ), Seed_( Seed )
{
	memset( Data, UNUSED, MemSize * MemSize * sizeof( int ) );
	srand( Seed );
	Generate();
}

void WorldGen::Mapper::Print()
{
	for ( int Idx1 = -1; Idx1 < Width_ + 1; ++Idx1 )
	{
		printf("#");
	}
	printf( "\n" );

	for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
	{
		printf( "#" );
		for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
		{
			printf("%c", Data[ Idx1 ][ Idx2 ] == -1 ? ' ' : Data[ Idx1 ][ Idx2 ] + 'a' );
		}
		printf( "#\n" );
	}
	for ( int Idx1 = -1; Idx1 < Width_ + 1; ++Idx1 )
	{
		printf( "#" );

	}
	printf("\n\n");
}

void WorldGen::Mapper::Generate()
{
	int u = rand() % Width_;
	int v = rand() % Height_;

	Data[ u ][ v ] = GUILTY;
	int z = 1;
	while ( UnsetLocationExists() )
	{
		Pair p = GetUnsetPair();
		Data[ p.X_ ][ p.Y_ ] = LOCKED;
		int match = UnusedColumn( p.Y_ );
		if ( match == -1 )
		{
			 int column = GetRandomSetColumn( p.Y_ );
			 Data[ p.X_ ][ p.Y_ ] = Data[ column ][ p.Y_ ];
		}
		else
		{
			int sameRoom = rand() % 2;
			int newVal = UNUSED;
			if ( sameRoom == 1 )
			{
				int newY1 = UNUSED;
				int newY2 = UNUSED;
				if ( p.Y_ > 0 )
				{
					newY1 = GetLocation( p.X_ , p.Y_ - 1 );
				}
				if ( p.Y_ < Height_ - 1 )
				{
					newY2 = GetLocation( p.X_, p.Y_ + 1 );
				}
				if ( ( newY1 > UNUSED ) && ( newY2 > UNUSED ) )
				{
					newVal = (rand() % 2 == 0) ? newY1 : newY2;
				}
				else if ( newY1 > UNUSED )
					newVal = newY1;
				else if ( newY2 > UNUSED )
					newVal = newY2;
			}
			if ( ( sameRoom == 0 ) || ( newVal == UNUSED ) )
			{
				newVal = z;
				++z;
			}
			Data[ p.X_ ][ p.Y_ ] = newVal;
			Data[ match ][ p.Y_ ] = newVal;
		}
	}
}

WorldGen::Mapper::Pair WorldGen::Mapper::GetUnsetPair()
{
	std::vector<Pair> AvailablePairs;
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Data[ Idx1 ][ Idx2 ] == UNUSED )
			{
				AvailablePairs.push_back( Pair( Idx1, Idx2 ) );
			}
		}
	}

	return AvailablePairs[ rand() % AvailablePairs.size() ];
}

bool WorldGen::Mapper::UnsetLocationExists()
{
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Data[ Idx1 ][ Idx2 ] == UNUSED )
			{
				return true;
			}
		}
	}

	return false;
}

int WorldGen::Mapper::UnusedColumn( int Row )
{
	std::vector<int> unused;
	for ( int Idx = 0; Idx < Width_; ++Idx )
	{
		if ( Data[ Idx ][ Row ] == UNUSED )
			unused.push_back( Idx );
	}
	if ( unused.size() == 0 )
		return -1;
	return unused[ rand() % unused.size() ];
}

int WorldGen::Mapper::UnusedRow( int Col )
{
	std::vector<int> unused;
	for ( int Idx = 0; Idx < Width_; ++Idx )
	{
		if ( Data[ Col ][ Idx ] == UNUSED )
			unused.push_back( Idx );
	}
	if ( unused.size() == 0 )
		return -1;
	return unused[ rand() % unused.size() ];
}

void WorldGen::Mapper::SetLocation( int X, int Y, int Val )
{
	Data[ X ][ Y ] = Val;
}

int WorldGen::Mapper::GetLocation( int X, int Y )
{
	return Data[ X ][ Y ];
}

int WorldGen::Mapper::GetRandomSetColumn( int Y )
{
	std::vector<int> unused;
	for ( int Idx = 0; Idx < Width_; ++Idx )
	{
		if ( Data[ Idx ][ Y ] > UNUSED )
			unused.push_back( Idx );
	}
	if ( unused.size() == 0 )
		return -1;
	return unused[ rand() % unused.size() ];

}

void WorldGen::Mapper::NormaliseRooms()
{
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Data[ Idx1 ][ Idx2 ] == GUILTY )
				continue;
			if ( !IdInOtherRow( Data[ Idx1 ][ Idx2 ], Idx2 ) )
			{
				int NewId = GetRoomIdNotInRow( Idx2 );
				if ( NewId != UNUSED )
					ReplaceRoom( Data[ Idx1 ][ Idx2 ], NewId );
			}
		}
	}

}

void WorldGen::Mapper::ReplaceRoom( int RoomId, int NewId )
{
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Data[ Idx1 ][ Idx2 ] == RoomId )
				Data[ Idx1 ][ Idx2 ] = NewId;
		}

	}
}

bool WorldGen::Mapper::IdInOtherRow( int RoomId, int CurrentRow )
{
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Idx2 == CurrentRow )
				continue;
			if ( Data[ Idx1 ][ Idx2 ] == RoomId )
				return true;
		}
	}
	return false;
}


int WorldGen::Mapper::GetRoomIdNotInRow( int CurrentRow )
{
	std::vector< int > MyRowRoomIds;
	std::vector< int > NewRoomIds;
	for ( int Idx = 0; Idx < Width_; ++Idx )
	{
		if ( std::find( MyRowRoomIds.begin(), MyRowRoomIds.end(), Data[ Idx ][ CurrentRow ] ) == MyRowRoomIds.end() )
			MyRowRoomIds.push_back( Data[ Idx ][ CurrentRow ] );
	}
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Idx2 == CurrentRow )
				continue;
			if ( Data[ Idx1 ][ Idx2 ] == GUILTY )
				continue;
			if ( std::find( MyRowRoomIds.begin(), MyRowRoomIds.end(), Data[ Idx1 ][ Idx2 ] ) == MyRowRoomIds.end() )
			{
				if ( std::find( NewRoomIds.begin(), NewRoomIds.end(), Data[ Idx1 ][ Idx2 ] ) == NewRoomIds.end() )
				{
					NewRoomIds.push_back( Data[ Idx1 ][ Idx2 ] );
				}
			}
		}
	}
	if ( NewRoomIds.size() == 0 )
		return UNUSED;
	return NewRoomIds[ rand() % NewRoomIds.size() ];


}

WorldGen::Mapper::Pair WorldGen::Mapper::GetMurder()
{
	for ( int Idx1 = 0; Idx1 < Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Height_; ++Idx2 )
		{
			if ( Data[ Idx1 ][ Idx2 ] == GUILTY )
			{
				return Pair( Idx1, Idx2 );
			}
		}
	}
	return Pair( UNUSED, UNUSED );
}
