#include "GaGameComponent.h"
#include "GaModalComponent.h"

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
REFLECTION_DEFINE_DERIVED( GaGameComponent );

void GaGameObject::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Object_", &GaGameObject::Object_ ),
		new ReField( "Room_", &GaGameObject::Room_ ),
		new ReField( "InfoText_", &GaGameObject::InfoText_ ),
		new ReField( "Infos_", &GaGameObject::Infos_ ),
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


void GaGameComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaGameComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "GameState_", &GaGameComponent::GameState_, bcRFF_TRANSIENT ),	
		new ReField( "CurrentRoomEntity_", &GaGameComponent::CurrentRoomEntity_, bcRFF_TRANSIENT ),	
		new ReField( "ModalDialogEntity_", &GaGameComponent::ModalDialogEntity_, bcRFF_TRANSIENT ),	
		
		new ReField( "CharacterNames_", &GaGameComponent::CharacterNames_, bcRFF_IMPORTER ),

		new ReField( "Room_", &GaGameComponent::Room_, bcRFF_IMPORTER ),
		new ReField( "Rooms_", &GaGameComponent::Rooms_, bcRFF_IMPORTER ),	
		new ReField( "Objects_", &GaGameComponent::Objects_, bcRFF_IMPORTER ),	
		new ReField( "Solution_", &GaGameComponent::Solution_, bcRFF_IMPORTER ),
		new ReField( "AttemptedSolutionObjects_", &GaGameComponent::AttemptedSolutionObjects_, bcRFF_TRANSIENT ),
		new ReField( "CorrectSolutionObjects_", &GaGameComponent::CorrectSolutionObjects_, bcRFF_TRANSIENT ),
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
	WorldGen::Generator gen( 4, 4, 0x845efad7 );
	char buffer[ 2048 ];
	char buffer2[ 64 ];
	for ( int Idx = 0; Idx < gen.People_.Size(); ++Idx )
	{
		WorldGen::Person* person = gen.People_.GetItem( Idx );
		GaGameObject obj;
		sprintf(buffer, "PERSON_%d", person->Id_);
		obj.Object_ = buffer;
		sprintf(buffer, "ROOM_%d", Idx);
		obj.Room_ = buffer;

		int start = 0;
		for ( int Idx2 = 0; Idx2 < person->Information_.size(); ++Idx2 )
		{
			int timeLength = person->Information_[ Idx2 ]->EndTimeId_ - person->Information_[ Idx2 ]->StartTimeId_;
			start += sprintf( &buffer[start], "I was with PERSON_%d in the ROOM_%d at %d:00 for %d hour%s.\n", 
				person->Information_[ Idx2 ]->PersonId_ ,
				gen.Rooms_.GetItemById(person->Information_[ Idx2 ]->RoomId_ )->NormalRoomId_, 
				person->Information_[ Idx2 ]->StartTimeId_ + 11,
				timeLength, timeLength > 1 ? "s" : "");
			sprintf( buffer2, "PERSON_%d", person->Information_[ Idx2 ]->PersonId_ );
			obj.Infos_.push_back( buffer2 );
			sprintf( buffer2, "ROOM_%d", gen.Rooms_.GetItemById( person->Information_[ Idx2 ]->RoomId_ )->NormalRoomId_ );
			obj.Infos_.push_back( buffer2 );
		}
		obj.InfoText_ = buffer;
		Objects_.push_back(obj);
	}

	for ( int Idx = 0; Idx < gen.Rooms_.Size(); ++Idx )
	{
		WorldGen::Room* room = gen.Rooms_.GetItem( Idx );
		GaGameObject obj;
		sprintf( buffer, "DOOR_%dLeft", room->NormalRoomId_ );
		obj.Object_ = buffer;
		sprintf( buffer, "ROOM_", room->NormalRoomId_ );
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

		if ( Idx < gen.Rooms_.Size() - 1 )
		{
			GaGameObject obj;
			sprintf( buffer, "DOOR_%dRight", room->NormalRoomId_ );
			obj.Object_ = buffer;
			sprintf( buffer, "ROOM_", room->NormalRoomId_ );
			obj.Room_ = buffer;
			if ( Idx == 0 )
			{
				sprintf( buffer, "ROOM_%d", Idx + 1 );
				obj.Target_ = buffer;
				obj.Infos_.push_back( buffer );
			}
			Objects_.push_back( obj );

		}
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
									GaModalOption( Info, Info ) );
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
	
	// Spawn room entity.
	CurrentRoomEntity_ = ScnCore::pImpl()->spawnEntity( 
		ScnEntitySpawnParams( 
			RoomName, getParentEntity()->getBasis()->getOwner()->getName(), RoomName,
			MaMat4d(), getParentEntity() ) );
	BcAssert( CurrentRoomEntity_ );
	Room_ = *RoomName;
	
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
