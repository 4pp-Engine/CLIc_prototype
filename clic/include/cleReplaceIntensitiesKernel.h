/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/


#ifndef __cleReplaceIntensitiesKernel_h
#define __cleReplaceIntensitiesKernel_h

#include "cleKernel.h"

namespace cle
{
    
class ReplaceIntensitiesKernel : public Kernel
{
private:

public:
    ReplaceIntensitiesKernel(GPU& gpu) : Kernel(gpu) 
    {
        kernelName = "replace_intensities";
        tagList = {"dst", "src", "map"};
    }

    void SetInput(Object&);
    void SetOutput(Object&);
    void SetMap(Object&);
    void Execute();

    ~ReplaceIntensitiesKernel() = default;
};

} // namespace cle

#endif // __cleReplaceIntensitiesKernel_h
