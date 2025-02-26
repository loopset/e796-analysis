#ifndef Colors_h
#define Colors_h

#include "PhysColors.h"

namespace PubUtils
{
// Color for p reactions: (pp) and (pd)
inline int pcol {gPhysColors->Get(16)};
// For d reactions: (dd) and (dt)
inline int dcol {gPhysColors->Get(15)};

} // namespace PubUtils

#endif // !Colors_h
