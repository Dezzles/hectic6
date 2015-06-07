#include "GaGameComponent.h"
#include "GaModalComponent.h"
#include "GaObjectComponent.h"
#include "GaRoomComponent.h"

#include "System/Scene/Rendering/ScnCanvasComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"
#include "System/Debug/DsImGui.h"

#include "System/Os/OsCore.h"

#include "System/SysKernel.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"
#include "WorldGen/Generator.h"
//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_BASIC( GaGameObject );
REFLECTION_DEFINE_BASIC( GaSolutionObject );
REFLECTION_DEFINE_BASIC( GaRoomObject );
REFLECTION_DEFINE_DERIVED( GaGameComponent );

void GaGameObject::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Object_", &GaGameObject::Object_ ),
		new ReField( "Room_", &GaGameObject::Room_ ),
		new ReField( "InfoText_", &GaGameObject::InfoText_ ),
		new ReField( "Infos_", &GaGameObject::Infos_ ),

		new ReField( "Target_", &GaGameObject::Target_ ),
	};

	ReRegisterClass< GaGameObject >( Fields );
}


void GaSolutionObject::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Type_", &GaSolutionObject::Type_ ),
		new ReField( "Name_", &GaSolutionObject::Name_ ),
		new ReField( "MissingMessage_", &GaSolutionObject::MissingMessage_ ),
		new ReField( "Question_", &GaSolutionObject::Question_ ),
	};

	ReRegisterClass< GaSolutionObject >( Fields );
}


void GaRoomObject::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Entity_", &GaRoomObject::Entity_, bcRFF_SHALLOW_COPY ),
		new ReField( "Text_", &GaRoomObject::Text_ ),
		new ReField( "Verbs_", &GaRoomObject::Verbs_ ),
	};

	ReRegisterClass< GaRoomObject >( Fields );
}


void GaCharacterObject::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Entity_", &GaCharacterObject::Entity_, bcRFF_SHALLOW_COPY ),
		new ReField( "Text_", &GaCharacterObject::Text_ ),
		new ReField( "ValidNames_", &GaCharacterObject::ValidNames_ ),
	};

	ReRegisterClass< GaCharacterObject >( Fields );
}

//#define DEBUG_UI

void GaGameComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaGameComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "GameState_", &GaGameComponent::GameState_, bcRFF_TRANSIENT ),	
		new ReField( "CurrentRoomEntity_", &GaGameComponent::CurrentRoomEntity_, bcRFF_TRANSIENT ),	
		new ReField( "ModalDialogEntity_", &GaGameComponent::ModalDialogEntity_, bcRFF_TRANSIENT ),	
		
		new ReField( "CharacterNames_", &GaGameComponent::CharacterNames_, bcRFF_IMPORTER ),

		new ReField( "Rooms_", &GaGameComponent::Rooms_, bcRFF_IMPORTER ),	
		new ReField( "Characters_", &GaGameComponent::Characters_, bcRFF_IMPORTER ),	

		new ReField( "Room_", &GaGameComponent::Room_, bcRFF_IMPORTER ),
		new ReField( "Objects_", &GaGameComponent::Objects_, bcRFF_IMPORTER ),	
		new ReField( "Solution_", &GaGameComponent::Solution_, bcRFF_IMPORTER ),
		new ReField( "AttemptedSolutionObjects_", &GaGameComponent::AttemptedSolutionObjects_, bcRFF_TRANSIENT ),
		new ReField( "CorrectSolutionObjects_", &GaGameComponent::CorrectSolutionObjects_, bcRFF_TRANSIENT ),
		new ReField( "IDTextMapping_", &GaGameComponent::IDTextMapping_, bcRFF_TRANSIENT ),
	};

	using namespace std::placeholders;
	ReRegisterClass< GaGameComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(
			{
				ScnComponentProcessFuncEntry(
					"Set canvas projection",
					ScnComponentPriority::CANVAS_CLEAR + 1,
					[]( const ScnComponentList& Components )
					{
						for( auto Component : Components )
						{
							BcAssert( Component->isTypeOf< GaGameComponent >() );
							auto* TestComponent = static_cast< GaGameComponent* >( Component.get() );

							OsClient* Client = OsCore::pImpl()->getClient( 0 );

							BcF32 PixelW = static_cast< BcF32 >( Client->getWidth() );
							BcF32 PixelH = static_cast< BcF32 >( Client->getHeight() );

#if 0 // 0,0 centre.
							BcF32 EdgeL = floorf( -PixelW / 2.0f );
							BcF32 EdgeT = floorf( -PixelH / 2.0f );
#else // 0,0 top left.
							BcF32 EdgeL = 0.0f;
							BcF32 EdgeT = 0.0f;
#endif

							BcF32 EdgeR = EdgeL + PixelW;
							BcF32 EdgeB = EdgeT + PixelH;

							MaMat4d Projection;
							Projection.orthoProjection( 
								EdgeL, 
								EdgeR, 
								EdgeB, 
								EdgeT, 
								-1.0f, 1.0f );
							MaMat4d InvProjection = Projection;
							InvProjection.inverse();

							TestComponent->Canvas_->clear();
							TestComponent->Canvas_->pushMatrix( Projection );
						}
					} ),
#ifdef DEBUG_UI
				ScnComponentProcessFuncEntry(
					"Unlocked components",
					ScnComponentPriority::DEFAULT_UPDATE + 1,
					[]( const ScnComponentList& Components )
					{
						ImGui::Begin( "Game" );
						for( auto InComponent : Components )
						{
							GaGameComponentRef Component( InComponent );

							ImGui::Text( "Unlocked info:" );
							for( const auto& Info : Component->Infos_ )
							{
								ImGui::BulletText( Info.c_str() );
							}
						}

						ImGui::End();
					} ),
#endif
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaGameComponent::GaGameComponent():
	Canvas_( nullptr ),
	GameState_( GameState::IDLE ),
	AttemptedSolutionObjects_( 0 ),
	CorrectSolutionObjects_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaGameComponent::~GaGameComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();
	//0x845efad7 4,4 - Solvable puzzle.
	//0x12345678 4,4 - Solvable puzzle.
	//0x11111111 3,3 - Solvable puzzle.
	auto Seed = 0x845efad7;
	int totalHours = 4;
	int totalPeople = 4;

#if PSY_PRODUCTION
	static BcTimer Timer;
	Seed = Timer.time();

#endif
	
#if 0
	// Randomise our room list.
	srand( Seed );
	for( int Idx = 0; Idx < Rooms_.size(); ++Idx )
	{
		auto TargetIdx = rand() % Rooms_.size();
		std::swap( Rooms_[ Idx ], Rooms_[ TargetIdx ] );
	}
#endif

	std::swap( Rooms_[ 0 ], Rooms_[ 7 ] );

	// Randomise our character list.
	for( int Idx = 0; Idx < Characters_.size(); ++Idx )
	{
		auto TargetIdx = rand() % Characters_.size();
		std::swap( Characters_[ Idx ], Characters_[ TargetIdx ] );

		auto NameIdx = rand() % Characters_[ Idx ].ValidNames_.size();
		Characters_[ Idx ].Text_ = Characters_[ Idx ].ValidNames_[ NameIdx ];
	}

	// Randomly select name.
	for( int Idx = 0; Idx < Characters_.size(); ++Idx )
	{
		auto NameIdx = rand() % Characters_[ Idx ].ValidNames_.size();
		Characters_[ Idx ].Text_ = Characters_[ Idx ].ValidNames_[ NameIdx ];

		IDTextMapping_[ *BcName( "PERSON", Idx ) ] = Characters_[ Idx ].Text_;
	}

	BcAssert( Rooms_.size() > 0 );
	BcAssert( Characters_.size() > 0 );

	int startTimeOffset = 11;
	
	// Gen the world.
	WorldGen::Generator gen( totalPeople, totalHours, Seed );
	char buffer[ 2048 ];
	char buffer2[ 64 ];

	for( int hourOffset = 0; hourOffset < totalHours; ++hourOffset )
	{
		int hour = startTimeOffset + hourOffset;
				
		sprintf( buffer, "TIME_%d", hour );
		Infos_.insert( buffer );

		sprintf( buffer2, "%d:00", hour );
		IDTextMapping_[ buffer ] = buffer2;
	}

	// solution.
	sprintf( buffer, "PERSON_%d", gen.Murder_.PersonId_ );
	Infos_.insert( buffer );
	Solution_[ 0 ].Name_ = buffer;
	
	sprintf( buffer, "TIME_%d", startTimeOffset + gen.Murder_.TimeId_ );
	Infos_.insert( buffer );
	Solution_[ 1 ].Name_ = buffer;

	std::set< int > RoomIds;
	for ( int Idx = 0; Idx < gen.People_.Size(); ++Idx )
	{
		WorldGen::Person* person = gen.People_.GetItem( Idx );
		GaGameObject obj;

		int RoomId = Idx % gen.Rooms_.Size();
		sprintf(buffer, "PERSON_%d", person->Id_);
		obj.Object_ = buffer;
		sprintf(buffer, "ROOM_%d", RoomId);
		obj.Room_ = buffer;
		RoomIds.insert( RoomId );

		int start = 0;
		for ( int Idx2 = 0; Idx2 < person->Information_.size(); ++Idx2 )
		{
			int timeLength = person->Information_[ Idx2 ]->EndTimeId_ - person->Information_[ Idx2 ]->StartTimeId_;
			int InfoRoomId = gen.Rooms_.GetItemById(person->Information_[ Idx2 ]->RoomId_ )->NormalRoomId_;
			int targetId = person->Information_[ Idx2 ]->TargetId_;
			std::string targetName = Characters_[ targetId ].Text_;
			std::string roomName = Rooms_[ InfoRoomId ].Text_;

			start += sprintf( &buffer[start], "I was with %s in the %s at %d:00 for %d hour%s.\n", 
				targetName.c_str(),
				roomName.c_str(), 
				person->Information_[ Idx2 ]->StartTimeId_ + startTimeOffset,
				timeLength, timeLength > 1 ? "s" : "");

			sprintf( buffer2, "PERSON_%d", targetId );
			obj.Infos_.push_back( buffer2 );
			
			sprintf( buffer2, "ROOM_%d", gen.Rooms_.GetItemById( person->Information_[ Idx2 ]->RoomId_ )->NormalRoomId_ );
			obj.Infos_.push_back( buffer2 );

			// Add time infos.
			for( int hourOffset = 0; hourOffset < timeLength; ++hourOffset )
			{
				int hour = person->Information_[ Idx2 ]->StartTimeId_ + startTimeOffset + hourOffset;
				
				sprintf( buffer2, "TIME_%d", hour );
				obj.Infos_.push_back( buffer2 );
			}
			
		}
		obj.InfoText_ = buffer;
		Objects_.push_back(obj);
	}

	int Idx = 0;
	for ( auto RoomId : RoomIds )
	{
		GaGameObject obj;
		sprintf( buffer, "DOOR_LEFT", RoomId );
		obj.Object_ = buffer;
		sprintf( buffer, "ROOM_%d", RoomId );
		obj.Room_= buffer;
		if ( Idx == 0 )
		{
			obj.Target_ = "ROOM_LOBBY";
			obj.Infos_.push_back( "ROOM_LOBBY" );
		}
		else
		{
			sprintf(buffer, "ROOM_%d", Idx - 1);
			obj.Target_ = buffer;
			obj.Infos_.push_back( buffer );
		}
		Objects_.push_back(obj);

		if ( Idx < RoomIds.size() - 1 )
		{
			GaGameObject obj;
			sprintf( buffer, "DOOR_RIGHT", RoomId );
			obj.Object_ = buffer;
			sprintf( buffer, "ROOM_%d", RoomId );
			obj.Room_ = buffer;

			sprintf( buffer, "ROOM_%d", Idx + 1 );
			obj.Target_ = buffer;
			obj.Infos_.push_back( buffer );

			Objects_.push_back( obj );

		}

		++Idx;
	}

	
	// Subscribe to events
	getParentEntity()->subscribe( gaEVT_FLOW_ACTION, this,
		[ this ]( EvtID ID, const EvtBaseEvent& InEvent )
		{
			const auto& Event = InEvent.get< GaActionEvent >();
			
			// No modal dialog, then don't handle events.
			if( ModalDialogEntity_ == nullptr )
			{
				// If a door event, spawn room.
				if( Event.SourceType_ == "DOOR" )
				{
					useObject( Event.SourceName_ );
					spawnRoom( Event.Target_ );
				}

				// If a person event, spawn a modal.
				if( Event.SourceType_ == "PERSON" )
				{
					useObject( Event.SourceName_ );
					spawnModal( "MODAL", Event.Target_ );
				}

				// If a person event, spawn a modal.
				if( Event.SourceType_ == "BUTLER" )
				{
					std::vector< GaModalOptionGroup > OptionGroups = 
					{
						GaModalOptionGroup( "MODAL", "Do you think you've solved it?",
							{
								GaModalOption( "RESET_ATTEMPTS", "Yes!" ),
								GaModalOption( "CLOSE", "No" )
							} ),
					};

					// Reset correct.
					AttemptedSolutionObjects_ = 0;
					CorrectSolutionObjects_ = 0;

					//
					for( const auto& SolutionObject : Solution_ )
					{
						bool HaveSolutionObject = false;

						auto ModalOption = GaModalOptionGroup( "MODAL", SolutionObject.Question_, {} );

						for( const std::string& Info : Infos_ )
						{
							if( Info.substr( 0, SolutionObject.Type_.size() ) == SolutionObject.Type_ )
							{
								HaveSolutionObject = true;

								ModalOption.Options_.emplace_back( 
									GaModalOption( Info, IDTextMapping_[ Info ] ) );
							}
						}
						
						if( HaveSolutionObject == false )
						{
							OptionGroups.clear();
							OptionGroups.emplace_back( 
								GaModalOptionGroup( "MODAL", SolutionObject.MissingMessage_,
								{
									GaModalOption( "CLOSE", "Ok." ),
								} ) );
						}
						else
						{
							OptionGroups.emplace_back( ModalOption );
						}
					}

					useObject( Event.SourceName_ );
					spawnModal( "MODAL", OptionGroups );
				}
			}
			else
			{
				if( Event.SourceType_ == "CLOSE" ||
					Event.Target_ == "CLOSE" )
				{
					ScnCore::pImpl()->removeEntity( ModalDialogEntity_ );
					ModalDialogEntity_ = nullptr;
				}

				if( Event.SourceType_ == "SELECTION" )
				{
					if( Event.Target_ == "RESET_ATTEMPTS" )
					{
						AttemptedSolutionObjects_ = 0;
						CorrectSolutionObjects_ = 0;
					}
					else
					{
						// Test for solution.
						auto SolutionIt = std::find_if( Solution_.begin(), Solution_.end(),
							[ this, &Event ]( const GaSolutionObject& Solution )
							{
								if( Event.Target_ == Solution.Name_ )
								{
									return true;
								}
								return false;
							} );
						AttemptedSolutionObjects_++;
						if( SolutionIt != Solution_.end() )
						{
							CorrectSolutionObjects_++;
						}

						if( AttemptedSolutionObjects_ == Solution_.size() )
						{
							if( CorrectSolutionObjects_ == AttemptedSolutionObjects_ )
							{
								PSY_LOG( "WIN CONDITION!" );
							}
							else
							{
								PSY_LOG( "LOSE CONDITION!" );
							}
						}
					}
				}
			}

			return evtRET_PASS;
		} );

	useObject( Room_ );
	spawnRoom( Room_ );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	getParentEntity()->unsubscribeAll( this );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// spawnRoom
void GaGameComponent::spawnRoom( const BcName& RoomName )
{
	// Destroy old room.
	if( CurrentRoomEntity_ )
	{
		ScnCore::pImpl()->removeEntity( CurrentRoomEntity_ );
	}

	// Get room idx.
	Room_ = *RoomName;
	auto RoomID = RoomName.getID();

	// Spawn room entity.
	if( RoomID == BcErrorCode )
	{
		// lobby.
		CurrentRoomEntity_ = ScnCore::pImpl()->spawnEntity( 
			ScnEntitySpawnParams( 
				RoomName, getParentEntity()->getBasis()->getOwner()->getName(), RoomName,
				MaMat4d(), getParentEntity() ) );
	}
	else
	{
		const auto Room = Rooms_[ RoomID ];
		CurrentRoomEntity_ = ScnCore::pImpl()->spawnEntity( 
			ScnEntitySpawnParams( 
				RoomName, Room.Entity_,
				MaMat4d(), getParentEntity() ) );

		CurrentRoomEntity_->getComponentByType< GaRoomComponent >()->RoomName_ = Rooms_[ RoomID ].Text_;
	}
	BcAssert( CurrentRoomEntity_ );

	// Count people in room.
	std::string Person = "PERSON_";
	std::string Butler = "BUTLER_";
	BcU32 NoofPeopleInRoom = 0;
	BcU32 CurrPeopleInRoom = 0;
	for( const auto& Object : Objects_ )
	{
		// If object's location is the room.
		if( Object.Room_ == Room_ )
		{
			if( Object.Object_.substr( 0, Person.size() ) == Person ||
				Object.Object_.substr( 0, Butler.size() ) == Butler  )
			{
				NoofPeopleInRoom++;
			}
		}
	}

	MaVec2d PersonStart( 0.0f, 0.0f );
	MaVec2d PersonWidth( 270.0f, 0.0f );

	if( NoofPeopleInRoom == 1 )
	{
		PersonStart = MaVec2d( 0.0f, 200.0f );
		PersonWidth = MaVec2d( 270.0f, 0.0f );
	}
	else if( NoofPeopleInRoom == 2 )
	{
		PersonStart = MaVec2d( 0.0f, 200.0f );
		PersonWidth = MaVec2d( 270.0f, 0.0f );
	}
	else if( NoofPeopleInRoom == 3 )
	{
		PersonStart = MaVec2d( 0.0f, 200.0f );
		PersonWidth = MaVec2d( 270.0f, 0.0f );
	}
	else if( NoofPeopleInRoom == 4 )
	{
		PersonStart = MaVec2d( 0.0f, 200.0f );
		PersonWidth = MaVec2d( 270.0f, 0.0f );
	}

	PersonStart += ( PersonWidth * ( 4 - NoofPeopleInRoom ) ) / 2.0f - MaVec2d( 150.0f, 0.0f );

	// Spawn objects for room.
	for( const auto& Object : Objects_ )
	{
		// If object's location is the room.
		if( Object.Room_ == Room_ )
		{
			auto ObjectEntity = ScnCore::pImpl()->spawnEntity( 
				ScnEntitySpawnParams( 
					Object.Object_, getParentEntity()->getBasis()->getOwner()->getName(), Object.Object_,
					MaMat4d(), CurrentRoomEntity_ ) );
			BcAssert( ObjectEntity );

			MaVec2d Position( 0.0f, 0.0f );
			if( Object.Object_.substr( 0, Person.size() ) == Person ||
				Object.Object_.substr( 0, Butler.size() ) == Butler  )
			{
				PersonStart += PersonWidth;
				Position = PersonStart;
			}

			// Override target.
			if( !Object.Target_.empty() )
			{
				auto ObjectComponent = ObjectEntity->getComponentByType< GaObjectComponent >();
				ObjectComponent->setup( 
					Object.Object_, 
					ObjectComponent->ObjectType_, 
					Object.Target_, 
					nullptr, Position );
			}
			else
			{
				auto ObjectComponent = ObjectEntity->getComponentByType< GaObjectComponent >();
				ObjectComponent->setup( 
					ObjectComponent->ObjectName_, 
					ObjectComponent->ObjectType_, 
					ObjectComponent->Target_,
					nullptr, Position );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// spawnModal
void GaGameComponent::spawnModal( const BcName& ModalName, const BcName& Target )
{
	// Destroy old modal.
	if( ModalDialogEntity_ )
	{
		ScnCore::pImpl()->removeEntity( ModalDialogEntity_ );
	}

	// Spawn modal entity.
	ModalDialogEntity_ = ScnCore::pImpl()->spawnEntity( 
		ScnEntitySpawnParams( 
			ModalName, getParentEntity()->getBasis()->getOwner()->getName(), ModalName,
			MaMat4d(), getParentEntity() ) );
	BcAssert( ModalDialogEntity_ );

	// Setup modal.
	auto Modal = ModalDialogEntity_->getComponentByType< GaModalComponent >();
	BcAssert( Modal );

	if( auto* FoundObject = findObject( Target ) )
	{
		Modal->setup( FoundObject->InfoText_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// spawnModal
void GaGameComponent::spawnModal( const BcName& ModalName, const std::vector< GaModalOptionGroup >& OptionGroups )
{
	// Destroy old modal.
	if( ModalDialogEntity_ )
	{
		ScnCore::pImpl()->removeEntity( ModalDialogEntity_ );
	}

	// Spawn modal entity.
	ModalDialogEntity_ = ScnCore::pImpl()->spawnEntity( 
		ScnEntitySpawnParams( 
			ModalName, getParentEntity()->getBasis()->getOwner()->getName(), ModalName,
			MaMat4d(), getParentEntity() ) );
	BcAssert( ModalDialogEntity_ );

	// Setup modal.
	auto Modal = ModalDialogEntity_->getComponentByType< GaModalComponent >();
	BcAssert( Modal );
	Modal->setup( OptionGroups );
}

//////////////////////////////////////////////////////////////////////////
// useObject
void GaGameComponent::useObject( const BcName& ObjectName )
{
	Infos_.insert( *ObjectName );
	if( auto* FoundObject = findObject( ObjectName ) )
	{
		for( const auto& InfoName : FoundObject->Infos_ )
		{
			Infos_.insert( InfoName );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// findObject
GaGameObject* GaGameComponent::findObject( const BcName& ObjectName )
{
	for( auto& Object : Objects_ )
	{
		if( Object.Object_ == *ObjectName )
		{
			return &Object;
		}
	}

	return nullptr;
}
