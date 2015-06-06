#include "GaRoomComponent.h"

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
REFLECTION_DEFINE_DERIVED( GaRoomComponent );

void GaRoomComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaRoomComponent::Canvas_, bcRFF_TRANSIENT ),

		new ReField( "RoomName_", &GaRoomComponent::RoomName_, bcRFF_IMPORTER ),

	};

	using namespace std::placeholders;
	ReRegisterClass< GaRoomComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(
			{
				ScnComponentProcessFuncEntry(
					"Draw Object UI",
					ScnComponentPriority::DEFAULT_UPDATE,
					[]( const ScnComponentList& Components )
					{
						ImGui::Begin( "Room" );
						for( auto InComponent : Components )
						{
							GaRoomComponentRef Component( InComponent );

							std::string Text = std::string( "You are in the " ) + Component->RoomName_;
							ImGui::Text( Text.c_str() );
						}

						ImGui::End();
					} ),
			} ) );

}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaRoomComponent::GaRoomComponent():
	Canvas_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaRoomComponent::~GaRoomComponent()
{

}


//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaRoomComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaRoomComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
