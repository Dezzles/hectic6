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
	gaEVT_FLOW_ACTION,
};

struct GaActionEvent : EvtEvent< GaActionEvent >
{
	BcName SourceName_;
	BcName SourceType_;
	BcName Target_;
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

	void spawnRoom( const BcName& RoomName );

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
	ScnEntityRef CurrentRoomEntity_;
	
	// TODO: Use BcNames...
	std::string Room_;
	std::list< std::string > Rooms_;
	std::map< std::string, std::string > Objects_;

};
