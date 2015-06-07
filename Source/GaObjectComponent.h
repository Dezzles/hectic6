#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaObjectComponentRef
typedef ReObjectRef< class GaObjectComponent > GaObjectComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaObjectComponent
class GaObjectComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaObjectComponent, ScnComponent );

	GaObjectComponent();
	virtual ~GaObjectComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	void setup( 
		const std::string& Name, 
		const std::string& Type, 
		const std::string& Target,
		class ScnTexture* Texture,
		MaVec2d Position );

private:
	static void setCanvasProjection( const ScnComponentList& Components );

public:
	class ScnCanvasComponent* Canvas_;

	std::string ObjectName_;
	std::string ObjectType_;
	std::string Target_;

	class ScnMaterial* Material_;
	class ScnTexture* Texture_;

	MaVec2d Position_;
	MaVec2d Size_;

	MaVec2d HotspotStart_;
	MaVec2d HotspotSize_;

	class ScnMaterialComponent* MaterialComponent_;

};
