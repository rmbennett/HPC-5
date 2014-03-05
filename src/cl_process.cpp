// #include <unistd.h>
// #include <algorithm>
// #include <cassert>
// #include <stdexcept>
// #include <vector>
// #include <cstdio>
// #include <iostream>
// #include <string>
// #include <sys/time.h>
// #include <time.h>
// #include <stddef.h>
// #include <sys/sysinfo.h>

// #ifdef __APPLE__
// #include <OpenCL/opencl.h>
// #else
// #include <CL/cl.h>
// #endif

// #include "cl_process.hpp"

// std::string LoadSource(const char *fileName)
// {
//     // Don't forget to change your_login here
//     std::string baseDir = "src/";
//     if (getenv("HPCE_CL_SRC_DIR"))
//     {
//         baseDir = getenv("HPCE_CL_SRC_DIR");
//     }

//     std::string fullName = baseDir + "/" + fileName;

//     std::ifstream src(fullName, std::ios::in | std::ios::binary);
//     if (!src.is_open())
//     {
//         throw std::runtime_error("LoadSource : Couldn't load cl file from '" + fullName + "'.");
//     }

//     return std::string((std::istreambuf_iterator<char>(src)), // Node the extra brackets.
//                        std::istreambuf_iterator<char>()
//                       );
// }

// void process_cl(int levels, unsigned w, unsigned h, unsigned /*bits*/, std::vector<uint32_t> &pixels)
// {
//     std::vector<cl::Platform> platforms;

//     cl::Platform::get(&platforms);
//     if (platforms.size() == 0)
//     {
//         throw std::runtime_error("No OpenCL platforms found.");
//     }

//     std::cerr << "Found " << platforms.size() << " platforms\n";
//     for (unsigned i = 0; i < platforms.size(); i++)
//     {
//         std::string vendor = platforms[0].getInfo<CL_PLATFORM_VENDOR>();
//         std::cerr << "  Platform " << i << " : " << vendor << "\n";
//     }

//     int selectedPlatform = 0;
//     if (getenv("HPCE_SELECT_PLATFORM"))
//     {
//         selectedPlatform = atoi(getenv("HPCE_SELECT_PLATFORM"));
//     }
//     std::cerr << "Choosing platform " << selectedPlatform << "\n";
//     cl::Platform platform = platforms.at(selectedPlatform);

//     std::vector<cl::Device> devices;
//     platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
//     if (devices.size() == 0)
//     {
//         throw std::runtime_error("No opencl devices found.\n");
//     }

//     std::cerr << "Found " << devices.size() << " devices\n";
//     for (unsigned i = 0; i < devices.size(); i++)
//     {
//         std::string name = devices[i].getInfo<CL_DEVICE_NAME>();
//         std::cerr << "  Device " << i << " : " << name << "\n";
//     }

//     int selectedDevice = 0;
//     if (getenv("HPCE_SELECT_DEVICE"))
//     {
//         selectedDevice = atoi(getenv("HPCE_SELECT_DEVICE"));
//     }
//     std::cerr << "Choosing device " << selectedDevice << "\n";
//     cl::Device device = devices.at(selectedDevice);

//     cl::Context context(devices);

//     std::string kernelSource = LoadSource("process_kernel.cl");

//     cl::Program::Sources sources;   // A vector of (data,length) pairs
//     sources.push_back(std::make_pair(kernelSource.c_str(), kernelSource.size() + 1)); // push on our single string

//     cl::Program program(context, sources);
//     try
//     {
//         program.build(devices);
//     }
//     catch (...)
//     {
//         for (unsigned i = 0; i < devices.size(); i++)
//         {
//             std::cerr << "Log for device " << devices[i].getInfo<CL_DEVICE_NAME>() << ":\n\n";
//             std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]) << "\n\n";
//         }
//         throw;
//     }
//     /**************** Dunno about buffers**/
//     // size_t cbBuffer=4*world.w*world.h;
//     // cl::Buffer buffProperties(context, CL_MEM_READ_ONLY, cbBuffer);
//     // cl::Buffer buffState(context, CL_MEM_READ_WRITE, cbBuffer);
//     // cl::Buffer buffBuffer(context, CL_MEM_READ_WRITE, cbBuffer);

//     cl::Kernel kernel(program, "process");

//     unsigned w = world.w, h = world.h;

//     float outer = world.alpha * dt; // We spread alpha to other cells per time
//     float inner = 1 - outer / 4;        // Anything that doesn't spread stays

//     kernel.setArg(0, levels);
//     kernel.setArg(1, w);
//     kernel.setArg(3, h);
//     kernel.setArg(2, &pixels);
//     (int levels, unsigned w, unsigned h, unsigned /*bits*/, std::vector<uint32_t> &pixels
//      cl::CommandQueue queue(context, device);

//      queue.enqueueWriteBuffer(buffProperties, CL_TRUE, 0, cbBuffer, &world.properties[0]);

//      // This is our temporary working space
//      //std::vector<float> buffer(w*h);
//      cl::NDRange offset(0, 0);               // Always start iterations at x=0, y=0
//      cl::NDRange globalSize(1, levels);  // Global size must match the original loops
//      cl::NDRange localSize = cl::NullRange;  // We don't care about local size

//      queue.enqueueWriteBuffer(buffState, CL_TRUE, 0, cbBuffer, &world.state[0]); //not sure about this

//      //cl::Event evCopiedState;

//      //std::vector<cl::Event> kernelDependencies(1, evCopiedState);
//      // cl::Event evExecutedKernel;
//      // std::vector<uint32_t> packed(w*h, 0);
//      // for(uint32_t x = 0; x < w; x++)
//      // {
//      //  for(uint32_t y = 0; y < h; y++)
//      //  {
//      //      unsigned index=y*w + x;
//      //      packed[index] = world.properties[index];
//      //      if(!((world.properties[index] & Cell_Fixed) || (world.properties[index] & Cell_Insulator)))
//      //      {
//      //              if(!(world.properties[index-w] & Cell_Insulator))
//      //              {
//      //                  packed[index] +=0x4;
//      //              }

//      //              // Cell below
//      //              if(!(world.properties[index+w] & Cell_Insulator))
//      //              {
//      //                  packed[index] +=0x8;
//      //              }

//      //                  // Cell left
//      //              if(! (world.properties[index-1] & Cell_Insulator))
//      //              {
//      //                  packed[index] +=0x10;
//      //              }

//      //                  // Cell right
//      //              if(! (world.properties[index+1] & Cell_Insulator))
//      //              {
//      //                  packed[index] +=0x20;
//      //              }
//      //      }
//      //  }
//      // }
//      //  for(unsigned t=0;t<n;t++)
//      //  {
//      //      queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);
//      //      queue.enqueueBarrier(); // <- new barrier here

//      //      //std::swap(buffState, buffBuffer);
//      //      queue.enqueueCopyBuffer(buffBuffer, buffState, 0,0, cbBuffer);
//      //      //  kernel_xy(x,y,w, &world.state[0], &world.properties[0], &buffer[0], outer, inner);

//      //      //std::vector<cl::Event> copyBackDependencies(1, evExecutedKernel);

//      //      world.t += dt; // We have moved the world forwards in time

//      //  } // end of for(t...

//      //  queue.enqueueReadBuffer(buffBuffer, CL_TRUE, 0, cbBuffer, &world.state[0]);
//      // }