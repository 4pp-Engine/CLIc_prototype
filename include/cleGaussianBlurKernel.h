/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/


#ifndef __cleGaussianBlurKernel_h
#define __cleGaussianBlurKernel_h

#include "cleKernel.h"

namespace cle
{
    
class GaussianBlurKernel : public Kernel
{
private:
    float x;
    float y;
    float z;

public:
    GaussianBlurKernel(GPU& gpu) : Kernel(gpu) 
    {
        kernelName = "gaussian_blur";
        tagList = {"dst", "src", "scalar"};
    }

    void SetInput(Object&);
    void SetOutput(Object&);
    void SetSigma(float=0, float=0, float=0);
    void Execute();

    ~GaussianBlurKernel() = default;
};

} // namespace cle

#endif // __cleGaussianBlurKernel_h
