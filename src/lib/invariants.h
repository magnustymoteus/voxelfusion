//

#ifndef VOXELFUSION_INVARIANTS_H
#define VOXELFUSION_INVARIANTS_H

#include <cassert>

#define PRECONDITION(assertion, what) \
	if (!(assertion)) assert (what, __FILE__, __LINE__)

#define POSTCONDITION(assertion, what) \
	if (!(assertion)) assert (what, __FILE__, __LINE__)


#endif //VOXELFUSION_INVARIANTS_H
