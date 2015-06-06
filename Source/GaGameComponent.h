#pragma once

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

#include "System/Scene/Rendering/ScnMaterial.h"

//////////////////////////////////////////////////////////////////////////
// GaGameEvents
#define GA_EVENTGROUP_FLOW		EVT_MAKE_ID( 'G', 'a', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum GsEvents
{
	// All core events.
	gaEVT_FLOW_FIRST = GA_EVENTGROUP_FLOW,
	gaEVT_FLOW_MOVE_TO_ROOM,
	gaEVT_CORE_OPEN_BUTLER_DIALOG,
	gaEVT_CORE_OPEN_ITEM_DIALOG,
};


//////////////////////////////////////////////////////////////////////////
// GaGameComponentRef
typedef ReObjectRef< class GaGameComponent > GaGameComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( GaGameComponent, ScnComponent );

	GaGameComponent();
	virtual ~GaGameComponent();
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	/**
	 * Called to move to a new room.
	 */
	eEvtReturn onMoveToRoom( EvtID ID, const EvtBaseEvent& InEvent );

	/**
	 * Called to open butler dialog.
	 */
	eEvtReturn onOpenButlerDialog( EvtID ID, const EvtBaseEvent& InEvent );

	/**
	 * Called to open item dialog.
	 */
	eEvtReturn onOpenItemDialog( EvtID ID, const EvtBaseEvent& InEvent );


private:
	static void setCanvasProjection( const ScnComponentList& Components );

private:
	class ScnCanvasComponent* Canvas_;

	enum class GameState
	{
		IDLE,
		TRANSITION_OUT,
		TRANSITION_IN
	};

	GameState GameState_;
};
