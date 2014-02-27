#ifndef MTRLTINOPENCL_H
#define MTRLTINOPENCL_H

class OpenCL
{
private:
public:
	void Init();
	void Quit();

	uint GetVectorSize();
};

#include "pthread.h"

#ifdef __APPLE_CC__
#include <OpenCL/opencl.h>
#else
#ifdef WIN32
#include "CL/cl.h"
#else
#include "cl.h"
#endif
#endif

#include "Util.h"
#include "App.h"

struct _clState
{
	cl_context context;
	cl_kernel kernel;
	cl_command_queue commandQueue;
	cl_program program;
	cl_mem CLbuffer[2];
	cl_mem padbuffer;

	uint vectors;
	uint thread_id;

	pthread_t thread;

	bool shares_available;
	deque<vector<uchar> > shares;
	pthread_mutex_t share_mutex;

	ullint hashes;
};

#endif
