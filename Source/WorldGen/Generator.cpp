#include "Generator.h"
#include "cpplinq.hpp"
WorldGen::Generator::Generator( int Width, int Height, int Seed )
	:Mapper_(Width, Height, Seed), Murder_( 0 )
{

	Mapper_.NormaliseRooms();
	Mapper_.Print();
	printf( "\n" );
	int GuiltyX = -1;
	int GuiltyY = -1;
	for ( int Idx1 = 0; Idx1 < Mapper_.Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Mapper_.Height_; ++Idx2 )
		{
			if ( Mapper_.Data[ Idx1 ][ Idx2 ] != Mapper::GUILTY )
			{
				if ( Rooms_.GetItemById( Mapper_.Data[ Idx1 ][ Idx2 ] ) == nullptr ) 
				{
					Rooms_.Create( Mapper_.Data[ Idx1 ][ Idx2 ] );
				}
			}
			else
			{
				GuiltyX = Idx1;
				GuiltyY = Idx2;
			}
		}
	}

	for ( int Idx = 0; Idx < Mapper_.Width_; ++Idx )
	{
		People_.Create( Idx );
	}

	for ( int Idx = 0; Idx <= Mapper_.Height_; ++Idx )
	{
		Times_.Create( Idx );
	}


	Mapper::Pair murderLocation = Mapper_.GetMurder();
	Murder_.PersonId_ = murderLocation.X_;
	Murder_.TimeId_ = murderLocation.Y_;
	Murder_.RoomId_ = Mapper_.GetRoomIdNotInRow( murderLocation.Y_ );

	Mapper_.Data[ murderLocation.X_ ][ murderLocation.Y_ ] = Murder_.RoomId_;


	for ( int PersonIdx = 0; PersonIdx < Mapper_.Width_; ++PersonIdx )
	{
		int Start = 0;
		int PrevRoomId = -1;
		if ( ( PersonIdx == Murder_.PersonId_ ) && ( Murder_.TimeId_ == Start ) )
			++Start;
		Information* info = Information_.Create();
		info->PersonId_ = PersonIdx;
		info->StartTimeId_ = 0;
		info->EndTimeId_ = 0;
		info->RoomId_ = Mapper_.Data[ PersonIdx ][ Start ];
		PrevRoomId = info->RoomId_;
		for ( int TimeIdx = Start; TimeIdx < Mapper_.Height_; ++TimeIdx )
		{
			if ( ( PersonIdx == Murder_.PersonId_ ) && ( Murder_.TimeId_ == TimeIdx ) )
			{
				PrevRoomId = -1;
				continue;
			}
			if ( PrevRoomId != Mapper_.Data[ PersonIdx ][ TimeIdx ] )
			{
				info = Information_.Create();
				info->PersonId_ = PersonIdx;
				info->StartTimeId_ = TimeIdx;
				info->EndTimeId_ = TimeIdx;
				info->RoomId_ = Mapper_.Data[ PersonIdx ][ TimeIdx ];
			}
			PrevRoomId = Mapper_.Data[ PersonIdx ][ TimeIdx ];
			++info->EndTimeId_;
		}
	}
}


void WorldGen::Generator::Print()
{
	Mapper_.Print();
	printf( "\n" );

	for ( int Idx = 0; Idx < Rooms_.Size(); ++Idx )
	{
		printf( "%s\n", Rooms_.GetItem( Idx )->ToString().c_str() );
	}
	printf("\n");

	for ( int Idx = 0; Idx < People_.Size(); ++Idx )
	{
		printf( "%s\n", People_.GetItem( Idx )->ToString().c_str() );
	}
	printf( "\n" );

	for ( int Idx = 0; Idx < Times_.Size(); ++Idx )
	{
		printf( "%s\n", Times_.GetItem( Idx )->ToString().c_str() );
	}

	printf( "Id\tRoom\tPerson\tStart\tFinish\n" );
	for ( int Idx = 0; Idx < Information_.Size(); ++Idx )
	{
		Information* info = Information_.GetItem( Idx );
		printf( "%d\t%d\t%d\t%d\t%d\n", info->Id_, info->RoomId_, info->PersonId_, info->StartTimeId_, info->EndTimeId_ );
	}
	printf( "\n" );

	using namespace cpplinq;

	auto c = from_iterators(Information_.Internal().begin(), Information_.Internal().end())
		>> where( [ ]( Information* const & c ) { return c->PersonId_ == 0; } )
		>> orderby( [ ]( Information* const & c ) {return c->RoomId_; }, true )
		>> to_vector();
	
	for ( int Idx = 0; Idx < c.size(); ++Idx )
	{
		Information* info = c[ Idx ];
		printf( "%d\t%d\t%d\t%d\t%d\n", info->Id_, info->RoomId_, info->PersonId_, info->StartTimeId_, info->EndTimeId_ );
	}

	printf( "\n" );
	auto c2 = from_iterators( Information_.Internal().begin(), Information_.Internal().end() )
		>> join(
		from_iterators( Information_.Internal().begin(), Information_.Internal().end() ),
		[ ]( Information* const & c ) {return c->RoomId_; },
		[ ]( Information* const & ca ) {return ca->RoomId_; },
		[ ]( Information* const & c, Information* const & ca ) {return std::make_pair( c, ca ); } )
		>> where([ ](std::pair<Information*, Information*> const & p) 
			{
				return (p.first->PersonId_ < p.second->PersonId_) &&
					(p.first->RoomId_ == p.second->RoomId_ ) &&
					!( ( p.first->EndTimeId_ <= p.second->StartTimeId_ ) ||
						(p.first->StartTimeId_ >= p.second->EndTimeId_ )
					);
			})
		>> to_vector();
	for ( int Idx = 0; Idx < c2.size(); ++Idx )
	{
		Information* infoA = c2[ Idx ].first;
		Information* infoB = c2[ Idx ].second;
		printf("OVERLAP");
		printf( "\t%d\t%d\t%d\t%d\t%d\n", infoA->Id_, infoA->RoomId_, infoA->PersonId_, infoA->StartTimeId_, infoA->EndTimeId_ );
		printf( "\t%d\t%d\t%d\t%d\t%d\n", infoB->Id_, infoB->RoomId_, infoB->PersonId_, infoB->StartTimeId_, infoB->EndTimeId_ );
		printf("\n");
	}

	printf( "\n" );

	printf("Murderer: \t%d\nRoom: \t\t%d\nTime:\t\t%d\n", Murder_.PersonId_, Murder_.RoomId_, Murder_.TimeId_);
}