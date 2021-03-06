
#include <random>

#include "CLE.h"

/**
 * Main test function
 *
 */
int main(int argc, char **argv)
{
    // Initialise random input and valid output.
    unsigned int width (4), height (4), depth (1);
    unsigned int dims[3] = {width, height, depth};
    
    std::vector<float> input_data = {
                0, 0, 0, 1,
                0, 0, 3, 1,
                0, 0, 3, 1,
                1, 1, 1, 1
    };

    std::vector<float> valid_data = {
                0, 0, 0, 13,
                0, 0, 10, 14,
                0, 0, 11, 15,
                4, 8, 12, 16
    };

    // Initialise GPU information.
    cle::GPU gpu;
    cle::CLE cle(gpu);

    // Initialise device memory and push from host to device
    cle::Buffer Buffer_A = cle.Push<float>(input_data, dims);
    cle::Buffer Buffer_B = cle.Create<float>(dims);

    // Call kernel
    cle.SetNonzeroPixelsToPixelindex(Buffer_A, Buffer_B);

    // pull device memory to host
    std::vector<float> output_data = cle.Pull<float>(Buffer_B);    

    // Verify output
    float difference = 0;
    for (size_t i = 0; i < output_data.size(); i++)
    {
        std::cout << valid_data[i] << " " << output_data[i] << std::endl;
        difference += std::abs(valid_data[i] - output_data[i]);
    }
    if (difference > std::numeric_limits<float>::epsilon())
    {
        std::cout << "Test failed, cumulated absolute difference " << difference << " > CPU epsilon (" << std::numeric_limits<float>::epsilon() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    // That's all folks!
    std::cout << "Test succeded!"<< std::endl;
    return EXIT_SUCCESS;
}