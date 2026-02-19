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

void ComputeShader::init(const std::string& path, DevicePreference preference)
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
	const auto matches_preference = [&](cl_device_type type) -> bool
	{
		switch (preference)
		{
		case DevicePreference::GPU: return (type & CL_DEVICE_TYPE_GPU) != 0;
		case DevicePreference::CPU: return (type & CL_DEVICE_TYPE_CPU) != 0;
		case DevicePreference::Any:
		default: return true;
		}
	};

	std::vector<std::pair<cl::Platform, cl::Device>> candidates;
	for (const cl::Platform& platform : platforms)
	{
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		for (const cl::Device& candidate : devices)
		{
			const cl_device_type type = candidate.getInfo<CL_DEVICE_TYPE>();
			if (matches_preference(type))
				candidates.emplace_back(platform, candidate);
		}
	}

	if (candidates.empty() && preference != DevicePreference::Any)
	{
		for (const cl::Platform& platform : platforms)
		{
			std::vector<cl::Device> devices;
			platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
			for (const cl::Device& candidate : devices)
				candidates.emplace_back(platform, candidate);
		}
		errors += "Warning: preferred device type unavailable, falling back to any available OpenCL device.\n";
	}

	if (candidates.empty())
	{
		std::cerr << "No devices found!" << std::endl;
		exit(1);
	}

	for (const auto& entry : candidates)
	{
		const cl::Platform& platform = entry.first;
		const cl::Device& candidate = entry.second;
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
				std::cout << "[OpenCL] Platform: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
				std::cout << "[OpenCL] Device: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
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
