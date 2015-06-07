#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaRoomComponentRef
typedef ReObjectRef< class GaRoomComponent > GaRoomComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaRoomProp
struct GaRoomProp
{
	REFLECTION_DECLARE_BASIC( GaRoomProp );
	GaRoomProp():
		Texture_( nullptr ),
		Position_( 0.0f, 0.0f ),
		Scale_( 0.3f, 0.3f )
	{}
	class ScnTexture* Texture_;
	MaVec2d Position_;
	MaVec2d Scale_;
};

//////////////////////////////////////////////////////////////////////////
// GaRoomComponent
class GaRoomComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaRoomComponent, ScnComponent );

	GaRoomComponent();
	virtual ~GaRoomComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

public:
	class ScnCanvasComponent* Canvas_;

	std::string RoomName_;

	class ScnMaterial* Material_;
	std::vector< class ScnTexture* > Foreground_;
	std::vector< class ScnTexture* > Background_;
	std::vector< GaRoomProp > Props_;
	std::vector< class ScnMaterialComponent* > FgMaterialComponents_;
	std::vector< class ScnMaterialComponent* > BgMaterialComponents_;
	std::vector< class ScnMaterialComponent* > PropMaterialComponents_;
	
};
