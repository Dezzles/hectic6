#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaModalComponentRef
typedef ReObjectRef< class GaModalComponent > GaModalComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaModalComponent
class GaModalComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaModalComponent, ScnComponent );

	GaModalComponent();
	virtual ~GaModalComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );


private:
	static void setCanvasProjection( const ScnComponentList& Components );

private:
	class ScnCanvasComponent* Canvas_;

	// Text to show.
	std::string Text_;

	// Information this unlocks for accusations.
	std::vector< std::string > Info_;
};

