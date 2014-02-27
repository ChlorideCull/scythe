#include "Global.h"
#include "AppOpenCL.h"
#include "Util.h"

#include <algorithm>

uint OpenCL::GetVectorSize()
{
	uint ret = globalconfs.coin.config.GetValue < uint > ("vectors");
	if (ret > 4)
		ret = 4;
	else if (ret > 2)
		ret = 2;
	else if (ret == 0)
		ret = 1;
	return ret;
}

#ifndef CPU_MINING_ONLY
vector < _clState > GPUstates;
#endif

extern pthread_mutex_t current_work_mutex;
extern Work current_work;

#include <ctime>

/*
struct BLOCK_DATA
{
0	int32 nVersion;
4	uint256 hashPrevBlock;
36	uint256 hashMerkleRoot;
68	int64 nBlockNum;
76	int64 nTime;
84	uint64 nNonce1;
92	uint64 nNonce2;
100	uint64 nNonce3;
108	uint32 nNonce4;
112 char miner_id[12];
124	uint32 dwBits;
};
*/

extern unsigned char *BlockHash_1_MemoryPAD8;
extern uint *BlockHash_1_MemoryPAD32;

extern ullint shares_hwinvalid;
extern ullint shares_hwvalid;

#include "RSHash.h"

#include <deque>
using std::deque;

#define Ch(x, y, z) (z ^ (x & (y ^ z)))
#define Ma(x, y, z) ((y & z) | (x & (y | z)))

#define Tr(x,a,b,c) (rotl(x,a)^rotl(x,b)^rotl(x,c))

uint rotl(uint x, uint y);

#define R(x) (work[x] = (rotl(work[x-2],15)^rotl(work[x-2],13)^((work[x-2])>>10)) + work[x-7] + (rotl(work[x-15],25)^rotl(work[x-15],14)^((work[x-15])>>3)) + work[x-16])
#define sharound(a,b,c,d,e,f,g,h,x,K) h+=Tr(e,7,21,26)+Ch(e,f,g)+K+x; d+=h; h+=Tr(a,10,19,30)+Ma(a,b,c);

#define Tr1(x) Tr(x,7U,21U,26U)
#define Tr2(x) Tr(x,10U,19U,30U)

string VectorToHexString(vector < uchar > vec);

const uint K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	    0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
	    0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa,
	    0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	    0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb,
	    0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624,
	    0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	    0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb,
	    0xbef9a3f7, 0xc67178f2
};

#ifndef CPU_MINING_ONLY

void PreCalc(uchar * in, cl_uint8 & s)
{
	uint work[64];

	uint *udata = (uint *) in;
#define A s.s[0]
#define B s.s[1]
#define C s.s[2]
#define D s.s[3]
#define E s.s[4]
#define F s.s[5]
#define G s.s[6]
#define H s.s[7]

	work[0] = EndianSwap(udata[0]);
	D = 0x98c7e2a2U + work[0];
	H = 0xfc08884dU + work[0];

	work[1] = EndianSwap(udata[1]);
	C = 0xcd2a11aeU + Tr1(D) + Ch(D, 0x510e527fU, 0x9b05688cU) + work[1];
	G = 0xC3910C8EU + C + Tr2(H) + Ch(H, 0xfb6feee7U, 0x2a01a605U);

	work[2] = EndianSwap(udata[2]);
	B = 0x0c2e12e0U + Tr1(C) + Ch(C, D, 0x510e527fU) + work[2];
	F = 0x4498517BU + B + Tr2(G) + Ma(G, H, 0x6a09e667U);

	work[3] = EndianSwap(udata[3]);
	A = 0xa4ce148bU + Tr1(B) + Ch(B, C, D) + work[3];
	E = 0x95F61999U + A + Tr2(F) + Ma(F, G, H);

	work[4] = EndianSwap(udata[4]);
	sharound(E, F, G, H, A, B, C, D, work[4], K[4]);
	work[5] = EndianSwap(udata[5]);
	sharound(D, E, F, G, H, A, B, C, work[5], K[5]);
	work[6] = EndianSwap(udata[6]);
	sharound(C, D, E, F, G, H, A, B, work[6], K[6]);
	work[7] = EndianSwap(udata[7]);
	sharound(B, C, D, E, F, G, H, A, work[7], K[7]);
	work[8] = EndianSwap(udata[8]);
	sharound(A, B, C, D, E, F, G, H, work[8], K[8]);
	work[9] = EndianSwap(udata[9]);
	sharound(H, A, B, C, D, E, F, G, work[9], K[9]);
	work[10] = EndianSwap(udata[10]);
	sharound(G, H, A, B, C, D, E, F, work[10], K[10]);
	work[11] = EndianSwap(udata[11]);
	sharound(F, G, H, A, B, C, D, E, work[11], K[11]);
	work[12] = EndianSwap(udata[12]);
	sharound(E, F, G, H, A, B, C, D, work[12], K[12]);
	work[13] = EndianSwap(udata[13]);
	sharound(D, E, F, G, H, A, B, C, work[13], K[13]);
	work[14] = EndianSwap(udata[14]);
	sharound(C, D, E, F, G, H, A, B, work[14], K[14]);
	work[15] = EndianSwap(udata[15]);
	sharound(B, C, D, E, F, G, H, A, work[15], K[15]);
	for (uint i = 16; i < 64; i += 8) {
		sharound(A, B, C, D, E, F, G, H, R(i + 0), K[i + 0]);
		sharound(H, A, B, C, D, E, F, G, R(i + 1), K[i + 1]);
		sharound(G, H, A, B, C, D, E, F, R(i + 2), K[i + 2]);
		sharound(F, G, H, A, B, C, D, E, R(i + 3), K[i + 3]);
		sharound(E, F, G, H, A, B, C, D, R(i + 4), K[i + 4]);
		sharound(D, E, F, G, H, A, B, C, R(i + 5), K[i + 5]);
		sharound(C, D, E, F, G, H, A, B, R(i + 6), K[i + 6]);
		sharound(B, C, D, E, F, G, H, A, R(i + 7), K[i + 7]);
	}
	A += 0x6a09e667;
	B += 0xbb67ae85;
	C += 0x3c6ef372;
	D += 0xa54ff53a;
	E += 0x510e527f;
	F += 0x9b05688c;
	G += 0x1f83d9ab;
	H += 0x5be0cd19;
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
}

void v3hash(uchar * input, uchar * scratch, uchar * output);

void *Reap_GPU_SLC(void *param)
{
	_clState *state = (_clState *) param;
	state->hashes = 0;

	size_t globalsize = globalconfs.coin.global_worksize;
	size_t localsize = globalconfs.coin.local_worksize;

	Work tempwork;

	uchar tempdata[1024];
	memset(tempdata, 0, 1024);

	clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[0], true, 0,
			     KERNEL_INPUT_SIZE, tempdata, 0, NULL, NULL);
	clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[1], true, 0,
			     KERNEL_OUTPUT_SIZE * sizeof(uint), tempdata, 0,
			     NULL, NULL);

	uint kernel_output[KERNEL_OUTPUT_SIZE] = { };

	bool write_kernel_output = true;
	bool write_kernel_input = true;

	uint scversion = 2;
	if (globalconfs.coin.name == "solidcoin3")
		scversion = 3;

	size_t base = 0;
	clSetKernelArg(state->kernel, 2, sizeof(cl_mem), &state->padbuffer8);

	bool errorfree = true;
	deque < uint > runtimes;
	while (!shutdown_now) {
		if (globalconfs.coin.max_aggression && !runtimes.empty()) {
			uint avg_runtime = 0;
			for (deque < uint >::iterator it = runtimes.begin();
			     it != runtimes.end(); ++it) {
				avg_runtime += *it;
			}
			avg_runtime /= (uint) runtimes.size();
			if (avg_runtime >
			    TARGET_RUNTIME_MS + TARGET_RUNTIME_ALLOWANCE_MS) {
				globalsize -= localsize;
			} else if (avg_runtime * 3 <
				   TARGET_RUNTIME_MS -
				   TARGET_RUNTIME_ALLOWANCE_MS) {
				globalsize =
				    (globalsize +
				     globalsize / 2) / localsize * localsize;
			} else if (avg_runtime <
				   TARGET_RUNTIME_MS -
				   TARGET_RUNTIME_ALLOWANCE_MS) {
				globalsize += localsize;
			}
		}
		clock_t starttime = ticker();
		if (current_work.old) {
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time) {
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			memcpy(tempdata, &tempwork.data[0], 128);
			*(uint *) & tempdata[100] = state->thread_id;
			base = 0;
			write_kernel_input = true;
		}

		ullint newtime =
		    tempwork.ntime_at_getwork + (ticker() -
						 tempwork.time) / 1000;
		if (*(ullint *) & tempdata[76] != newtime) {
			*(ullint *) & tempdata[76] = newtime;
			write_kernel_input = true;
		}
		if (write_kernel_input) {
			cl_uint8 precalc;
			PreCalc(tempdata, precalc);
			clEnqueueWriteBuffer(state->commandQueue,
					     state->CLbuffer[0], true, 0,
					     KERNEL_INPUT_SIZE, tempdata, 0,
					     NULL, NULL);
			clSetKernelArg(state->kernel, 3, sizeof(cl_uint8),
				       &precalc);
		}
		if (write_kernel_output)
			clEnqueueWriteBuffer(state->commandQueue,
					     state->CLbuffer[1], true, 0,
					     KERNEL_OUTPUT_SIZE * sizeof(uint),
					     kernel_output, 0, NULL, NULL);

		clSetKernelArg(state->kernel, 0, sizeof(cl_mem),
			       &state->CLbuffer[0]);
		clSetKernelArg(state->kernel, 1, sizeof(cl_mem),
			       &state->CLbuffer[1]);

		cl_int returncode;
		returncode =
		    clEnqueueNDRangeKernel(state->commandQueue, state->kernel,
					   1, &base, &globalsize, &localsize, 0,
					   NULL, NULL);
		//OpenCL throws CL_INVALID_KERNEL_ARGS randomly, let's just ignore them.
		if (returncode != CL_SUCCESS
		    && returncode != CL_INVALID_KERNEL_ARGS && errorfree) {
			cout << humantime() << "Error " << returncode <<
			    " while trying to run OpenCL kernel" << endl;
			errorfree = false;
		} else
		    if ((returncode == CL_SUCCESS
			 || returncode == CL_INVALID_KERNEL_ARGS)
			&& !errorfree) {
			cout << humantime() << "Previous OpenCL error cleared"
			    << endl;
			errorfree = true;
		}
		clEnqueueReadBuffer(state->commandQueue, state->CLbuffer[1],
				    true, 0, KERNEL_OUTPUT_SIZE * sizeof(uint),
				    kernel_output, 0, NULL, NULL);

		write_kernel_input = false;
		write_kernel_output = false;
		for (uint i = 0; i < KERNEL_OUTPUT_SIZE; ++i) {
			if (kernel_output[i] == 0)
				continue;
			uint result = kernel_output[i];
			uchar testmem[512];
			uchar finalhash[32];

			if (scversion == 2) {
				memcpy(testmem, tempdata, 128);
				*((uint *) & testmem[108]) = result;
				BlockHash_1(testmem, finalhash);
			}
			if (finalhash[31] != 0 || finalhash[30] != 0
			    || finalhash[29] >= 0x80)
				++shares_hwinvalid;
			else
				++shares_hwvalid;
			bool below = true;
			for (int j = 0; j < 32; ++j) {
				if (finalhash[31 - j] >
				    tempwork.target_share[j]) {
					below = false;
					break;
				}
				if (finalhash[31 - j] <
				    tempwork.target_share[j]) {
					break;
				}
			}
			if (below) {
				vector < uchar > share(testmem, testmem + 128);
				pthread_mutex_lock(&state->share_mutex);
				state->shares_available = true;
				state->shares.
				    push_back(Share
					      (share, tempwork.target_share,
					       tempwork.server_id));
				pthread_mutex_unlock(&state->share_mutex);
			}
			kernel_output[i] = 0;
			write_kernel_output = true;
		}
		if (errorfree) {
			state->hashes += globalsize;
		}
		base += globalsize;
		clock_t endtime = ticker();
		runtimes.push_back(uint(endtime - starttime));
		if (runtimes.size() > RUNTIMES_SIZE)
			runtimes.pop_front();
	}
	pthread_exit(NULL);
	return NULL;
}

#endif

#define Ch(x, y, z) (z ^ (x & (y ^ z)))
#define Ma(x, y, z) ((y & z) | (x & (y | z)))
#undef Tr
#define Tr(x,a,b,c) (rot(x,a)^rot(x,b)^rot(x,c))
#define Wr(x,a,b,c) (rot(x,a)^rot(x,b)^(x>>c))

uint rot(uint a, uint b)
{
	return (a >> b) | (a << (32 - b));
}

void pushvector(vector < uint > &v, uint value, uint vectors)
{
	v.push_back(value);
	if (vectors >= 2)
		v.push_back(value ^ 0x80000000);
	if (vectors >= 4) {
		v.push_back(value ^ 0x40000000);
		v.push_back(value ^ 0xC0000000);
	}
}

void Precalc_BTC(Work & work, uint vectors)
{
	uint *midstate = (uint *) (&work.midstate[0]);
	uint *data = (uint *) (&work.data[64]);

	uint A = midstate[0];
	uint B = midstate[1];
	uint C = midstate[2];
	uint D = midstate[3];
	uint E = midstate[4];
	uint F = midstate[5];
	uint G = midstate[6];
	uint H = midstate[7];

	H += Tr(midstate[4], 25, 11, 6) + Ch(midstate[4], F,
					     G) + 0x428A2F98 + data[0];
	D += H;
	H += Tr(midstate[0], 22, 13, 2) + Ma(midstate[0], B, C);
	G += Tr(D, 25, 11, 6) + Ch(D, midstate[4], F) + 0x71374491 + data[1];
	C += G;
	G += Tr(H, 22, 13, 2) + Ma(H, midstate[0], B);
	F += Tr(C, 25, 11, 6) + Ch(C, D, midstate[4]) + 0xB5C0FBCF + data[2];
	B += F;
	F += Tr(G, 22, 13, 2) + Ma(G, H, midstate[0]);
	E += Tr(B, 25, 11, 6) + Ch(B, C, D) + 0xE9B5DBA5 + Tr(F, 22, 13,
							      2) + Ma(F, G, H);
	A += midstate[4] + Tr(B, 25, 11, 6) + Ch(B, C, D) + 0xE9B5DBA5;
	D += 0xB956C25B;

	uint W16 = data[0] + Wr(data[1], 7, 18, 3);
	uint W17 = data[1] + 0x01100000 + Wr(data[2], 7, 18, 3);
	uint W18_partial = data[2] + Wr(W16, 17, 19, 10);
	uint W19_partial = Wr(W17, 17, 19, 10) + 0x11002000;
	uint W31_partial = 0x280 + Wr(W16, 7, 18, 3);
	uint W32_partial = W16 + Wr(W17, 7, 18, 3);

	uint b_start_k6 = B + 0x923F82A4U;
	uint c_start_k5 = C + 0x59F111F1U;

	work.precalc.clear();

	work.precalc.reserve(32);
	work.precalc.push_back(B);
	work.precalc.push_back(C);
	work.precalc.push_back(D);
	work.precalc.push_back(F);
	work.precalc.push_back(G);
	work.precalc.push_back(H);
	work.precalc.push_back(W16);
	work.precalc.push_back(W17);
	work.precalc.push_back(W18_partial);
	work.precalc.push_back(W31_partial);
	work.precalc.push_back(W32_partial);
	work.precalc.push_back(b_start_k6);
	work.precalc.push_back(c_start_k5);

	work.precalc.push_back(midstate[0] + 0x5F395B94U);

	work.precalc.push_back(W16 + 0xE49B69C1U);
	work.precalc.push_back(W17 + 0xEFBE4786U);

	pushvector(work.precalc, A, vectors);
	pushvector(work.precalc, E, vectors);
	pushvector(work.precalc, W19_partial, vectors);
}

#ifndef CPU_MINING_ONLY

pthread_mutex_t noncemutex = PTHREAD_MUTEX_INITIALIZER;
uint nonce = 0;
uint NextNonce(uint vectors)
{
	pthread_mutex_lock(&noncemutex);
	nonce += globalconfs.coin.global_worksize;
	if (nonce >= 0x80000000 || vectors >= 4 && nonce >= 0x40000000) {
		pthread_mutex_lock(&current_work_mutex);
		uint *realdata = (uint *) (&current_work.data[68]);
		*realdata = EndianSwap(EndianSwap(*realdata) + 1);
		Precalc_BTC(current_work, vectors);
		current_work.time = ticker();
		pthread_mutex_unlock(&current_work_mutex);
		nonce = 0;
	}
	uint ret = nonce;
	pthread_mutex_unlock(&noncemutex);
	return ret;
}

void *Reap_GPU_BTC(void *param)
{
	_clState *state = (_clState *) param;
	state->hashes = 0;

	uint numbre = 0;
	clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[1], true, 0,
			     sizeof(uint), &numbre, 0, NULL, NULL);

	uint vectors = state->vectors;

	size_t globalsize = globalconfs.coin.global_worksize;
	size_t localsize = globalconfs.coin.local_worksize;

	Work tempwork;

	while (true) {
		if (current_work.old) {
			Wait_ms(20);
			continue;
		}
		if (tempwork.time != current_work.time) {
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);

			uint *midstate = (uint *) & tempwork.midstate[0];
			midstate[4] += 0xC67178F2U;
			clSetKernelArg(state->kernel, 0, 4, midstate + 1);
			clSetKernelArg(state->kernel, 1, 4, midstate + 2);
			clSetKernelArg(state->kernel, 2, 4, midstate + 3);
			clSetKernelArg(state->kernel, 3, 4, midstate + 4);
			clSetKernelArg(state->kernel, 4, 4, midstate + 5);
			clSetKernelArg(state->kernel, 5, 4, midstate + 6);
			clSetKernelArg(state->kernel, 6, 4, midstate + 7);
			midstate[4] -= 0xC67178F2U;
		}

		uint *precalc = &tempwork.precalc[0];
		uint number = 7;
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//B
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//C
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//D
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//F
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//G
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//H

		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W16
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W17
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W18
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W31
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W32
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//Badd6
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//Cadd5

		clSetKernelArg(state->kernel, number++, 4, precalc++);	//state0p

		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W16p
		clSetKernelArg(state->kernel, number++, 4, precalc++);	//W17p

		clSetKernelArg(state->kernel, number++, sizeof(uint) * vectors, precalc);	//A
		precalc += vectors;
		clSetKernelArg(state->kernel, number++, sizeof(uint) * vectors, precalc);	//E
		precalc += vectors;
		clSetKernelArg(state->kernel, number++, sizeof(uint) * vectors, precalc);	//W19
		precalc += vectors;

		clSetKernelArg(state->kernel, number, sizeof(cl_mem),
			       &state->CLbuffer[1]);
		size_t base = NextNonce(vectors);
		clEnqueueNDRangeKernel(state->commandQueue, state->kernel, 1,
				       &base, &globalsize, &localsize, 0, NULL,
				       NULL);
		uint result = 0;
		clEnqueueReadBuffer(state->commandQueue, state->CLbuffer[1],
				    true, 0, sizeof(uint), &result, 0, NULL,
				    NULL);
		state->hashes += globalsize * state->vectors;
		if (result) {
			uint *data = (uint *) & tempwork.data[76];
			*data = result;
			pthread_mutex_lock(&state->share_mutex);
			//todo: check share target
			state->shares.
			    push_back(Share
				      (tempwork.data, tempwork.target_share,
				       tempwork.server_id));
			state->shares_available = true;
			pthread_mutex_unlock(&state->share_mutex);

			result = 0;
			clEnqueueWriteBuffer(state->commandQueue,
					     state->CLbuffer[1], true, 0,
					     sizeof(uint), &result, 0, NULL,
					     NULL);
		}
	}
	pthread_exit(NULL);
}

int scanhash_scrypt(unsigned char *pdata, unsigned char *scratchbuf,
		    const unsigned char *ptarget);

vector < uchar > CalculateMidstate(vector < uchar > in);

void *Reap_GPU_LTC(void *param)
{
	const uint TOTAL_OUTPUTS = KERNEL_OUTPUT_SIZE;
	_clState *state = (_clState *) param;
	state->hashes = 0;

	size_t globalsize = globalconfs.coin.global_worksize;
	size_t localsize = globalconfs.coin.local_worksize;

	Work tempwork;

	size_t nonce = state->offset;

	uint gpu_outputs[TOTAL_OUTPUTS] = { };
	clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[1], true, 0,
			     TOTAL_OUTPUTS * sizeof(uint), gpu_outputs, 0, NULL,
			     NULL);

	vector < uchar > midstate;

	while (true) {
		if (current_work.old) {
			Wait_ms(20);
			continue;
		}

		if (tempwork.time != current_work.time) {
			pthread_mutex_lock(&current_work_mutex);
			tempwork = current_work;
			pthread_mutex_unlock(&current_work_mutex);
			nonce = state->offset;
			midstate = CalculateMidstate(tempwork.data);
		}
		clEnqueueWriteBuffer(state->commandQueue, state->CLbuffer[0],
				     true, 0, 80, &tempwork.data[0], 0, NULL,
				     NULL);
		clSetKernelArg(state->kernel, 0, sizeof(cl_mem),
			       &state->CLbuffer[0]);
		clSetKernelArg(state->kernel, 1, sizeof(cl_mem),
			       &state->CLbuffer[1]);
		clSetKernelArg(state->kernel, 2, sizeof(cl_mem),
			       &state->padbuffer8);
		clSetKernelArg(state->kernel, 3, sizeof(cl_uint4),
			       &midstate[0]);
		clSetKernelArg(state->kernel, 4, sizeof(cl_uint4),
			       &midstate[16]);
		clEnqueueNDRangeKernel(state->commandQueue, state->kernel, 1,
				       &nonce, &globalsize, &localsize, 0, NULL,
				       NULL);
		clEnqueueReadBuffer(state->commandQueue, state->CLbuffer[1],
				    true, 0, TOTAL_OUTPUTS * sizeof(uint),
				    gpu_outputs, 0, NULL, NULL);
		bool writeoutput = false;
		for (uint i = 0; i < TOTAL_OUTPUTS; ++i) {
			if (!gpu_outputs[i])
				continue;
			uint *data = (uint *) & tempwork.data[76];
			*data = gpu_outputs[i];
			bool result = true;

			if (result) {
				pthread_mutex_lock(&state->share_mutex);
				state->shares.
				    push_back(Share
					      (tempwork.data,
					       tempwork.target_share,
					       tempwork.server_id));
				state->shares_available = true;
				pthread_mutex_unlock(&state->share_mutex);
			} else
				++shares_hwvalid;
			writeoutput = true;
			gpu_outputs[i] = 0;
		}
		if (writeoutput)
			clEnqueueWriteBuffer(state->commandQueue,
					     state->CLbuffer[1], true, 0,
					     TOTAL_OUTPUTS * sizeof(uint),
					     gpu_outputs, 0, NULL, NULL);
		state->hashes += globalsize;
		nonce += globalsize;
	}
	pthread_exit(NULL);
}

_clState clState;

#endif

#include "Config.h"
extern Config config;
void OpenCL::Init()
{
#ifdef CPU_MINING_ONLY
	if (globalconfs.coin.threads_per_gpu != 0) {
		cout << "This binary was built with CPU mining support only." <<
		    endl;
	}
#else
	if (globalconfs.coin.threads_per_gpu == 0) {
		cout << "No GPUs selected." << endl;
		return;
	}

	cl_int status = 0;

	cl_uint numPlatforms;
	cl_platform_id platform = NULL;

	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
		throw string("Error getting OpenCL platforms");

	if (numPlatforms > 0) {
		cl_platform_id *platforms = new cl_platform_id[numPlatforms];

		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if (status != CL_SUCCESS)
			throw string("Error getting OpenCL platform IDs");

		unsigned int i;
		cout << "List of platforms:" << endl;
		for (i = 0; i < numPlatforms; ++i) {
			char pbuff[100];

			status =
			    clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
					      sizeof(pbuff), pbuff, NULL);
			if (status != CL_SUCCESS) {
				delete[]platforms;
				throw
				    string
				    ("Error getting OpenCL platform info");
			}

			cout << "\t" << i << "\t" << pbuff << endl;
			if (globalconfs.platform == i) {
				platform = platforms[i];
			}
		}
		delete[]platforms;
	} else {
		throw string("No OpenCL platforms found");
	}

	if (platform == NULL) {
		throw string("Chosen platform number does not exist");
	}
	cout << "Using platform number " << globalconfs.platform << endl;

	cl_uint numDevices;
	cl_uint devicetype = CL_DEVICE_TYPE_ALL;
	status = clGetDeviceIDs(platform, devicetype, 0, NULL, &numDevices);
	if (status != CL_SUCCESS) {
		throw string("Error getting OpenCL device IDs");
	}

	if (numDevices == 0)
		throw string("No OpenCL devices found");

	vector < cl_device_id > devices;
	cl_device_id *devicearray = new cl_device_id[numDevices];

	status =
	    clGetDeviceIDs(platform, devicetype, numDevices, devicearray, NULL);
	if (status != CL_SUCCESS)
		throw string("Error getting OpenCL device ID list");

	for (uint i = 0; i < numDevices; ++i)
		devices.push_back(devicearray[i]);

	cl_context_properties cps[3] =
	    { CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0 };

	clState.context =
	    clCreateContextFromType(cps, devicetype, NULL, NULL, &status);
	if (status != CL_SUCCESS)
		throw string("Error creating OpenCL context");

	cout << endl;
	if (globalconfs.devices.empty()) {
		cout << "Using all devices" << endl;
	} else {
		cout << "Using device" << (globalconfs.devices.size() ==
					   1 ? "" : "s") << " ";
		for (uint i = 0; i < globalconfs.devices.size(); ++i) {
			cout << globalconfs.devices[i];
			if (i + 1 < globalconfs.devices.size()) {
				cout << ", ";
			}
		}
		cout << endl;
	}
	for (uint device_id = 0; device_id < numDevices; ++device_id) {
		string source;
		string sourcefilename;
		{
			sourcefilename =
			    config.GetCombiValue < string > ("device",
							     device_id,
							     "kernel");
			if (sourcefilename == "")
				sourcefilename =
				    config.GetValue < string > ("kernel");
			sourcefilename =
			    globalconfs.coin.protocol + "-" + sourcefilename;
			FILE *filu = fopen(sourcefilename.c_str(), "rb");
			if (filu == NULL) {
				throw string("Couldn't find kernel file ") +
				    sourcefilename;
			}
			fseek(filu, 0, SEEK_END);
			uint size = ftell(filu);
			fseek(filu, 0, SEEK_SET);
			size_t readsize = 0;
			for (uint i = 0; i < size; ++i) {
				char c;
				readsize += fread(&c, 1, 1, filu);
				source.push_back(c);
			}
			if (readsize != size) {
				cout <<
				    "Read error while reading kernel source " <<
				    sourcefilename << endl;
			}
		}

		vector < size_t > sourcesizes;
		sourcesizes.push_back(source.length());

		const char *see = source.c_str();

		char pbuff[512];
		status =
		    clGetDeviceInfo(devices[device_id], CL_DEVICE_NAME,
				    sizeof(pbuff), pbuff, NULL);
		cout << "\t" << device_id << "\t" << pbuff;
		if (status != CL_SUCCESS)
			throw string("Error getting OpenCL device info");

		if (!globalconfs.devices.empty()
		    && std::find(globalconfs.devices.begin(),
				 globalconfs.devices.end(),
				 device_id) == globalconfs.devices.end()) {
			cout << " (disabled)" << endl;
			continue;
		}

		cout << endl;

		uchar *filebinary = NULL;
		size_t filebinarysize = 0;
		string filebinaryname;
		for (char *p = &pbuff[0]; *p != 0; ++p) {
			//get rid of unwanted characters in filenames
			if (*p >= 33 && *p < 127 && *p != '\\' && *p != ':'
			    && *p != '/' && *p != '*' && *p != '<' && *p != '>'
			    && *p != '"' && *p != '?' && *p != '|')
				filebinaryname += *p;
		}
		filebinaryname +=
		    string("-") + ToString(globalconfs.coin.local_worksize);
		if (globalconfs.coin.protocol == "litecoin") {
			filebinaryname +=
			    string("-") +
			    ToString(globalconfs.coin.config.GetValue < string >
				     ("gpu_thread_concurrency"));
			filebinaryname +=
			    string("-") +
			    ToString(globalconfs.coin.config.GetValue < string >
				     ("lookup_gap"));
		}
		if (globalconfs.coin.protocol == "bitcoin") {
			filebinaryname +=
			    string("-") +
			    ToString(globalconfs.coin.config.GetValue < string >
				     ("vectors"));
		}

		filebinaryname =
		    sourcefilename.substr(0,
					  sourcefilename.size() - 3) +
		    REAPER_VERSION + "." + filebinaryname + ".bin";
		if (globalconfs.save_binaries) {
			FILE *filu = fopen(filebinaryname.c_str(), "rb");
			if (filu != NULL) {
				fseek(filu, 0, SEEK_END);
				uint size = ftell(filu);
				fseek(filu, 0, SEEK_SET);
				if (size > 0) {
					filebinary = new uchar[size];
					filebinarysize = size;
					size_t readsize =
					    fread(filebinary, size, 1, filu);
					if (readsize != 1) {
						cout <<
						    "Read error while reading binary"
						    << endl;
					}
				}
				fclose(filu);
			}
		}

		_clState GPUstate;
		status =
		    clGetDeviceInfo(devices[device_id], CL_DEVICE_EXTENSIONS,
				    sizeof(pbuff), pbuff, NULL);
		vector < string > extensions = Explode(string(pbuff), ' ');

		if (filebinary == NULL) {
			cout <<
			    "Compiling kernel... this could take up to 2 minutes."
			    << endl;
			GPUstate.program =
			    clCreateProgramWithSource(clState.context, 1,
						      (const char **)&see,
						      &sourcesizes[0], &status);
			if (status != CL_SUCCESS)
				throw
				    string
				    ("Error creating OpenCL program from source");

			string compile_options;
			compile_options +=
			    " -D WORKSIZE=" +
			    ToString(globalconfs.coin.local_worksize);

			bool amd_gpu =
			    (std::
			     find(extensions.begin(), extensions.end(),
				  "cl_amd_media_ops") != extensions.end());

			if (amd_gpu)
				compile_options += " -D AMD_GPU";

			compile_options += " -D SHAREMASK=";
			compile_options +=
			    globalconfs.coin.config.GetValue < string >
			    ("gpu_sharemask");
			uint vectors = GetVectorSize();
			if (vectors == 2)
				compile_options += " -D VECTORS";
			else if (vectors == 4)
				compile_options += " -D VECTORS4";
			if (globalconfs.coin.protocol == "litecoin") {
				compile_options +=
				    string(" -D LOOKUP_GAP=") +
				    globalconfs.coin.config.GetValue < string >
				    ("lookup_gap");
				compile_options +=
				    string(" -D CONCURRENT_THREADS=") +
				    globalconfs.coin.config.GetValue < string >
				    ("gpu_thread_concurrency");
			}
			status =
			    clBuildProgram(GPUstate.program, 1,
					   &devices[device_id],
					   compile_options.c_str(), NULL, NULL);
			if (status != CL_SUCCESS) {
				size_t logSize;
				status =
				    clGetProgramBuildInfo(GPUstate.program,
							  devices[device_id],
							  CL_PROGRAM_BUILD_LOG,
							  0, NULL, &logSize);

				char *log = new char[logSize];
				status =
				    clGetProgramBuildInfo(GPUstate.program,
							  devices[device_id],
							  CL_PROGRAM_BUILD_LOG,
							  logSize, log, NULL);
				cout << log << endl;
				delete[]log;
				throw string("Error building OpenCL program");
			}

			uint device_amount;
			clGetProgramInfo(GPUstate.program,
					 CL_PROGRAM_NUM_DEVICES, sizeof(uint),
					 &device_amount, NULL);

			size_t *binarysizes = new size_t[device_amount];
			uchar **binaries = new uchar *[device_amount];
			for (uint curr_binary = 0; curr_binary < device_amount;
			     ++curr_binary) {
				clGetProgramInfo(GPUstate.program,
						 CL_PROGRAM_BINARY_SIZES,
						 device_amount * sizeof(size_t),
						 binarysizes, NULL);
				binaries[curr_binary] =
				    new uchar[binarysizes[curr_binary]];
			}
			clGetProgramInfo(GPUstate.program, CL_PROGRAM_BINARIES,
					 sizeof(uchar *) * device_amount,
					 binaries, NULL);
			for (uint binary_id = 0; binary_id < device_amount;
			     ++binary_id) {
				if (binarysizes[binary_id] == 0)
					continue;

				cout << "Binary size: " <<
				    binarysizes[binary_id] << " bytes" << endl;
				if (globalconfs.save_binaries) {
					FILE *filu =
					    fopen(filebinaryname.c_str(), "wb");
					fwrite(binaries[binary_id],
					       binarysizes[binary_id], 1, filu);
					fclose(filu);
				}
			}

			cout << "Program built from source." << endl;
			delete[]binarysizes;
			for (uint binary_id = 0; binary_id < device_amount;
			     ++binary_id)
				delete[]binaries[binary_id];
			delete[]binaries;
		} else {
			cl_int binary_status, errorcode_ret;
			GPUstate.program =
			    clCreateProgramWithBinary(clState.context, 1,
						      &devices[device_id],
						      &filebinarysize,
						      const_cast <
						      const uchar **
						      >(&filebinary),
						      &binary_status,
						      &errorcode_ret);
			if (binary_status != CL_SUCCESS)
				cout << "Binary status error code: " <<
				    binary_status << endl;
			if (errorcode_ret != CL_SUCCESS)
				cout << "Binary loading error code: " <<
				    errorcode_ret << endl;
			status =
			    clBuildProgram(GPUstate.program, 1,
					   &devices[device_id], NULL, NULL,
					   NULL);
			if (status != CL_SUCCESS)
				cout << "Error while building from binary: " <<
				    status << endl;

			cout << "Program built from saved binary." << endl;
		}
		delete[]filebinary;

		GPUstate.kernel =
		    clCreateKernel(GPUstate.program, "search", &status);
		if (status != CL_SUCCESS) {
			cout << "Kernel build not successful: " << status <<
			    endl;
			throw string("Error creating OpenCL kernel");
		}
		cl_mem padbuffer8;
		if (globalconfs.coin.protocol == "solidcoin"
		    || globalconfs.coin.protocol == "solidcoin3") {
			padbuffer8 =
			    clCreateBuffer(clState.context, CL_MEM_READ_ONLY,
					   1024 * 1024 * 4 + 8, NULL, &status);
		} else if (globalconfs.coin.protocol == "litecoin") {
			ullint lookup_gap =
			    globalconfs.coin.config.GetValue < ullint >
			    ("lookup_gap");
			ullint thread_concurrency =
			    globalconfs.coin.config.GetValue < ullint >
			    ("gpu_thread_concurrency");
			ullint itemsperthread =
			    (1024 / lookup_gap + (1024 % lookup_gap > 0));
			ullint bufsize =
			    128 * itemsperthread * thread_concurrency;
			cout << "LTC buffer size: " << bufsize / 1024.0 /
			    1024.0 << "MB." << endl;
			padbuffer8 =
			    clCreateBuffer(clState.context, CL_MEM_READ_WRITE,
					   bufsize, NULL, &status);
			if (status != 0) {
				cout <<
				    "Buffer too big: allocation failed. Either raise 'lookup_gap' or lower 'gpu_thread_concurrency'."
				    << endl;
				throw string("");
			}
		}
		for (uint thread_id = 0;
		     thread_id < globalconfs.coin.threads_per_gpu;
		     ++thread_id) {
			GPUstate.commandQueue =
			    clCreateCommandQueue(clState.context,
						 devices[device_id], 0,
						 &status);
			if (thread_id == 0
			    && (globalconfs.coin.protocol == "solidcoin"
				|| globalconfs.coin.protocol == "solidcoin3")) {
				clEnqueueWriteBuffer(GPUstate.commandQueue,
						     padbuffer8, true, 0,
						     1024 * 1024 * 4 + 8,
						     BlockHash_1_MemoryPAD8, 0,
						     NULL, NULL);
			}
			if (status != CL_SUCCESS)
				throw
				    string
				    ("Error creating OpenCL command queue");

			GPUstate.CLbuffer[0] =
			    clCreateBuffer(clState.context, CL_MEM_READ_ONLY,
					   KERNEL_INPUT_SIZE, NULL, &status);
			GPUstate.CLbuffer[1] =
			    clCreateBuffer(clState.context, CL_MEM_WRITE_ONLY,
					   KERNEL_OUTPUT_SIZE * sizeof(uint),
					   NULL, &status);
			GPUstate.padbuffer8 = padbuffer8;

			if (status != CL_SUCCESS) {
				cout << status << endl;
				throw string("Error creating OpenCL buffer");
			}

			GPUstate.offset =
			    0x100000000ULL / globalconfs.coin.threads_per_gpu /
			    numDevices * (device_id *
					  globalconfs.coin.threads_per_gpu +
					  thread_id);

			pthread_mutex_t initializer = PTHREAD_MUTEX_INITIALIZER;

			GPUstate.share_mutex = initializer;
			GPUstate.shares_available = false;

			GPUstate.vectors = GetVectorSize();
			GPUstate.thread_id = device_id * numDevices + thread_id;
			GPUstates.push_back(GPUstate);
		}
	}

	if (GPUstates.empty()) {
		cout << "No GPUs selected." << endl;
		return;
	}

	cout << "Creating " << GPUstates.size() << " GPU threads" << endl;
	for (uint i = 0; i < GPUstates.size(); ++i) {
		cout << i + 1 << "...";
		if (globalconfs.coin.protocol == "bitcoin")
			pthread_create(&GPUstates[i].thread, NULL, Reap_GPU_BTC,
				       (void *)&GPUstates[i]);
		else if (globalconfs.coin.protocol == "solidcoin"
			 || globalconfs.coin.protocol == "solidcoin3")
			pthread_create(&GPUstates[i].thread, NULL, Reap_GPU_SLC,
				       (void *)&GPUstates[i]);
		else if (globalconfs.coin.protocol == "litecoin")
			pthread_create(&GPUstates[i].thread, NULL, Reap_GPU_LTC,
				       (void *)&GPUstates[i]);
	}
	cout << "done" << endl;
#endif
}

void OpenCL::Quit()
{

}
