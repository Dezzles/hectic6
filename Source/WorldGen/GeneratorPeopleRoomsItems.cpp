#include "GeneratorPeopleRoomsItems.h"
#include <cstdlib>

#include "cpplinq.hpp"

WorldGen::GeneratorPeopleRoomsItems::GeneratorPeopleRoomsItems( int People, int Hours, int Seed )
	:Mapper_( People, Hours, Seed ), Seed_( Seed )
{

	Mapper_.NormaliseRooms();
	//Mapper_.Print();
	printf( "\n" );
	int GuiltyX = -1;
	int GuiltyY = -1;
	int NormalRoomId = 0;

	Murder* Murder_ = DB_.Murders_.Create();
	Mapper::Pair murderLocation = Mapper_.GetMurder();
	Murder_->PersonId_ = murderLocation.X_;
	Murder_->TimeId_ = murderLocation.Y_;
	Murder_->RoomId_ = Mapper_.GetRoomIdNotInRow( murderLocation.Y_ );
	if ( Murder_->RoomId_ == -1 ){
		Murder_->RoomId_ = Mapper_.GetNewRoomId();
	}

	Mapper_.Data[ murderLocation.X_ ][ murderLocation.Y_ ] = Murder_->RoomId_;


	for ( int Idx1 = 0; Idx1 < Mapper_.Width_; ++Idx1 )
	{
		for ( int Idx2 = 0; Idx2 < Mapper_.Height_; ++Idx2 )
		{
			if ( Mapper_.Data[ Idx1 ][ Idx2 ] != Mapper::GUILTY )
			{
				if ( DB_.Rooms_.GetItemById( Mapper_.Data[ Idx1 ][ Idx2 ] ) == nullptr )
				{
					Room* room = DB_.Rooms_.Create( Mapper_.Data[ Idx1 ][ Idx2 ] );
					room->NormalRoomId_ = NormalRoomId;
					++NormalRoomId;
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
		DB_.People_.Create( Idx );
	}

	for ( int Idx = 0; Idx <= Mapper_.Height_; ++Idx )
	{
		DB_.Times_.Create( Idx );
	}


	

	for ( int PersonIdx = 0; PersonIdx < Mapper_.Width_; ++PersonIdx )
	{
		int Start = 0;
		int PrevRoomId = -1;
		if ( ( PersonIdx == Murder_->PersonId_ ) && ( Murder_->TimeId_ == Start ) )
			++Start;
		Information* info = DB_.Information_.Create();
		info->PersonId_ = PersonIdx;
		info->StartTimeId_ = 0;
		info->EndTimeId_ = 0;
		info->RoomId_ = Mapper_.Data[ PersonIdx ][ Start ];
		PrevRoomId = info->RoomId_;
		for ( int TimeIdx = Start; TimeIdx < Mapper_.Height_; ++TimeIdx )
		{
			if ( ( PersonIdx == Murder_->PersonId_ ) && ( Murder_->TimeId_ == TimeIdx ) )
			{
				PrevRoomId = -1;
				continue;
			}
			if ( PrevRoomId != Mapper_.Data[ PersonIdx ][ TimeIdx ] )
			{
				info = DB_.Information_.Create();
				info->PersonId_ = PersonIdx;
				info->StartTimeId_ = TimeIdx;
				info->EndTimeId_ = TimeIdx;
				info->RoomId_ = Mapper_.Data[ PersonIdx ][ TimeIdx ];
			}
			PrevRoomId = Mapper_.Data[ PersonIdx ][ TimeIdx ];
			++info->EndTimeId_;
		}
	}

	int minRooms = 10000;
	for ( int Idx0 = 0; Idx0 < Mapper_.Height_; ++Idx0 )
	{
		std::vector<int> rooms;
		for ( int Idx1 = 0; Idx1 < Mapper_.Width_; ++Idx1 )
		{
			bool containsKey = false;
			for ( int Idx2 = 0; Idx2 < rooms.size(); ++Idx2 )
			{
				if ( Mapper_.Data[ Idx0 ][ Idx1 ]  == rooms[ Idx2 ] )
				{
					containsKey = true;
					break;
				}
			}
			if ( !containsKey )
			{
				rooms.push_back( Mapper_.Data[ Idx0 ][ Idx1 ] );
			}
		}
		if ( minRooms > rooms.size() )
		{
			minRooms = rooms.size();
		}
	}

	for ( int Idx = 0; Idx < minRooms; ++Idx )
	{
		Item* item = DB_.Items_.Create();
		
	}
	GenerateClues();
}


void WorldGen::GeneratorPeopleRoomsItems::Print()
{
	Mapper_.Print();
	printf( "\n" );
	printf( "Seed: %d\n", Seed_ );
	for ( int Idx = 0; Idx < DB_.Rooms_.Size(); ++Idx )
	{
		printf( "%s\n", DB_.Rooms_.GetItem( Idx )->ToString().c_str() );
	}
	printf( "\n" );

	for ( int Idx = 0; Idx < DB_.People_.Size(); ++Idx )
	{
		printf( "%s\n", DB_.People_.GetItem( Idx )->ToString().c_str() );
	}
	printf( "\n" );

	for ( int Idx = 0; Idx < DB_.Times_.Size(); ++Idx )
	{
		printf( "%s\n", DB_.Times_.GetItem( Idx )->ToString().c_str() );
	}

	printf( "\tId\tRoom\tPerson\tStart\tFinish\n" );
	for ( int Idx = 0; Idx <DB_.Information_.Size(); ++Idx )
	{
		Information* info = DB_.Information_.GetItem( Idx );
		printf( "\t%d\t%d\t%d\t%d\t%d\n", info->Id_, info->RoomId_, info->GetPerson()->Id_, info->StartTimeId_, info->EndTimeId_ );
	}
	printf( "\n" );


	printf("Items");
	printf( "\tId\tRoom\tPerson\tStart\tFinish\n" );
	for ( int Idx = 0; Idx < DB_.Items_.Size(); ++Idx )
	{
		Item* item = DB_.Items_.GetItem( Idx );
		printf( "\t%d\n", item->Id_ );
	}

	printf("\n");
	using namespace cpplinq;

	Murder* Murder_ = DB_.Murders_.GetItem(0);
	printf( "Murderer: \t%d\nRoom: \t\t%d\nTime:\t\t%d\n", Murder_->PersonId_, Murder_->RoomId_, Murder_->TimeId_ );

	printf( "\n" );
	for ( int Idx1 = 0; Idx1 < DB_.People_.Size(); ++Idx1 )
	{
		Person* p = DB_.People_.GetItem( Idx1 );
		printf( "Person %c\n", p->Id_ + 'A' );
		for ( int Idx2 = 0; Idx2 < p->Information_.Size(); ++Idx2 )
		{
			printf( "\t%c\t%c\t%d-%d\n", p->Information_[ Idx2 ]->RoomId_ + 'Q', p->Information_[ Idx2 ]->TargetId_ + 'A',
				p->Information_[ Idx2 ]->StartTimeId_ + 13, p->Information_[ Idx2 ]->EndTimeId_ + 13 );
		}

		printf( "\n" );
	}
}

void WorldGen::GeneratorPeopleRoomsItems::ShortPrint()
{
	printf( "\n" );
	printf( "Seed: %d\n", Seed_ );

	using namespace cpplinq;

	auto c = from_iterators( DB_.Information_.Internal().begin(), DB_.Information_.Internal().end() )
		>> where( [ ]( Information* const & c ) { return c->PersonId_ == 0; } )
		>> orderby( [ ]( Information* const & c ) {return c->RoomId_; }, true )
		>> to_vector();

	Murder* Murder_ = DB_.Murders_.GetItem(0);
	printf( "Murderer: \t%d\nRoom: \t\t%d\nTime:\t\t%d\n", Murder_->PersonId_, Murder_->RoomId_, Murder_->TimeId_ );

	printf( "\n" );
	for ( int Idx1 = 0; Idx1 < DB_.People_.Size(); ++Idx1 )
	{
		Person* p = DB_.People_.GetItem( Idx1 );
		printf( "Person %c\n", p->Id_ + 'A' );
		for ( int Idx2 = 0; Idx2 < p->Information_.Size(); ++Idx2 )
		{
			printf( "\t%c\t%c\t%d-%d\n", p->Information_[ Idx2 ]->RoomId_ + 'Q', p->Information_[ Idx2 ]->TargetId_ + 'A',
				p->Information_[ Idx2 ]->StartTimeId_ + 13, p->Information_[ Idx2 ]->EndTimeId_ + 13 );
		}
		printf( "\n" );
	}
}

void WorldGen::GeneratorPeopleRoomsItems::GenerateClues()
{
	using namespace cpplinq;
	auto c2 = from_iterators( DB_.Information_.Internal().begin(), DB_.Information_.Internal().end() )
		>> join(
		from_iterators( DB_.Information_.Internal().begin(), DB_.Information_.Internal().end() ),
		[ ]( Information* const & c ) {return c->RoomId_; },
		[ ]( Information* const & ca ) {return ca->RoomId_; },
		[ ]( Information* const & c, Information* const & ca ) {return std::make_pair( c, ca ); } )
		>> where( [ ]( std::pair<Information*, Information*> const & p )
	{
		return ( p.first->PersonId_ < p.second->PersonId_ ) &&
			( p.first->RoomId_ == p.second->RoomId_ ) &&
			!( ( p.first->EndTimeId_ <= p.second->StartTimeId_ ) ||
			( p.first->StartTimeId_ >= p.second->EndTimeId_ )
			);
	} )
		>> to_vector();
	for ( int Idx = 0; Idx < c2.size(); ++Idx )
	{
		Information* infoA = c2[ Idx ].first;
		Information* infoB = c2[ Idx ].second;
		InfoForPlayer* data = DB_.PlayerInfo_.Create();

		int personACount = DB_.People_.GetItemById( infoA->PersonId_ )->Information_.Size();
		int personBCount = DB_.People_.GetItemById( infoB->PersonId_ )->Information_.Size();
		int infoGiver = infoA->PersonId_;
		int target = infoB->PersonId_;
		if ( personACount > personBCount )
		{
			infoGiver = infoB->PersonId_;
			target = infoA->PersonId_;
		}
		if ( personACount == personBCount )
		{
			if ( rand() % 2 == 1 )
			{
				infoGiver = infoB->PersonId_;
				target = infoA->PersonId_;
			}
		}
		data->PersonId_ = infoGiver;
		data->TargetId_ = target;
		data->StartTimeId_ = ( infoA->StartTimeId_ < infoB->StartTimeId_ ) ? infoB->StartTimeId_ : infoA->StartTimeId_;
		data->EndTimeId_ = ( infoA->EndTimeId_ < infoB->EndTimeId_ ) ? infoA->EndTimeId_ : infoB->EndTimeId_;
		data->RoomId_ = infoA->RoomId_;
		DB_.People_.GetItemById( data->PersonId_ )->Information_.Add( data );
	}

	for ( int Counter = 0; Counter < 3; ++Counter )
	{
		for ( int Idx = 0; Idx < DB_.PlayerInfo_.Size(); ++Idx )
		{
			InfoForPlayer* data = DB_.PlayerInfo_.GetItem( Idx );
			int personACount = DB_.People_.GetItemById( data->PersonId_ )->Information_.Size();
			int personBCount = DB_.People_.GetItemById( data->TargetId_ )->Information_.Size();

			if ( personACount > personBCount + 1 )
			{
				Person* pA = DB_.People_.GetItemById( data->PersonId_ );
				Person* pB = DB_.People_.GetItemById( data->TargetId_ );

				int t = data->PersonId_;
				data->PersonId_ = data->TargetId_;
				data->TargetId_ = t;
				auto Information = pB->Information_.GetInformation_();
				Information.push_back( data );
				for ( auto iter = Information.begin(); iter != Information.end(); ++iter )
				{
					if ( *iter == data )
					{
						Information.erase( iter );
						break;
					}
				}

			}
		}
	}/**/

}