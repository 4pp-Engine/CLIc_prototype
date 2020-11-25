/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/


#include "cleSetNonzeroPixelsToPixelindexKernel.h"

namespace cle
{

void SetNonzeroPixelsToPixelindexKernel::SetInput(Object& x)
{
    this->AddObject(&x, "src");
}

void SetNonzeroPixelsToPixelindexKernel::SetOutput(Object& x)
{
    this->AddObject(&x, "dst");
}

void SetNonzeroPixelsToPixelindexKernel::Execute()
{
    CompileKernel();
    AddArgumentsToKernel();
    DefineRangeKernel();
}

} // namespace cle
