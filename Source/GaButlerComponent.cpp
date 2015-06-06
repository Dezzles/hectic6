#include "GaButlerComponent.h"

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
REFLECTION_DEFINE_DERIVED( GaButlerComponent );

void GaButlerComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaButlerComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "SelectionState_", &GaButlerComponent::SelectionState_, bcRFF_TRANSIENT ),
	};

	using namespace std::placeholders;
	ReRegisterClass< GaButlerComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(
			{
				ScnComponentProcessFuncEntry(
					"Butler update",
					ScnComponentPriority::DEFAULT_UPDATE,
					[]( const ScnComponentList& Components )
					{
						for( auto InComponent : Components )
						{
							GaButlerComponentRef Component( InComponent );

							ImGui::Begin( "Butler" );
							std::string Text = std::string( "You have all the information? Excellent" );
							ImGui::Text( Text.c_str() );
							ImGui::End();
						}

					} ),
			} ) );

}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaButlerComponent::GaButlerComponent():
	Canvas_( nullptr ),
	SelectionState_( SelectionState::IDLE )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaButlerComponent::~GaButlerComponent()
{

}


//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaButlerComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaButlerComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
