#include "Database.h"

WorldGen::Database::Database()
	: Rooms_(this), People_(this), Times_(this), Information_(this),
	PlayerInfo_(this), Murders_(this), Items_(this)
{

}