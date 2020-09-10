/**
 * Author: Stephane Rigaud - @strigaud 
 */

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <random>

#include "utils.h"
#include "tiffreader.h"
#include "tiffwriter.h"
#include "image.h"
#include "clbuffer.h"
#include "clgpu.h"


/**
 * Get preamble source code
 */
std::string LoadPreamble()
{
    std::string preamble;
    const std::string preamble_path = CLP_PATH;
    std::string filename = preamble_path + "/preamble.cl";
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (file)
    {
        file.seekg(0, std::ios::end);
        preamble.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&preamble[0], preamble.size());
        file.close();
    }
    else
    {
        std::cerr << "Error reading file! Cannot open " << filename << std::endl;
    }
    return preamble;
}


/**
 * Get kernel source code
 */
std::string LoadSources(std::string kernelFilename)
{
    std::string sources;
    const std::string kernels_path = CLI_PATH;
    std::string filename = kernels_path + "/" + kernelFilename + "_x.cl";
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    if (file)
    {
        file.seekg(0, std::ios::end);
        sources.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&sources[0], sources.size());
        file.close();
    }
    else
    {
        std::cerr << "Error reading file! Cannot open " << filename << std::endl;
    }
    return sources;
}


/**
 * Set OpenCL Defines variable from clBuffer.
 */
std::string LoadDefines(std::map<std::string, clBuffer>& parameters)
{
    std::string defines;
    defines = defines + "\n#define GET_IMAGE_WIDTH(image_key) IMAGE_SIZE_ ## image_key ## _WIDTH";
    defines = defines + "\n#define GET_IMAGE_HEIGHT(image_key) IMAGE_SIZE_ ## image_key ## _HEIGHT";
    defines = defines + "\n#define GET_IMAGE_DEPTH(image_key) IMAGE_SIZE_ ## image_key ## _DEPTH";
    defines = defines + "\n";   

    for (auto itr = parameters.begin(); itr != parameters.end(); ++itr)
    {
        // image type handling
        defines = defines + "\n#define CONVERT_" + itr->first + "_PIXEL_TYPE clij_convert_" + itr->second.GetType() + "_sat";
        defines = defines + "\n#define IMAGE_" + itr->first + "_TYPE __global " + itr->second.GetType() + "*";
        defines = defines + "\n#define IMAGE_" + itr->first + "_PIXEL_TYPE " + itr->second.GetType();

        // image size handling
        if (itr->second.GetDimensions()[2] > 1)
        {
            defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_WIDTH " + std::to_string(itr->second.GetDimensions()[0]);
            defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_HEIGHT " + std::to_string(itr->second.GetDimensions()[1]);
            defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_DEPTH " + std::to_string(itr->second.GetDimensions()[2]);
        }
        else
        {
            if (itr->second.GetDimensions()[1] > 1)
            {
                defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_WIDTH " + std::to_string(itr->second.GetDimensions()[0]);
                defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_HEIGHT " + std::to_string(itr->second.GetDimensions()[1]);
            }
            else
            {
                defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_WIDTH " + std::to_string(itr->second.GetDimensions()[0]);
                defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_HEIGHT 1";
            }
            defines = defines + "\n#define IMAGE_SIZE_" + itr->first + "_DEPTH 1";
        }

        // position (dimensionality) handling
        if (itr->second.GetDimensions()[2] == 1)
        {
            defines = defines + "\n#define POS_" + itr->first + "_TYPE int2";
            if (itr->second.GetDimensions()[1] == 1) // 1D
            {
                defines = defines + "\n#define POS_" + itr->first + "_INSTANCE(pos0,pos1,pos2,pos3) (int2)(pos0, 0)";
            }
            else // 2D
            {
                defines = defines + "\n#define POS_" + itr->first + "_INSTANCE(pos0,pos1,pos2,pos3) (int2)(pos0, pos1)";
            }
        }
        else // 3/4D
        {
            defines = defines + "\n#define POS_" + itr->first + "_TYPE int4";
            defines =
                defines + "\n#define POS_" + itr->first + "_INSTANCE(pos0,pos1,pos2,pos3) (int4)(pos0, pos1, pos2, 0)";
        }

        // read/write images
        std::string sdim = (itr->second.GetDimensions()[2] == 1) ? "2" : "3";
        defines = defines + "\n#define READ_" + itr->first + "_IMAGE(a,b,c) read_buffer" + sdim + "d" + itr->second.GetTypeId() +
                  "(GET_IMAGE_WIDTH(a),GET_IMAGE_HEIGHT(a),GET_IMAGE_DEPTH(a),a,b,c)";
        defines = defines + "\n#define WRITE_" + itr->first + "_IMAGE(a,b,c) write_buffer" + sdim + "d" + itr->second.GetTypeId() +
                  "(GET_IMAGE_WIDTH(a),GET_IMAGE_HEIGHT(a),GET_IMAGE_DEPTH(a),a,b,c)";
        defines = defines + "\n";
    }
    return defines;
}

/**
 * Set maximum z projection kernel.
 *
 * Basic function that load and prepare the maximum_z_projection code.
 * Build the program, kernel, and setup the parameters before enqueuing the kernel.
 *
 */
int maximumzprojection(clBuffer src_gpu_obj, clBuffer dst_gpu_obj, clGPU gpu)
{
    // initialise information on kernel and data to process
    cl_int clError;
    std::string kernel_name = "maximum_z_projection";

    std::map<std::string, clBuffer> dataList;
    std::pair<std::string, clBuffer> p1 = std::make_pair("src", src_gpu_obj);
    std::pair<std::string, clBuffer> p2 = std::make_pair("dst_max", dst_gpu_obj);
    dataList.insert(p1);
    dataList.insert(p2);

    // read kernel, defines, and preamble
    std::string kernel_src = LoadSources(kernel_name);
    std::string defines_src = LoadDefines(dataList);
    std::string preambule_src = LoadPreamble();

    // construct final source code
    std::string ocl_src = defines_src + "\n" + preambule_src + "\n" + kernel_src;
    const char *source_str = (ocl_src).c_str();
    size_t source_size = (ocl_src).size();

    // OpenCL kernel code debuging. Uncomment for kernel code information
    // write_kernel_source(ocl_src);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(gpu.GetContext(), 1, &source_str, &source_size, &clError);
    if (clError != CL_SUCCESS)
    {
        std::cerr << "OpenCL Error! Fail to create program in maximumzprojection() : " << getOpenCLErrorString(clError) << std::endl;
        return EXIT_FAILURE;
    }
    // build the program
    cl_device_id device_id = gpu.GetDevice();
    clError = clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);
    if (clError != CL_SUCCESS)
    {
        std::cerr << "OpenCL Error! Fail to build program in maximumzprojection() : " << getOpenCLErrorString(clError) << std::endl;
        return EXIT_FAILURE;
    }
    // create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &clError);
    if (clError != CL_SUCCESS)
    {
        std::cerr << "OpenCL Error! Fail to create kernel in maximumzprojection() : " << getOpenCLErrorString(clError) << std::endl;
        return EXIT_FAILURE;
    }

    // Set the arguments of the kernel
    cl_mem src_mem = src_gpu_obj.GetPointer();
    cl_mem dst_mem = dst_gpu_obj.GetPointer();
    clError = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dst_mem);
    clError = clSetKernelArg(kernel, 1, sizeof(cl_mem), &src_mem);

    // execute the opencl kernel
    size_t global_item_size[3];
    for (size_t i = 0; i < 3; i++)
    {
        global_item_size[i] = std::max(src_gpu_obj.GetDimensions()[i], dst_gpu_obj.GetDimensions()[i]);
    }
    size_t work_dim = 3;
    clError = clEnqueueNDRangeKernel(gpu.GetCommandQueue(), kernel, work_dim, nullptr, global_item_size, nullptr, 0, nullptr, nullptr);
    if (clError != CL_SUCCESS)
    {
        std::cerr << "OpenCL Error! Could not enqueue the ND-Range in maximumzprojection() : " << getOpenCLErrorString(clError) << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}



/**
 * Main test function
 *
 * Example of minipipeline operation.
 * Two kernel execution in a row, with push, create, and pull.
 *
 */
int main(int argc, char **argv)
{
    // Initialise random input and expected output.
    std::default_random_engine generator;
    std::normal_distribution<float> distribution(5.0,2.0);
    unsigned int width (5), height (5), depth (5);
    float* raw_data = new float[width*height*depth];
    for (size_t d = 0; d < depth; d++)
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                int i = x + width*(y+height*d);
                if ( d == y )
                {
                    raw_data[i] = 100;
                }
                else
                {
                    raw_data[i] = distribution(generator);
                }
            }
        }
    }
    float* res_data = new float[width*height*1];
    for (size_t i = 0; i < width*height*1; i++)
    {
        res_data[i] = 100;
    }

    // Initialise device, context, and CQ.
    clGPU gpu;
    gpu.Initialisation();

    // Push / Create input and output buffer
    Image<float> raw_image (raw_data, width, height, depth, "float");
    clBuffer gpuRawImage = gpu.Push<float>(raw_image);
    std::array<unsigned int, 3> dimensions = {width, height, depth};
    dimensions.back() = 1;
    clBuffer gpuProjImage = gpu.Create<float>(dimensions.data(), "float");

    // Apply pipeline of kernels
    maximumzprojection(gpuRawImage, gpuProjImage, gpu);   

    // Pull output into container
    Image<float> out_image = gpu.Pull<float>(gpuProjImage);    

    // Verify output
    float difference = 0;
    for (size_t i = 0; i < width*height*1; i++)
    {
        difference += std::abs(res_data[i] - out_image.GetData()[i]);
    }
    if (difference > std::numeric_limits<float>::epsilon())
    {
        std::cout << difference << " > " << std::numeric_limits<float>::epsilon() << " , the test fail!" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}