#include "GaModalComponent.h"
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
REFLECTION_DEFINE_DERIVED( GaModalComponent );

void GaModalComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaModalComponent::Canvas_, bcRFF_TRANSIENT ),

		new ReField( "Text_", &GaModalComponent::Text_, bcRFF_IMPORTER ),
		new ReField( "Info_", &GaModalComponent::Info_, bcRFF_IMPORTER ),
	};

	using namespace std::placeholders;
	ReRegisterClass< GaModalComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor(
			{
				ScnComponentProcessFuncEntry(
					"Draw Object UI",
					ScnComponentPriority::DEFAULT_UPDATE,
					[]( const ScnComponentList& Components )
					{
						for( auto InComponent : Components )
						{
							ImGui::Begin( "Modals" );
							GaModalComponentRef Component( InComponent );

							ImGui::Text( Component->Text_.c_str() );

							//std::string ButtonTest = Component->ObjectType_ + ": " + Component->ObjectName_;

							if( ImGui::Button( "Close" ) )
							{
								// Send message to parent to activate object appropriately.
								GaActionEvent Event;
								Event.Target_ = "CLOSE";
								InComponent->getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );
							}
							ImGui::End();
						}
					} ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaModalComponent::GaModalComponent():
	Canvas_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaModalComponent::~GaModalComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// setup
void GaModalComponent::setup( const std::string& Text, const std::vector< std::string >& Info )
{
	Text_ = Text;
	Info_ = Info;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaModalComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaModalComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
