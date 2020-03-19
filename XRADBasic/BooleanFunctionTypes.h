// file BooleanFunctionTypes.h
//--------------------------------------------------------------
#ifndef __BooleanFunctionTypes_h
#define __BooleanFunctionTypes_h
//--------------------------------------------------------------

#include "Sources/Containers/BooleanFunction.h"
#include "Sources/Containers/BooleanFunction2D.h"

XRAD_BEGIN

//--------------------------------------------------------------

typedef BooleanFunctionLogical<int8_t> BooleanFunctionLogical8;
typedef BooleanFunctionLogical2D<int8_t> BooleanFunctionLogical2D8;

//--------------------------------------------------------------

XRAD_END

#endif // __BooleanFunctionTypes_h
