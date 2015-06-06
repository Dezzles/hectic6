#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaRoomComponentRef
typedef ReObjectRef< class GaRoomComponent > GaRoomComponentRef;

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

private:
	class ScnCanvasComponent* Canvas_;

	std::string RoomName_;

	class ScnMaterial* Material_;
	std::vector< class ScnTexture* > Foreground_;
	std::vector< class ScnTexture* > Background_;
	std::vector< class ScnMaterialComponent* > FgMaterialComponents_;
	std::vector< class ScnMaterialComponent* > BgMaterialComponents_;
	
};
