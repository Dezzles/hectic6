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
	std::string Name_;
	std::string Text_;
};

struct GaModalOptionGroup
{
	REFLECTION_DECLARE_BASIC( GaModalOptionGroup );
	GaModalOptionGroup(){}
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
};

