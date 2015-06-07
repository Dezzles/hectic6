#include "GaModalComponent.h"
#include "GaGameComponent.h"

#include "System/Scene/Rendering/ScnCanvasComponent.h"
#include "System/Scene/Rendering/ScnFont.h"

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

		new ReField( "Material_", &GaModalComponent::Material_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "FontMaterial_", &GaModalComponent::FontMaterial_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Texture_", &GaModalComponent::Texture_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "TextureButton_", &GaModalComponent::TextureButton_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Font_", &GaModalComponent::Font_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),

		new ReField( "MaterialComponent_", &GaModalComponent::MaterialComponent_, bcRFF_TRANSIENT ),
		new ReField( "FontComponent_", &GaModalComponent::FontComponent_, bcRFF_TRANSIENT ),
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

							if( Component->CurrentOptionGroup_ < Component->OptionGroups_.size() )
							{
								const auto& OptionGroup = Component->OptionGroups_[ Component->CurrentOptionGroup_ ];
								ImGui::Text( OptionGroup.Text_.c_str() );

								bool Clicked = false;
								for( const auto& Option : OptionGroup.Options_ )
								{
									if( ImGui::Button( Option.Text_.c_str() ) )
									{
										GaActionEvent Event;
										Event.SourceType_ = "SELECTION";
										Event.SourceName_ = Option.Name_;
										Event.Target_ = Option.Name_;
										InComponent->getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );

										Clicked = true;
									}							

									ImGui::SameLine();
								}

								if( Clicked )
								{
									Component->CurrentOptionGroup_++;
									if( Component->CurrentOptionGroup_ >= Component->OptionGroups_.size() )
									{
										GaActionEvent Event;
										Event.SourceType_ = "CLOSE";
										InComponent->getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );
									}
								}

								// Draw!
								if( Component->MaterialComponent_ )
								{
									auto Pos = MaVec2d( 640.0f, 200.0f );
									auto Size = MaVec2d( Component->Texture_->getWidth(), Component->Texture_->getHeight() );
									auto Canvas = Component->Canvas_;
									Canvas->setMaterialComponent( Component->MaterialComponent_ );
									Canvas->drawSpriteCentered( 
										Pos,
										Size,
										0, 
										RsColour::WHITE, 100 );

									ScnFontDrawParams Params = ScnFontDrawParams()
										.setAlignment( ScnFontAlignment::HCENTRE | ScnFontAlignment::VCENTRE )
										.setLayer( 101 )
										.setTextColour( RsColour::BLACK )
										.setSize( 24.0f );

									Pos -= Size * 0.5f;

									Component->FontComponent_->drawText( 
										Canvas, 
										Params,
										Pos,
										Size,
										OptionGroup.Text_ );

									if( Component->CurrentOptionGroup_ < Component->OptionGroups_.size() )
									{
										const auto& OptionGroup = Component->OptionGroups_[ Component->CurrentOptionGroup_ ];

										MaVec2d Position( 640.0f, 500.0f );
										MaVec2d Size( Component->TextureButton_->getWidth(), Component->TextureButton_->getHeight() );
										MaVec2d TotalSize( Size.x() * OptionGroup.Options_.size(), Size.y() );

										Position -= MaVec2d( TotalSize.x() * 0.5f, 0.0f );

										for( BcU32 ButtonIdx = 0; ButtonIdx < OptionGroup.Options_.size(); ++ButtonIdx )
										{
											const auto& ButtonOption = OptionGroup.Options_[ ButtonIdx ];
											auto NewPosition = Position;

											Canvas->setMaterialComponent( Component->ButtonMaterialComponent_ );
											Canvas->drawSprite( NewPosition, Size, 0, RsColour::WHITE, 100 );
										
											Component->FontComponent_->drawText( 
												Canvas, 
												Params,
												NewPosition,
												Size,
												ButtonOption.Text_ );
									
											Position += MaVec2d( 300.0f, 0.0f );
										}
									}
								}
							}
							Component->HadClick_ = BcFalse;
							ImGui::End();
						}
					} ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaModalComponent::GaModalComponent():
	Canvas_( nullptr ),
	CurrentOptionGroup_( 0 ),
	Material_( nullptr ),
	FontMaterial_( nullptr ),
	Texture_( nullptr ),
	TextureButton_( nullptr ),
	Font_( nullptr ),
	MaterialComponent_( nullptr ),
	ButtonMaterialComponent_( nullptr ),
	FontComponent_( nullptr ),
	ClickPosition_( 0.0f, 0.0f ),
	HadClick_( BcFalse )
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
#define TEST_GROUPS ( 0 )
#if TEST_GROUPS
	{
		GaModalOptionGroup OptionGroup;
		OptionGroup.Name_ = "MODAL";
		OptionGroup.Text_ = Text;

		{
			GaModalOption Option;
			Option.Name_ = "SOMETHING";
			Option.Text_ = "Tell me more!";
			OptionGroup.Options_.push_back( Option );
		}

		OptionGroups_.push_back( OptionGroup );
	}
#endif

	{
		GaModalOptionGroup OptionGroup;
		OptionGroup.Name_ = "MODAL";
		OptionGroup.Text_ = Text;

		{
			GaModalOption Option;
			Option.Name_ = "CLOSE";
			Option.Text_ = "Ok, thanks!";
			OptionGroup.Options_.push_back( Option );
		}
#if TEST_GROUPS
		{
			GaModalOption Option;
			Option.Name_ = "CLOSE";
			Option.Text_ = "I might chat to you later.";
			OptionGroup.Options_.push_back( Option );
		}

		{
			GaModalOption Option;
			Option.Name_ = "CLOSE";
			Option.Text_ = "You've been a great help.";
			OptionGroup.Options_.push_back( Option );
		}
#endif

		OptionGroups_.push_back( OptionGroup );
	}

#undef TEST_GROUPS
}

//////////////////////////////////////////////////////////////////////////
// setup
void GaModalComponent::setup( const std::vector< GaModalOptionGroup >& OptionGroups )
{
	OptionGroups_ = OptionGroups;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaModalComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Canvas_ = getParentEntity()->getComponentAnyParentByType< ScnCanvasComponent >();

	MaterialComponent_ = Parent->attach< ScnMaterialComponent >( "material", Material_, ScnShaderPermutationFlags::MESH_STATIC_2D );
	MaterialComponent_->setTexture( "aDiffuseTex", Texture_ );

	ButtonMaterialComponent_ = Parent->attach< ScnMaterialComponent >( "material", Material_, ScnShaderPermutationFlags::MESH_STATIC_2D );
	ButtonMaterialComponent_->setTexture( "aDiffuseTex", TextureButton_ );
	
	FontComponent_ = Parent->attach< ScnFontComponent >( "material", Font_, FontMaterial_ );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, this, 
		[ this ]( EvtID, const EvtBaseEvent& InEvent ) -> eEvtReturn
		{
			const auto& Event = InEvent.get< OsEventInputMouse >();
			ClickPosition_ = MaVec2d( Event.MouseX_, Event.MouseY_ );

			if( CurrentOptionGroup_ < OptionGroups_.size() )
			{
				const auto& OptionGroup = OptionGroups_[ CurrentOptionGroup_ ];

				MaVec2d Position( 640.0f, 500.0f );
				MaVec2d Size( TextureButton_->getWidth(), TextureButton_->getHeight() );
				MaVec2d TotalSize( Size.x() * OptionGroup.Options_.size(), Size.y() );

				Position -= MaVec2d( TotalSize.x() * 0.5f, 0.0f );
				bool Clicked = false;
				for( BcU32 ButtonIdx = 0; ButtonIdx < OptionGroup.Options_.size(); ++ButtonIdx )
				{
					const auto& ButtonOption = OptionGroup.Options_[ ButtonIdx ];
					auto NewPosition = Position;

					auto HotspotEnd = NewPosition + Size;
					if( Event.MouseX_ > NewPosition.x() && Event.MouseX_ < HotspotEnd.x() &&
						Event.MouseY_ > NewPosition.y() && Event.MouseY_ < HotspotEnd.y() )
					{
						GaActionEvent Event;
						Event.SourceType_ = "SELECTION";
						Event.SourceName_ = ButtonOption.Name_;
						Event.Target_ = ButtonOption.Name_;
						getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );

						Clicked = true;
					}
							
					if( Clicked )
					{
						CurrentOptionGroup_++;
						if( CurrentOptionGroup_ >= OptionGroups_.size() )
						{
							GaActionEvent Event;
							Event.SourceType_ = "CLOSE";
							getParentEntity()->publish( gaEVT_FLOW_ACTION, Event, BcFalse );
						}
						return evtRET_BLOCK;
					}
									
					Position += MaVec2d( 300.0f, 0.0f );
				}
			}

			return evtRET_PASS;
		}, true );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaModalComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	OsCore::pImpl()->unsubscribeAll( this );
}
