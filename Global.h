#ifndef GLOBAL_H
#define GLOBAL_H

#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <vector>
#include <iostream>
#include <string>
#include <deque>
using namespace std;

typedef unsigned long long int ullint;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

struct GlobalConfs
{
	uint global_worksize;
	uint local_worksize;
	uint threads_per_device;
	bool bitalign;
	bool bfi_int;
	bool bfi_int_fix;
	bool log_midhash;
};

extern GlobalConfs globalconfs;

const uint BLAKE_READ_BUFFER_SIZE = 128;
const uint KERNEL_OUTPUT_SIZE = 256;

const uint WORK_EXPIRE_TIME_SEC = 120;
const uint SHARE_THREAD_RESTART_THRESHOLD_SEC = 20;

#define foreachgpu() for(vector<_clState>::iterator it = GPUstates.begin(); it != GPUstates.end(); ++it)

#endif
