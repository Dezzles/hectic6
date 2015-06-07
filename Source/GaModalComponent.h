#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaModalComponentRef
typedef ReObjectRef< class GaModalComponent > GaModalComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaModalOption
struct GaModalOption
{
	REFLECTION_DECLARE_BASIC( GaModalOption );
	GaModalOption(){}
	GaModalOption( const std::string& Name, const std::string& Text ):
		Name_( Name ),
		Text_( Text )
	{}

	std::string Name_;
	std::string Text_;
};

struct GaModalOptionGroup
{
	REFLECTION_DECLARE_BASIC( GaModalOptionGroup );
	GaModalOptionGroup(){}
	GaModalOptionGroup( 
			const std::string& Name, const std::string& Text, 
			const std::vector< GaModalOption >& Options ):
		Name_( Name ),
		Text_( Text ),
		Options_( Options )
	{}

	std::string Name_;
	std::string Text_;
	std::vector< GaModalOption > Options_;
};

//////////////////////////////////////////////////////////////////////////
// GaModalComponent
class GaModalComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaModalComponent, ScnComponent );

	GaModalComponent();
	virtual ~GaModalComponent();

	void setup( const std::string& Text );
	void setup( const std::vector< GaModalOptionGroup >& OptionGroups );

	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );


private:
	static void setCanvasProjection( const ScnComponentList& Components );

private:
	class ScnCanvasComponent* Canvas_;
	BcU32 CurrentOptionGroup_;

	std::vector< GaModalOptionGroup > OptionGroups_;

	class ScnMaterial* Material_;
	class ScnMaterial* FontMaterial_;
	class ScnTexture* Texture_;
	class ScnFont* Font_;

	class ScnMaterialComponent* MaterialComponent_;
	class ScnFontComponent* FontComponent_;
};

