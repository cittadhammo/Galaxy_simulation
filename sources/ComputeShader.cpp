#include "ComputeShader.hpp"

cl::Program			ComputeShader::program;
cl::Context			ComputeShader::context;
cl::Device			ComputeShader::device;
cl::CommandQueue	ComputeShader::queue;

cl::Device ComputeShader::get_default_device()
{
	// Search for all the OpenCL platforms available and check if there are any.

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	if (platforms.empty())
	{
		std::cerr << "No platforms found!" << std::endl;
		exit(1);
	}

	// Search for all the devices on the first platform and check if there are any available.

	auto platform = platforms.front();
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	if (devices.empty())
	{
		std::cerr << "No devices found!" << std::endl;
		exit(1);
	}

	// Return the first device found.
	return devices.front();
}

void ComputeShader::init(const std::string& path)
{
	// Read OpenCL kernel file as a string.
	std::ifstream kernel_file(path);
	std::string src(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.empty())
	{
		std::cerr << "No platforms found!" << std::endl;
		exit(1);
	}

	std::string errors;
	for (const cl::Platform& platform : platforms)
	{
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		for (const cl::Device& candidate : devices)
		{
			try
			{
				cl::Context test_context(candidate);
				cl::Program test_program(test_context, sources);
				const cl_int build_result = test_program.build();
				if (build_result == CL_BUILD_SUCCESS)
				{
					device = candidate;
					context = test_context;
					program = test_program;
					queue = cl::CommandQueue(context, device);
					return;
				}

				errors += "Device: " + candidate.getInfo<CL_DEVICE_NAME>() + "\n";
				errors += "Build Status: " + std::to_string(test_program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(candidate)) + "\n";
				errors += "Build Log:\n" + test_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(candidate) + "\n";
			}
			catch (...)
			{
				errors += "Device: " + candidate.getInfo<CL_DEVICE_NAME>() + "\n";
				errors += "OpenCL error: exception during context/program setup\n";
			}
		}
	}

	std::cerr << "Error: failed to build compute program on all devices.\n" << errors << std::endl;
	exit(1);
}

void ComputeShader::launch(const std::string& function, const std::vector<cl::Buffer*>& buffers, const cl::NDRange& global, const cl::NDRange& local)
{
	cl::Kernel kernel(program, function.data());

	for (int i = 0; i < buffers.size(); i++)
		kernel.setArg(i, *(buffers[i]));

	queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
}

void ComputeShader::launch(const std::string& function, const std::vector<cl::Buffer*>& buffers, const cl::NDRange& global)
{
	cl::Kernel kernel(program, function.data());

	for (int i = 0; i < buffers.size(); i++)
		kernel.setArg(i, *(buffers[i]));

	queue.enqueueNDRangeKernel(kernel, cl::NullRange, global);
}
