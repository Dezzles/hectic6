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
// GaGameObject
struct GaGameObject
{
	REFLECTION_DECLARE_BASIC( GaGameObject );
	GaGameObject(){}
	std::string Object_; // Name as identifier ( {type}_{name} ) used for entity
	std::string Room_; // Name of the room they're in ( ROOM_{number} )
	std::string InfoText_; // Information returned by entity
	std::vector< std::string > Infos_; // What information is revealed ( {TYPE}_{NAME} )
};

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
	void spawnModal( const BcName& ModalName, const BcName& Source );
	void useObject( const BcName& ObjectName );

	GaGameObject* findObject( const BcName& ObjectName );

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
	ScnEntityRef ModalDialogEntity_;
	
	// TODO: Use BcNames...
	std::string Room_;
	std::list< std::string > Rooms_;
	std::list< GaGameObject > Objects_;
	std::set< std::string > Infos_;
};
