%module infomap

%{
/* Includes the header in the wrapper code */
#include "src/core/MemMapEquation.h"
%}

%include "MapEquation.i"
%include "InfoNode.i"

/* Parse the header file to generate wrappers */
%include "src/core/MemMapEquation.h"
