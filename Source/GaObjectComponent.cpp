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

		new ReField( "Material_", &GaObjectComponent::Material_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Texture_", &GaObjectComponent::Texture_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

		new ReField( "Position_", &GaObjectComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Size_", &GaObjectComponent::Size_, bcRFF_IMPORTER ),
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
								Event.SourceName_ = Component->getParentEntity()->getName();
								Event.SourceType_ = Component->ObjectType_;
								Event.Target_ = Component->Target_;
								InComponent->getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );
							}

							// Draw!
							if( Component->MaterialComponent_ )
							{
								auto Canvas = Component->Canvas_;
								Canvas->setMaterialComponent( Component->MaterialComponent_ );
								Canvas->drawSprite( 
									Component->Position_,
									Component->Size_, 
									0, 
									RsColour::WHITE, 50 );
							}
						}

						ImGui::End();
					} ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaObjectComponent::GaObjectComponent():
	Canvas_( nullptr ),
	Material_( nullptr ),
	Texture_( nullptr ),
	MaterialComponent_( nullptr ),
	Position_( 0.0f, 0.0f ),
	Size_( 1280.0f, 720.0f )
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

	if( Material_ && Texture_ )
	{
		MaterialComponent_ = Parent->attach< ScnMaterialComponent >( "material", Material_, ScnShaderPermutationFlags::MESH_STATIC_2D );
		MaterialComponent_->setTexture( "aDiffuseTex", Texture_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaObjectComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}

//////////////////////////////////////////////////////////////////////////
// setup
//virtual
void GaObjectComponent::setup( 
		const std::string& Name, 
		const std::string& Type, 
		const std::string& Target,
		class ScnTexture* Texture,
		MaVec2d Position )
{
	BcAssert( isAttached() == BcFalse );
	ObjectName_ = Name;
	ObjectType_ = Type;
	Target_ = Target;
	if( Texture != nullptr )
	{
		Texture_ = Texture;
	}
}
