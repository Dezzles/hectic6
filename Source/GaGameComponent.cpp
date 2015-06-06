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

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_BASIC( GaGameObject );
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


void GaGameComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaGameComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "GameState_", &GaGameComponent::GameState_, bcRFF_TRANSIENT ),	
		new ReField( "CurrentRoomEntity_", &GaGameComponent::CurrentRoomEntity_, bcRFF_TRANSIENT ),	
		new ReField( "ModalDialogEntity_", &GaGameComponent::ModalDialogEntity_, bcRFF_TRANSIENT ),	
		
		new ReField( "Room_", &GaGameComponent::Room_, bcRFF_IMPORTER ),	
		new ReField( "Rooms_", &GaGameComponent::Rooms_, bcRFF_IMPORTER ),	
		new ReField( "Objects_", &GaGameComponent::Objects_, bcRFF_IMPORTER ),	

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
	GameState_( GameState::IDLE )
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
			}
			else
			{
				ScnCore::pImpl()->removeEntity( ModalDialogEntity_ );
				ModalDialogEntity_ = nullptr;
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
			RoomName, "game", RoomName,
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
					Object.Object_, "game", Object.Object_,
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
			ModalName, "game", ModalName,
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
