#include "vpath.h"

VPath::VPath()
    : VObject()
{
}

VPath::~VPath()
{
    // remove primitives:
    VPrimitive* prim;
    for ( prim=m_primitives.first(); prim!=0; prim=m_primitives.next() )
    {
	delete prim;
    }
}

void
VPath::draw( VPainter& /*p*/ )
{
}
