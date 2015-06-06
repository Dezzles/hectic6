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
REFLECTION_DEFINE_BASIC( GaModalOption );
REFLECTION_DEFINE_BASIC( GaModalOptionGroup );
REFLECTION_DEFINE_DERIVED( GaModalComponent );

void GaModalOption::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Name_", &GaModalOption::Name_ ),
		new ReField( "Text_", &GaModalOption::Text_ ),
	};

	ReRegisterClass< GaModalOption >( Fields );
}


void GaModalOptionGroup::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Name_", &GaModalOptionGroup::Name_ ),
		new ReField( "Text_", &GaModalOptionGroup::Text_ ),
		new ReField( "Options_", &GaModalOptionGroup::Options_ ),
	};

	ReRegisterClass< GaModalOptionGroup >( Fields );
}


void GaModalComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &GaModalComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "CurrentOptionGroup_", &GaModalComponent::CurrentOptionGroup_, bcRFF_TRANSIENT ),

		new ReField( "OptionGroups_", &GaModalComponent::OptionGroups_, bcRFF_IMPORTER ),
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

							const auto& OptionGroup = Component->OptionGroups_[ Component->CurrentOptionGroup_ ];
							ImGui::Text( OptionGroup.Text_.c_str() );

							//for( const u)

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
	Canvas_( nullptr ),
	CurrentOptionGroup_( 0 )
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
void GaModalComponent::setup( const std::string& Text )
{
	GaModalOption Option;
	Option.Name_ = "CLOSE";
	Option.Text_ = "Ok, thanks!";

	GaModalOptionGroup OptionGroup;
	OptionGroup.Name_ = "MODAL";
	OptionGroup.Text_ = Text;
	OptionGroup.Options_.push_back( Option );

	OptionGroups_.push_back( OptionGroup );
}

//////////////////////////////////////////////////////////////////////////
// setup
void GaModalComponent::setup( const std::vector< GaModalOptionGroup >& OptionGroups )
{

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
