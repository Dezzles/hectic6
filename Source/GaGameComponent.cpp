#include "GaGameComponent.h"
#include "GaModalComponent.h"
#include "GaObjectComponent.h"
#include "GaRoomComponent.h"

#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFont.h"

#include "System/Scene/Sound/ScnSoundEmitter.h"

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

#define DEBUG_UI

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
		
		new ReField( "FontMaterial_", &GaGameComponent::FontMaterial_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Font_", &GaGameComponent::Font_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

		new ReField( "Music_", &GaGameComponent::Music_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

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
														
							ScnFontDrawParams Params = ScnFontDrawParams()
								.setAlignment( ScnFontAlignment::HCENTRE | ScnFontAlignment::VCENTRE )
								.setLayer( 80 )
								.setTextColour( RsColour::BLACK )
								.setSize( 40.0f )
								.setTextSettings( MaVec4d( 0.40f, 0.45f, -1.0f, -1.0f ) );

							BcName RoomName = TestComponent->Room_;

							// HACK.
							if( RoomName == "ROOM_LOBBY" )
							{
								TestComponent->FontComponent_->drawText( 
									TestComponent->Canvas_, 
									Params,
									MaVec2d( 0.0f, 0.0f ),
									MaVec2d( 1280.0f, 200.0f ),
									"The Private Dick\nby Amy, Dezzles, NeiloGD, and Music by Jared" );
							}
							else
							{							
								TestComponent->FontComponent_->drawText( 
									TestComponent->Canvas_, 
									Params,
									MaVec2d( 0.0f, 0.0f ),
									MaVec2d( 1280.0f, 100.0f ),
									TestComponent->Rooms_[ RoomName.getID() ].Text_ );
							}
#if 0
							ImGui::Begin( "Game" );
							for( auto InComponent : Components )
							{
								GaGameComponentRef Component( InComponent );

								ImGui::Text( "Unlocked info:" );
								for( const auto& Info : Component->Infos_ )
								{
									auto Obj = Component->findObject( Info.c_str() );
									ImGui::BulletText( "%s: (%s)", Info.c_str(), Obj ? Obj->InfoText_.c_str() : "" );
								}
							}

							ImGui::End();
#endif
						}
					} ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaGameComponent::GaGameComponent():
	Canvas_( nullptr ),
	GameState_( GameState::IDLE ),
	AttemptedSolutionObjects_( 0 ),
	CorrectSolutionObjects_( 0 ),
	Guessing_( BcFalse ),
	SetWin_( BcFalse ),
	SetLose_( BcFalse ),
	FontMaterial_( nullptr ),
	Font_( nullptr ),
	FontComponent_( nullptr ),
	Music_( nullptr )
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
static BcTimer Timer;
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	auto Emitter = getParentEntity()->getComponentAnyParentByType< ScnSoundEmitterComponent >();

	Emitter->play( Music_ );


	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();
	//0x845efad7 4,4 - Solvable puzzle.
	//0x12345678 4,4 - Solvable puzzle.
	//0x11111111 3,3 - Solvable puzzle.
	auto Seed = 0x845efad7;
	int totalHours = 4;
	int totalPeople = 4;

	Seed = Timer.time() * 100000.0f;
	
	// Randomise our room list.
	srand( Seed );
	for( int Idx = 0; Idx < Rooms_.size(); ++Idx )
	{
		auto TargetIdx = rand() % Rooms_.size();
		std::swap( Rooms_[ Idx ], Rooms_[ TargetIdx ] );
	}

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

		int RoomId = Idx;// % gen.Rooms_.Size();
		sprintf(buffer, "PERSON_%d", person->Id_);
		obj.Object_ = buffer;
		obj.Infos_.push_back( buffer );
		sprintf(buffer, "ROOM_%d", RoomId);
		obj.Room_ = buffer;
		RoomIds.insert( RoomId );

		int start = 0;

		start += sprintf( &buffer[start], "%s says:\n\n", 
			Characters_[ Idx ].Text_.c_str() );

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
			
			// If a door event, spawn room.
			if( Event.SourceType_ == "RESET_GAME" ||
				Event.Target_ == "RESET_GAME" )
			{
				ScnCore::pImpl()->removeEntity( getParentEntity() );

				exit(0);
			}

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
					useObject( Event.Target_ );
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

					if( SetWin_ )
					{
						std::vector< GaModalOptionGroup > OptionGroups = 
						{
							GaModalOptionGroup( "MODAL", "Thanks to your excellent detective skills we\nhave put away the murder!",
								{
									GaModalOption( "NEXT", "Just doin' my job." ),
									GaModalOption( "NEXT", "Elementry!" ),
									GaModalOption( "NEXT", "Justice served!" )
								} ),
							GaModalOptionGroup( "MODAL", "Thank you. Others may be in need of your service.",
								{
									GaModalOption( "RESET_GAME", "On my way!" )
								} ),
						};

						spawnModal( "MODAL", OptionGroups );
					}
					else if( SetLose_ )
					{
						std::vector< GaModalOptionGroup > OptionGroups = 
						{
							GaModalOptionGroup( "MODAL", "It turns out the evidence was not sufficient to convict!\nAlas, another criminal walks free again.\nThanks for nothing.",
								{
									GaModalOption( "NEXT", "I'm sorry." ),
									GaModalOption( "NEXT", "Oh well." ),
									GaModalOption( "NEXT", "I suck :(" ),
									GaModalOption( "NEXT", "I'm such a loser!" )
								} ),

							GaModalOptionGroup( "MODAL", "I know. Please don't let down the next person or you may\nend up a victim some day!",
								{
									GaModalOption( "NEXT", "Is that a threat?" ),
									GaModalOption( "RESET_GAME", "I hope not." ),
									GaModalOption( "RESET_GAME", "We'll see.." )

								} ),

							GaModalOptionGroup( "MODAL", "Don't hang around to find out.",
								{
									GaModalOption( "NEXT", "See you around." )
								} ),

							GaModalOptionGroup( "MODAL", "Don't trip on the way out.",
								{
									GaModalOption( "RESET_GAME", "I won't." ),
									GaModalOption( "NEXT", "*trip*" )
								} ),

							GaModalOptionGroup( "MODAL", "HAHAHAHAHAHAHAHAHAHA!!!!!",
								{
									GaModalOption( "RESET_GAME", ":(" )
								} ),
						};

						spawnModal( "MODAL", OptionGroups );
					}
				}

				if( Event.SourceType_ == "SELECTION" )
				{
					if( Event.Target_ == "RESET_ATTEMPTS" )
					{
						AttemptedSolutionObjects_ = 0;
						CorrectSolutionObjects_ = 0;
						Guessing_ = BcTrue;
					}
					else if( Guessing_ )
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
								Guessing_ = BcFalse;
								PSY_LOG( "WIN CONDITION!" );
								SetWin_ = BcTrue;
							}
							else
							{
								Guessing_ = BcFalse;
								PSY_LOG( "LOSE CONDITION!" );
								SetLose_ = BcTrue;
							}
						}
					}
				}
			}

			return evtRET_PASS;
		} );

	FontComponent_ = Parent->attach< ScnFontComponent >( "material", Font_, FontMaterial_ );

	Infos_.clear();
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
			bool IsPerson = false;
			MaVec2d Position( 0.0f, 0.0f );
			if( Object.Object_.substr( 0, Person.size() ) == Person ||
				Object.Object_.substr( 0, Butler.size() ) == Butler  )
			{
				PersonStart += PersonWidth;
				Position = PersonStart;
			}

			ScnEntityRef ObjectEntity;
			if( Object.Object_.substr( 0, Person.size() ) == Person )
			{
				auto PersonID = BcName( Object.Object_ ).getID();
				const auto& Person = Characters_[ PersonID ];
				ObjectEntity = ScnCore::pImpl()->spawnEntity( 
				ScnEntitySpawnParams( 
					Object.Object_, Person.Entity_,
					MaMat4d(), CurrentRoomEntity_ ) );

				auto ObjectComponent = ObjectEntity->getComponentByType< GaObjectComponent >();
				ObjectComponent->setup( 
					Object.Object_,
					ObjectComponent->ObjectType_, 
					Object.Object_,
					nullptr, Position );
			}
			else
			{
				ObjectEntity = ScnCore::pImpl()->spawnEntity( 
					ScnEntitySpawnParams( 
						Object.Object_, getParentEntity()->getBasis()->getOwner()->getName(), Object.Object_,
						MaMat4d(), CurrentRoomEntity_ ) );
			}
			BcAssert( ObjectEntity );
			

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
	else
	{
		BcBreakpoint;
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
