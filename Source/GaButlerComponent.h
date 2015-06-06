#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaButlerComponentRef
typedef ReObjectRef< class GaButlerComponent > GaButlerComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaButlerComponent
class GaButlerComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaButlerComponent, ScnComponent );

	GaButlerComponent();
	virtual ~GaButlerComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

private:
	class ScnCanvasComponent* Canvas_;
	
	enum class SelectionState
	{
		IDLE
	};

	SelectionState SelectionState_;
};
