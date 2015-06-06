#include "GaGameComponent.h"

#include "System/Scene/Rendering/ScnCanvasComponent.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsCore.h"

#include "System/Os/OsCore.h"

#include "System/SysKernel.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( GaGameComponent );

void GaGameComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaGameComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "GameState_", &GaGameComponent::GameState_, bcRFF_TRANSIENT ),	

		
	};

	using namespace std::placeholders;
	ReRegisterClass< GaGameComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(
			{
				ScnComponentProcessFuncEntry(
					"Set canvas projection",
					ScnComponentPriority::CANVAS_CLEAR + 1,
					std::bind( &GaGameComponent::setCanvasProjection, _1 ) ),
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

	// Spawn rooms.
	/*
	for( BcF32 X = -Width; X <= Width; X += 2.0f )
	{
		ScnEntitySpawnParams EntityParams = 
		{
			BcName( "SpriteEntity", Idx++ ), "sprite_test", "SpriteEntity",
			MaMat4d(),
			getParentEntity()
		};

		EntityParams.Transform_.translation( MaVec3d( 0.0f, 0.0f, X ) );
		ScnCore::pImpl()->spawnEntity( EntityParams );
	}*/

	// Spawn objects.
	// TODO.


}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}

//////////////////////////////////////////////////////////////////////////
// onMoveToRoom
eEvtReturn GaGameComponent::onMoveToRoom( EvtID ID, const EvtBaseEvent& InEvent )
{

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onOpenButlerDialog
eEvtReturn GaGameComponent::onOpenButlerDialog( EvtID ID, const EvtBaseEvent& InEvent )
{

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onOpenItemDialog
eEvtReturn GaGameComponent::onOpenItemDialog( EvtID ID, const EvtBaseEvent& InEvent )
{

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// setCanvasProjection
//static
void GaGameComponent::setCanvasProjection( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< GaGameComponent >() );
		auto* TestComponent = static_cast< GaGameComponent* >( Component.get() );

		OsClient* Client = OsCore::pImpl()->getClient( 0 );

		BcF32 PixelW = static_cast< BcF32 >( Client->getWidth() ) / 3.0f;
		BcF32 PixelH = static_cast< BcF32 >( Client->getHeight() ) / 3.0f;

		BcF32 EdgeL = floorf( -PixelW / 2.0f );
		BcF32 EdgeR = EdgeL + PixelW;
		BcF32 EdgeT = floorf( -PixelH / 2.0f );
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
}
