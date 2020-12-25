/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/


#ifndef __cleDilateSphereKernel_h
#define __cleDilateSphereKernel_h

#include "cleKernel.h"

namespace cle
{
    
class DilateSphereKernel : public Kernel
{
private:

    void DefineDimensionality();


public:
    DilateSphereKernel(GPU& gpu) : Kernel(gpu) 
    {
        kernelName = "dilate_sphere";
        tagList = {"src" , "dst"};
    }

    void SetInput(Object&);
    void SetOutput(Object&);
    void Execute();

    ~DilateSphereKernel() = default;
};

} // namespace cle

#endif // __cleDilateSphereKernel_h
