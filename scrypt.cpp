/*-
 * Copyright 2009 Colin Percival, 2011 ArtForz, 2011 pooler, 2012 mtrlt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND I AM A FUCKING MORON BECAUSE
 * I KEEP SHOUTING RIDICULOUS SHIT AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */

#include "Global.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

uint EndianSwap(uint n);

void memorycopy(uint* dst, const uint* src, uint len)
{
	for(uint i=0; i<len/4; ++i)
		dst[i] = src[i];
}

static inline void
byteswap_vec(uint *dest, const uint *src, uint len)
{
	for (uint i = 0; i < len; i++)
		dest[i] = EndianSwap(src[i]);
}

typedef struct SHA256Context {
	uint state[8];
	uint buf[16];
} SHA256_CTX;

/* Elementary functions used by SHA256 */
#define Ch(x, y, z)	((x & (y ^ z)) ^ z)
#define Maj(x, y, z)	((x & (y | z)) | (y & z))
#define SHR(x, n)	(x >> n)
#define ROTR(x, n)	((x >> n) | (x << (32 - n)))
#define S0(x)		(ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)		(ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)		(ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x)		(ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

/* SHA256 round function */
#define RND(a, b, c, d, e, f, g, h, k)			\
	t0 = h + S1(e) + Ch(e, f, g) + k;		\
	t1 = S0(a) + Maj(a, b, c);			\
	d += t0;					\
	h  = t0 + t1;

/* Adjusted round function for rotating state */
#define RNDr(S, W, i, k)			\
	RND(S[(64 - i) % 8], S[(65 - i) % 8],	\
	    S[(66 - i) % 8], S[(67 - i) % 8],	\
	    S[(68 - i) % 8], S[(69 - i) % 8],	\
	    S[(70 - i) % 8], S[(71 - i) % 8],	\
	    W[i] + k)

/*
 * SHA256 block compression function.  The 256-bit state is transformed via
 * the 512-bit input block to produce a new state.
 */
static void
SHA256_Transform(uint * state, const uint* block, int swap)
{
	uint W[64];
	uint S[8];
	uint t0, t1;
	int i;

	/* 1. Prepare message schedule W. */
	if(swap)
		byteswap_vec(W, block, 16);
	else
		memorycopy(W, block, 64);
	for (i = 16; i < 64; i += 2) {
		W[i] = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];
		W[i+1] = s1(W[i - 1]) + W[i - 6] + s0(W[i - 14]) + W[i - 15];
	}

	/* 2. Initialize working variables. */
	memorycopy(S, state, 32);

	/* 3. Mix. */
	RNDr(S, W, 0, 0x428a2f98);
	RNDr(S, W, 1, 0x71374491);
	RNDr(S, W, 2, 0xb5c0fbcf);
	RNDr(S, W, 3, 0xe9b5dba5);
	RNDr(S, W, 4, 0x3956c25b);
	RNDr(S, W, 5, 0x59f111f1);
	RNDr(S, W, 6, 0x923f82a4);
	RNDr(S, W, 7, 0xab1c5ed5);
	RNDr(S, W, 8, 0xd807aa98);
	RNDr(S, W, 9, 0x12835b01);
	RNDr(S, W, 10, 0x243185be);
	RNDr(S, W, 11, 0x550c7dc3);
	RNDr(S, W, 12, 0x72be5d74);
	RNDr(S, W, 13, 0x80deb1fe);
	RNDr(S, W, 14, 0x9bdc06a7);
	RNDr(S, W, 15, 0xc19bf174);
	RNDr(S, W, 16, 0xe49b69c1);
	RNDr(S, W, 17, 0xefbe4786);
	RNDr(S, W, 18, 0x0fc19dc6);
	RNDr(S, W, 19, 0x240ca1cc);
	RNDr(S, W, 20, 0x2de92c6f);
	RNDr(S, W, 21, 0x4a7484aa);
	RNDr(S, W, 22, 0x5cb0a9dc);
	RNDr(S, W, 23, 0x76f988da);
	RNDr(S, W, 24, 0x983e5152);
	RNDr(S, W, 25, 0xa831c66d);
	RNDr(S, W, 26, 0xb00327c8);
	RNDr(S, W, 27, 0xbf597fc7);
	RNDr(S, W, 28, 0xc6e00bf3);
	RNDr(S, W, 29, 0xd5a79147);
	RNDr(S, W, 30, 0x06ca6351);
	RNDr(S, W, 31, 0x14292967);
	RNDr(S, W, 32, 0x27b70a85);
	RNDr(S, W, 33, 0x2e1b2138);
	RNDr(S, W, 34, 0x4d2c6dfc);
	RNDr(S, W, 35, 0x53380d13);
	RNDr(S, W, 36, 0x650a7354);
	RNDr(S, W, 37, 0x766a0abb);
	RNDr(S, W, 38, 0x81c2c92e);
	RNDr(S, W, 39, 0x92722c85);
	RNDr(S, W, 40, 0xa2bfe8a1);
	RNDr(S, W, 41, 0xa81a664b);
	RNDr(S, W, 42, 0xc24b8b70);
	RNDr(S, W, 43, 0xc76c51a3);
	RNDr(S, W, 44, 0xd192e819);
	RNDr(S, W, 45, 0xd6990624);
	RNDr(S, W, 46, 0xf40e3585);
	RNDr(S, W, 47, 0x106aa070);
	RNDr(S, W, 48, 0x19a4c116);
	RNDr(S, W, 49, 0x1e376c08);
	RNDr(S, W, 50, 0x2748774c);
	RNDr(S, W, 51, 0x34b0bcb5);
	RNDr(S, W, 52, 0x391c0cb3);
	RNDr(S, W, 53, 0x4ed8aa4a);
	RNDr(S, W, 54, 0x5b9cca4f);
	RNDr(S, W, 55, 0x682e6ff3);
	RNDr(S, W, 56, 0x748f82ee);
	RNDr(S, W, 57, 0x78a5636f);
	RNDr(S, W, 58, 0x84c87814);
	RNDr(S, W, 59, 0x8cc70208);
	RNDr(S, W, 60, 0x90befffa);
	RNDr(S, W, 61, 0xa4506ceb);
	RNDr(S, W, 62, 0xbef9a3f7);
	RNDr(S, W, 63, 0xc67178f2);

	/* 4. Mix local working variables into global state */
	for (i = 0; i < 8; i++)
		state[i] += S[i];
}

static inline void
SHA256_InitState(uint * state)
{
	/* Magic initialization constants */
	state[0] = 0x6A09E667;
	state[1] = 0xBB67AE85;
	state[2] = 0x3C6EF372;
	state[3] = 0xA54FF53A;
	state[4] = 0x510E527F;
	state[5] = 0x9B05688C;
	state[6] = 0x1F83D9AB;
	state[7] = 0x5BE0CD19;
}

static const uint passwdpad[12] = {0x00000080, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80020000};
static const uint outerpad[8] = {0x80000000, 0, 0, 0, 0, 0, 0, 0x00000300};

static inline void
PBKDF2_SHA256_80_128_init(const uint *passwd, uint* tstate, uint* ostate)
{
	uint ihash[8];
	uint pad[16];
	uint i;
	memorycopy(pad, passwd+16, 16);
	memorycopy(pad+4, passwdpad, 48);

	SHA256_InitState(tstate);
	SHA256_Transform(tstate, passwd, 1);
	SHA256_Transform(tstate, pad, 1);
	memorycopy(ihash, tstate, 32);

	SHA256_InitState(ostate);
	for (i = 0; i < 8; i++)
		pad[i] = ihash[i] ^ 0x5c5c5c5c;
	for (; i < 16; i++)
		pad[i] = 0x5c5c5c5c;
	SHA256_Transform(ostate, pad, 0);

	SHA256_InitState(tstate);
	for (i = 0; i < 8; i++)
		pad[i] = ihash[i] ^ 0x36363636;
	for (; i < 16; i++)
		pad[i] = 0x36363636;
	SHA256_Transform(tstate, pad, 0);
}

/**
 * PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA256 as the PRF, and
 * write the output to buf.
 */
static inline void
PBKDF2_SHA256_80_128(const uint *tstate, const uint *ostate, const uint *passwd, uint *buf)
{
	static const uint innerpad[11] = {0x00000080, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xa0040000};
	SHA256_CTX PShictx, PShoctx;
	uint i;
	
	/* If Klen > 64, the key is really SHA256(K). */
	memorycopy(PShictx.state, tstate, 32);
	memorycopy(PShoctx.state, ostate, 32);
	
	memorycopy(PShoctx.buf+8, outerpad, 32);

	SHA256_Transform(PShictx.state, passwd, 1);
	byteswap_vec(PShictx.buf, passwd+16, 4);
	byteswap_vec(PShictx.buf+5, innerpad, 11);

	/* Iterate through the blocks. */
	for (i = 0; i < 4; i++) {
		uint ist[8];
		uint ost[8];
		
		memorycopy(ist, PShictx.state, 32);
		PShictx.buf[4] = i + 1;
		SHA256_Transform(ist, PShictx.buf, 0);
		memorycopy(PShoctx.buf, ist, 32);

		memorycopy(ost, PShoctx.state, 32);
		SHA256_Transform(ost, PShoctx.buf, 0);
		byteswap_vec(buf+i*8, ost, 8);
	}
}

static inline void
PBKDF2_SHA256_80_128_32(uint *tstate, uint *ostate, const uint *salt, uint *output)
{
	static const uint ihash_finalblk[16] = {0x00000001,0x80000000,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0x00000620};
	uint pad[16];
	
	SHA256_Transform(tstate, salt, 1);
	SHA256_Transform(tstate, salt+16, 1);
	SHA256_Transform(tstate, ihash_finalblk, 0);
	memorycopy(pad, tstate, 32);
	memorycopy(pad+8, outerpad, 32);

	SHA256_Transform(ostate, pad, 0);
	byteswap_vec(output, ostate, 8);
}


/**
 * salsa20_8(B):
 * Apply the salsa20/8 core to the provided block.
 */
static inline void
salsa20_8(uint B[16], const uint Bx[16])
{
	uint w[16];

	for(int i=0; i<16; ++i)
		w[i] = (B[i]^=Bx[i]);
	for (int i = 0; i < 8; i += 2) {
#define R(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
		/* Operate on columns. */
		w[ 4] ^= R(w[ 0]+w[12], 7);	w[ 9] ^= R(w[ 5]+w[ 1], 7);	w[14] ^= R(w[10]+w[ 6], 7);	w[ 3] ^= R(w[15]+w[11], 7);
		w[ 8] ^= R(w[ 4]+w[ 0], 9);	w[13] ^= R(w[ 9]+w[ 5], 9);	w[ 2] ^= R(w[14]+w[10], 9);	w[ 7] ^= R(w[ 3]+w[15], 9);
		w[12] ^= R(w[ 8]+w[ 4],13);	w[ 1] ^= R(w[13]+w[ 9],13);	w[ 6] ^= R(w[ 2]+w[14],13);	w[11] ^= R(w[ 7]+w[ 3],13);
		w[ 0] ^= R(w[12]+w[ 8],18);	w[ 5] ^= R(w[ 1]+w[13],18);	w[10] ^= R(w[ 6]+w[ 2],18);	w[15] ^= R(w[11]+w[ 7],18);

		/* Operate on rows. */
		w[ 1] ^= R(w[ 0]+w[ 3], 7); w[ 6] ^= R(w[ 5]+w[ 4], 7); w[11] ^= R(w[10]+w[ 9], 7); w[12] ^= R(w[15]+w[14], 7);
		w[ 2] ^= R(w[ 1]+w[ 0], 9); w[ 7] ^= R(w[ 6]+w[ 5], 9); w[ 8] ^= R(w[11]+w[10], 9); w[13] ^= R(w[12]+w[15], 9);
		w[ 3] ^= R(w[ 2]+w[ 1],13); w[ 4] ^= R(w[ 7]+w[ 6],13); w[ 9] ^= R(w[ 8]+w[11],13); w[14] ^= R(w[13]+w[12],13);
		w[ 0] ^= R(w[ 3]+w[ 2],18); w[ 5] ^= R(w[ 4]+w[ 7],18); w[10] ^= R(w[ 9]+w[ 8],18); w[15] ^= R(w[14]+w[13],18);
#undef R
	}
	for(int i=0; i<16; ++i)
		B[i]+=w[i];
}

static inline void scrypt_core(uint *X, uint *V)
{
	for (int i = 0; i < 1024; ++i)
	{
		memorycopy(&V[i * 32], X, 128);
		salsa20_8(&X[0], &X[16]);
		salsa20_8(&X[16], &X[0]);
	}
	for (int i = 0; i < 1024; ++i) 
	{
		uint j = X[16] & 0x3FF;

		uint* p2 = (uint *)(&V[j * 32]);
		for(int k = 0; k < 32; k++)
			X[k] ^= p2[k];

		salsa20_8(&X[0], &X[16]);
		salsa20_8(&X[16], &X[0]);
	}
}

/* cpu and memory intensive function to transform a 80 byte buffer into a 32 byte output
   scratchpad size needs to be at least 63 + (128 * r * p) + (256 * r + 64) + (128 * r * N) bytes
   r = 1, p = 1, N = 1024
 */
static void scrypt_1024_1_1_256_sp(uint* input, uint *output, unsigned char *scratchpad)
{
	uint tstate[8], ostate[8], X[32];
	PBKDF2_SHA256_80_128_init(input, tstate, ostate);
	PBKDF2_SHA256_80_128(tstate, ostate, input, X);
	scrypt_core(X, (uint*)scratchpad);
	PBKDF2_SHA256_80_128_32(tstate, ostate, X, output);
}

int test_lower_hash(const uint *hash,
	const uint *target)
{
	int i;
	for (i = 6; i >= 0; i--) {
		uint t = *(uint*)(&target[i]);
		if (hash[i] > t)
			return 0;
		if (hash[i] < t)
			return 1;
	}
	return 1;
}

SHARETEST_VALUE scanhash_scrypt(unsigned char *pdata, unsigned char* scratchbuf,
	const unsigned char *ptarget)
{
	uint data[20], hash[8];
	uint Htarg = *(uint*)(ptarget+28);

	

	for (int i = 0; i < 20; i++)
		data[i] = EndianSwap(((uint*)pdata)[i]);
	scrypt_1024_1_1_256_sp(data, hash, scratchbuf);

	SHARETEST_VALUE ret;
	if (hash[7] > 0xFFFF)
	{
		return ST_HNOTZERO;
	}
	if (hash[7] < Htarg || (hash[7] == Htarg && test_lower_hash(hash, (uint *)ptarget)))
		return ST_GOOD;
	return ST_MORETHANTARGET;
}
