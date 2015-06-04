/**************************************************************************
*
* File:		GaTestModelComponent.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaTestModelComponent_H__
#define __GaTestModelComponent_H__

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaTestModelComponent > GaTestModelComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaTestModelComponent
class GaTestModelComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaTestModelComponent, ScnComponent );

	GaTestModelComponent();
	virtual ~GaTestModelComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );
	
private:
	ScnMaterialRef Material_;
	ScnMaterialComponentRef MaterialComponent_;
};

#endif

