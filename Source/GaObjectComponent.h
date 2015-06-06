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
		class ScnTexture* Texture );

private:
	static void setCanvasProjection( const ScnComponentList& Components );

private:
	class ScnCanvasComponent* Canvas_;

	std::string ObjectName_;
	std::string ObjectType_;
	std::string Target_;

	class ScnMaterial* Material_;
	class ScnTexture* Texture_;

	class ScnMaterialComponent* MaterialComponent_;

};
