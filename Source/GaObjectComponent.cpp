#include "GaObjectComponent.h"
#include "GaGameComponent.h"

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
REFLECTION_DEFINE_DERIVED( GaObjectComponent );

void GaObjectComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaObjectComponent::Canvas_, bcRFF_TRANSIENT ),
		
		new ReField( "ObjectName_", &GaObjectComponent::ObjectName_, bcRFF_IMPORTER ),
		new ReField( "ObjectType_", &GaObjectComponent::ObjectType_, bcRFF_IMPORTER ),
		new ReField( "Target_", &GaObjectComponent::Target_, bcRFF_IMPORTER ),
	};

	using namespace std::placeholders;
	ReRegisterClass< GaObjectComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(
			{
				ScnComponentProcessFuncEntry(
					"Draw Object UI",
					ScnComponentPriority::DEFAULT_UPDATE,
					[]( const ScnComponentList& Components )
					{
						ImGui::Begin( "Objects" );

						for( auto InComponent : Components )
						{
							GaObjectComponentRef Component( InComponent );

							std::string ButtonTest = Component->ObjectType_ + ": " + Component->ObjectName_;

							if( ImGui::Button( ButtonTest.c_str() ) )
							{
								// Send message to parent to activate object appropriately.
								GaActionEvent Event;
								Event.SourceName_ = Component->ObjectName_;
								Event.SourceType_ = Component->ObjectType_;
								Event.Target_ = Component->Target_;
								InComponent->getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );
							}
						}

						ImGui::End();
					} ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaObjectComponent::GaObjectComponent():
	Canvas_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaObjectComponent::~GaObjectComponent()
{

}


//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaObjectComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaObjectComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
