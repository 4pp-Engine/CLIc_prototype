/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/


#ifndef __cleGreaterConstantKernel_h
#define __cleGreaterConstantKernel_h

#include "cleKernel.h"

namespace cle
{
    
class GreaterConstantKernel : public Kernel
{
private:

    void DefineDimensionality();

public:
    GreaterConstantKernel(GPU& gpu) : Kernel(gpu) 
    {
        kernelName = "greater_constant";
        tagList = {"src1", "scalar", "dst"};
    }

    void SetInput(Object&);
    void SetOutput(Object&);
    void SetScalar(float);
    void Execute();

    ~GreaterConstantKernel() = default;
};

} // namespace cle

#endif // __cleGreaterConstantKernel_h
