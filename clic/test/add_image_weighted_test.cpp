/*  CLIc - version 0.1 - Copyright 2020 Stéphane Rigaud, Robert Haase,
*   Institut Pasteur Paris, Max Planck Institute for Molecular Cell Biology and Genetics Dresden
*
*   CLIc is part of the clEsperanto project http://clesperanto.net 
*
*   This file is subject to the terms and conditions defined in
*   file 'LICENSE.txt', which is part of this source code package.
*/

#include <random>

#include "CLE.h"

/**
 * Main test function
 *
 */
int main(int argc, char **argv)
{
    // Initialise random input and valid output.
    unsigned int width (3), height (3), depth (3);
    float* input_data1 = new float[width*height*depth];
    float* input_data2 = new float[width*height*depth];
    float* valid_data  = new float[width*height*depth];
    for (size_t i = 0; i < width*height*depth; i++)
    {
        input_data1[i] = 1;
        input_data2[i] = 1;
        valid_data[i] = 1.5;
    }
    Image<float> input_img1 (input_data1, width, height, depth, "float");
    Image<float> input_img2 (input_data2, width, height, depth, "float");

    // Initialise GPU information.
    cle::GPU gpu;
    cle::CLE cle(gpu);

    // Initialise device memory and push from host to device
    cle::Buffer gpuInput1 = cle.Push<float>(input_img1);
    cle::Buffer gpuInput2 = cle.Push<float>(input_img2);
    cle::Buffer gpuOutput = cle.Create<float>(input_img1, "float");

    // Call kernel
    cle.AddImagesWeighted(gpuInput1, gpuInput2, gpuOutput, 1, 0.5);

    // pull device memory to host
    Image<float> output_img = cle.Pull<float>(gpuOutput);    

    // Verify output
    float difference = 0;
    for (size_t i = 0; i < width*height*depth; i++)
    {
        if(i%width==0)
        {
            std::cout << std::endl;
        }
        std::cout << output_img.GetData()[i] << " ";
        difference += std::abs(valid_data[i] - output_img.GetData()[i]);
    }
    std::cout << std::endl;
    if (difference > std::numeric_limits<float>::epsilon())
    {
        std::cout << "Test failed, cumulated absolute difference " << difference << " > CPU epsilon (" << std::numeric_limits<float>::epsilon() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    // That's all folks!
    std::cout << "Test succeded!"<< std::endl;
    return EXIT_SUCCESS;
}