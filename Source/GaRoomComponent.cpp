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
		new ReField( "Material_", &GaRoomComponent::Material_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Foreground_", &GaRoomComponent::Foreground_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Background_", &GaRoomComponent::Background_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

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

							// Draw!
							for( auto* MaterialComponent : Component->BgMaterialComponents_ )
							{
								auto Canvas = Component->Canvas_;
								Canvas->setMaterialComponent( MaterialComponent );
								Canvas->drawSprite( 
									MaVec2d( 0.0f, 0.0f ),
									MaVec2d( 1280.0f, 720.0f ), 
									0, 
									RsColour::WHITE, 0 );
							}

							for( auto* MaterialComponent : Component->FgMaterialComponents_ )
							{
								auto Canvas = Component->Canvas_;
								Canvas->setMaterialComponent( MaterialComponent );
								Canvas->drawSprite( 
									MaVec2d( 0.0f, 0.0f ),
									MaVec2d( 1280.0f, 720.0f ), 
									0, 
									RsColour::WHITE, 100 );
							}
						}

						ImGui::End();
					} ),
			} ) );

}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaRoomComponent::GaRoomComponent():
	Canvas_( nullptr ),
	Material_( nullptr )
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

	if( Material_ )
	{
		for( auto* Texture : Foreground_ )
		{
			auto MaterialComponent = Parent->attach< ScnMaterialComponent >( "material", Material_, ScnShaderPermutationFlags::MESH_STATIC_2D );
			MaterialComponent->setTexture( "aDiffuseTex", Texture );
			FgMaterialComponents_.emplace_back( MaterialComponent );
		}

		for( auto* Texture : Background_ )
		{
			auto MaterialComponent = Parent->attach< ScnMaterialComponent >( "material", Material_, ScnShaderPermutationFlags::MESH_STATIC_2D );
			MaterialComponent->setTexture( "aDiffuseTex", Texture );
			BgMaterialComponents_.emplace_back( MaterialComponent );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaRoomComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

}
