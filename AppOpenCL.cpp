#include "Global.h"
#include "AppOpenCL.h"
#include "Util.h"


uint OpenCL::GetVectorSize()
{
	return 2;
}

vector<_clState> GPUstates;

#ifdef WIN32
#include "windows.h"
void Wait_ms(uint n)
{
	Sleep(n);
}
#else
#include <unistd.h>
void Wait_ms(uint n)
{
	usleep(n*1000);
}
#endif

#define Ch(x, y, z) (z ^ (x & (y ^ z)))
#define Ma(x, y, z) ((y & z) | (x & (y | z)))
#define Tr(x,a,b,c) (rot(x,a)^rot(x,b)^rot(x,c))
#define Wr(x,a,b,c) (rot(x,a)^rot(x,b)^(x>>c))

uint rot(uint a, uint b)
{
	return (a>>b) | (a<<(32-b));
}

extern pthread_mutex_t current_work_mutex;
extern Work current_work;

#include <ctime>

pthread_mutex_t noncemutex = PTHREAD_MUTEX_INITIALIZER;
uint nonce = 0;

/*
struct BLOCK_DATA
{
	int32 nVersion;
	uint256 hashPrevBlock;
	uint256 hashMerkleRoot;
	int64 nBlockNum;
	int64 nTime;
	uint64 nNonce1;
	uint64 nNonce2;
	uint64 nNonce3;
	uint32 nNonce4;
	char miner_id[12];
	uint32 dwBits;
};
*/

extern unsigned char *BlockHash_1_MemoryPAD8;

#include "RSHash.h"

void* Reap(void* param)
{
	_clState* state = (_clState*)param;
	state->hashes = 0;

	size_t globalsize = globalconfs.global_worksize;
	size_t localsize = globalconfs.local_worksize;

	Work tempwork;

	uchar tempdata[512];
	memset(tempdata, 0, 512);

	clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[0], true, 0, BLAKE_READ_BUFFER_SIZE, tempdata, 0, NULL, NULL);
	clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[1], true, 0, KERNEL_OUTPUT_SIZE*sizeof(uint), tempdata, 0, NULL, NULL);
	clEnqueueWriteBuffer(state->commandQueue, state->padbuffer, true, 0, 1024*1024*4+8, BlockHash_1_MemoryPAD8, 0, NULL, NULL);

	uchar finalhash[256];
	memset(finalhash, 0, 256);

	uint kernel_output[KERNEL_OUTPUT_SIZE] = {};
	
	while(true)
	{
		if (current_work.old)
		{
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time)
		{
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			memcpy(tempdata, &tempwork.data[0], 128);
			*(uint*)&tempdata[88] = state->thread_id;
		}
		clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[0], true, 0, BLAKE_READ_BUFFER_SIZE, tempdata, 0, NULL, NULL);
		clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[1], true, 0, KERNEL_OUTPUT_SIZE*sizeof(uint), kernel_output, 0, NULL, NULL);
		clSetKernelArg(state->kernel, 0, sizeof(cl_mem), &state->CLbuffer[0]);
		clSetKernelArg(state->kernel, 1, sizeof(cl_mem), &state->CLbuffer[1]);
		clSetKernelArg(state->kernel, 2, sizeof(cl_mem), &state->padbuffer);
		clEnqueueNDRangeKernel(state->commandQueue, state->kernel, 1, NULL, &globalsize, &localsize, 0, NULL, NULL);
		clEnqueueReadBuffer(state->commandQueue, state->CLbuffer[1], true, 0, KERNEL_OUTPUT_SIZE*sizeof(uint), kernel_output, 0, NULL, NULL);

		for(uint i=0; i<256; ++i)
		{
			if (kernel_output[i] != 0)
			{
				uint result = kernel_output[i];
				*((uint*)&tempdata[92]) = result;
				{
					uchar testmem[544];
					memcpy(testmem, tempdata, 128);
					BlockHash_1(testmem, testmem+512);
					if (testmem[541] < 0x7F)
					{
						vector<uchar> share(tempdata, tempdata+128);
						bool below=true;
						for(int i=31; i>=0; --i)
						{
							if (testmem[512+i] > tempwork.target_share[31-i])
							{
								below=false;
								break;
							}
							if (testmem[512+i] < tempwork.target_share[31-i])
							{
								break;
							}
						}
						if (below)
						{
							pthread_mutex_lock(&state->share_mutex);
							state->shares_available = true;
							state->shares.push_back(share);
							pthread_mutex_unlock(&state->share_mutex);
						}
					}
				}
				kernel_output[i] = 0;
				*((uint*)&tempdata[92]) = 0;
			}
		}
		state->hashes += globalconfs.global_worksize;

		++*(uint*)&tempdata[84];
	}
	pthread_exit(NULL);
}

_clState clState;

void OpenCL::Init()
{
	cl_int status = 0;

	cl_uint numPlatforms;
	cl_platform_id platform = NULL;

	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if(status != CL_SUCCESS)
		throw string("Error getting OpenCL platforms");

	if(numPlatforms > 0)
	{   
		cl_platform_id* platforms = new cl_platform_id[numPlatforms];

		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if(status != CL_SUCCESS)
			throw string("Error getting OpenCL platform IDs");

		unsigned int i;
		for(i=0; i < numPlatforms; ++i)
		{   
			char pbuff[100];

			status = clGetPlatformInfo( platforms[i], CL_PLATFORM_VENDOR, sizeof(pbuff), pbuff, NULL);
			if(status != CL_SUCCESS)
			{   
				delete [] platforms;
				throw string("Error getting OpenCL platform info");
			}

			platform = platforms[i];
			if(!strcmp(pbuff, "Advanced Micro Devices, Inc."))
			{   
				break;
			}  
		}   
		delete [] platforms;
	}   

	if(platform == NULL) 
		throw string("No OpenCL platforms found");

	cl_uint numDevices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	if(status != CL_SUCCESS)
	{
		throw string("Error getting OpenCL device IDs");
	}

	if (numDevices == 0)
		throw string("No OpenCL devices found");

	vector<cl_device_id> devices;
	cl_device_id* devicearray = new cl_device_id[numDevices];

	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devicearray, NULL);
	if(status != CL_SUCCESS)
		throw string("Error getting OpenCL device ID list");

	for(uint i=0; i<numDevices; ++i)
		devices.push_back(devicearray[i]);

	cout << "List of devices" << endl;

	for(uint i=0; i<numDevices; i++) 
	{
		char pbuff[100];
		status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(pbuff), pbuff, NULL);
		if(status != CL_SUCCESS)
			throw string("Error getting OpenCL device info");

		cout << "\t" << i << "\t" << pbuff << endl;
	}

	cout << "Compiling OpenCL kernel.. this might take 20 seconds" << endl;

	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	clState.context = clCreateContextFromType(cps, CL_DEVICE_TYPE_GPU, NULL, NULL, &status);
	if(status != CL_SUCCESS) 
		throw string("Error creating OpenCL context");

	string source;
	{
		string filename  = "reaper.cl";
		FILE* filu = fopen(filename.c_str(), "r");
		fseek(filu, 0, SEEK_END);
		uint size = ftell(filu);
		fseek(filu, 0, SEEK_SET);
		for(uint i=0; i<size; ++i)
		{
			char c;
			fread(&c, 1, 1, filu);
			source.push_back(c);
		}
	}

	vector<size_t> sourcesizes;
	sourcesizes.push_back(source.length());

	const char* see = source.c_str();

	for(uint device_id=0; device_id<numDevices; ++device_id) 
	{
		bool use_vectors = true;

		_clState GPUstate;

		GPUstate.program = clCreateProgramWithSource(clState.context, 1, (const char **)&see, &sourcesizes[0], &status);
		if(status != CL_SUCCESS) 
			throw string("Error creating OpenCL program from source");

		string compile_options;

		if (use_vectors)
			compile_options += " -D VECTORS";

		status = clBuildProgram(GPUstate.program, 1, &devices[device_id], compile_options.c_str(), NULL, NULL);
		if(status != CL_SUCCESS) 
		{   
			size_t logSize;
			status = clGetProgramBuildInfo(GPUstate.program, devices[device_id], CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);

			char* log = new char[logSize];
			status = clGetProgramBuildInfo(GPUstate.program, devices[device_id], CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
			cout << log << endl;
			delete [] log;
			throw string("Error building OpenCL program");
		}
		
		{
			uint device_amount;
			clGetProgramInfo(GPUstate.program, CL_PROGRAM_NUM_DEVICES, sizeof(uint), &device_amount, NULL);

			size_t* binarysizes = new size_t[device_amount];
			uchar** binaries = new uchar*[device_amount];
			for(uint curr_binary = 0; curr_binary<device_amount; ++curr_binary)
			{
				clGetProgramInfo(GPUstate.program, CL_PROGRAM_BINARY_SIZES, device_amount*sizeof(uint), binarysizes, NULL);
				binaries[curr_binary] = new uchar[binarysizes[curr_binary]];
			}
			clGetProgramInfo(GPUstate.program, CL_PROGRAM_BINARIES, sizeof(uchar*)*device_amount, binaries, NULL);

			for(uint binary_id = 0; binary_id < device_amount; ++binary_id)
			{
				if (binarysizes[binary_id] == 0)
					continue;

				cout << "Binary size: " << binarysizes[binary_id] << " bytes" << endl;
			}
			clReleaseProgram(GPUstate.program);

			cl_int binary_status, errorcode_ret;
			GPUstate.program = clCreateProgramWithBinary(clState.context, 1, &devices[device_id], &binarysizes[device_id], const_cast<const uchar**>(&binaries[device_id]), &binary_status, &errorcode_ret);
			status = clBuildProgram(GPUstate.program, 1, &devices[device_id], compile_options.c_str(), NULL, NULL);

			if(status == CL_SUCCESS) 
				cout << "Program rebuilt." << endl;
			else
				cout << "Failed to BFI_INT patch kernel on device " << device_id <<  ". What now?" << endl;
			delete [] binarysizes;
			for(uint binary_id=0; binary_id < device_amount; ++binary_id)
				delete [] binaries[binary_id];
			delete [] binaries;
		}

		GPUstate.kernel = clCreateKernel(GPUstate.program, "search", &status);
		if(status != CL_SUCCESS)
		{
			cout << "Kernel build not successful: " << status << endl;
			throw string("Error creating OpenCL kernel");
		}
		for(uint thread_id = 0; thread_id < globalconfs.threads_per_device; ++thread_id)
		{
			GPUstate.commandQueue = clCreateCommandQueue(clState.context, devices[device_id], 0, &status);
			if(status != CL_SUCCESS)
				throw string("Error creating OpenCL command queue");

			GPUstate.CLbuffer[0] = clCreateBuffer(clState.context, CL_MEM_READ_ONLY, BLAKE_READ_BUFFER_SIZE, NULL, &status);
			GPUstate.CLbuffer[1] = clCreateBuffer(clState.context, CL_MEM_WRITE_ONLY, KERNEL_OUTPUT_SIZE*sizeof(uint), NULL, &status);
			GPUstate.padbuffer = clCreateBuffer(clState.context, CL_MEM_READ_ONLY, 1024*1024*4+8, NULL, &status);

			if(status != CL_SUCCESS)
			{
				cout << status << endl;
				throw string("Error creating OpenCL buffer");
			}

			pthread_mutex_t initializer = PTHREAD_MUTEX_INITIALIZER;

			GPUstate.share_mutex = initializer;
			GPUstate.shares_available = false;


			GPUstate.vectors = GetVectorSize();
			GPUstate.thread_id = device_id*numDevices+thread_id;
			GPUstates.push_back(GPUstate);
		}
	}
	cout << "Creating " << GPUstates.size() << " threads" << endl;
	for(uint i=0; i<GPUstates.size(); ++i)
	{
		cout << i+1 << "...";
		pthread_create(&GPUstates[i].thread, NULL, Reap, (void*)&GPUstates[i]);
	}
	cout << "done" << endl;
}

void OpenCL::Quit()
{
	
}
