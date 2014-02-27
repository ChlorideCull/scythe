#define rotl(x,y) rotate(x,y)

uint EndianSwap(uint n)
{
	return rotl(n&0x00FF00FF,24U)|rotl(n&0xFF00FF00,8U);
}

#define U8TO32(p) \
  EndianSwap(*(__local uint*)(p))
#define U8TO64(p) \
  (((ulong)U8TO32(p) << 32) | (ulong)U8TO32((p) + 4))
#define U32TO8(p,v) \
	(*(__local uint*)(p)) = EndianSwap(v);
#define U64TO8(p, v) \
    U32TO8((p),     (uint)((v) >> 32));	\
    U32TO8((p) + 4, (uint)((v)      )); 

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__constant uchar sigma[256] = 
{
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 ,
    14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3 ,
    11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4 ,
     7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8 ,
     9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13 ,
     2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9 ,
    12, 5, 1,15,14,13, 4,10, 0, 7, 6, 3, 9, 2, 8,11 ,
    13,11, 7,14,12, 1, 3, 9, 5, 0,15, 4, 8, 6, 2,10 ,
     6,15,14, 9,11, 3, 0, 8,12, 2,13, 7, 1, 4,10, 5 ,
    10, 2, 8, 4, 7, 6, 1, 5,15,11, 9,14, 3,12,13 ,0 ,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 ,
    14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3 ,
    11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4 ,
     7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8 ,
     9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13 ,
     2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9 
};

__constant ulong cst[16] = 
{
  0x243F6A8885A308D3UL,0x13198A2E03707344UL,0xA4093822299F31D0UL,0x082EFA98EC4E6C89UL,
  0x452821E638D01377UL,0xBE5466CF34E90C6CUL,0xC0AC29B7C97C50DDUL,0x3F84D5B5B5470917UL,
  0x9216D5D98979FB1BUL,0xD1310BA698DFB5ACUL,0x2FFD72DBD01ADFB7UL,0xB8E1AFED6A267E96UL,
  0xBA7C9045F12C7F99UL,0x24A19947B3916CF7UL,0x0801F2E2858EFC16UL,0x636920D871574E69UL
};

__constant uint K[64] = 
{ 
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#ifdef BFI_INT
#pragma OPENCL EXTENSION cl_amd_media_ops : enable
#define Ch(x,y,z) amd_sad(x,y,z)
#define Ma(x, y, z) Ch((x^z),y,z)
#else
#define Ch(x, y, z) (z^(x&(y^z)))
#define Ma(x, y, z) Ch((x^z),y,z)
#endif

#define Ch2(x, y, z) (z^(x&(y^z)))
#define Ma2(x, y, z) Ch((x^z),y,z)

#define Tr(x,a,b,c) (rotl(x,a)^rotl(x,b)^rotl(x,c))
#define Wr(x,a,b,c) (rotl(x,a)^(x>>c)^rotl(x,b))

#define Tr1(x) Tr(x,7U,21U,26U)
#define Tr2(x) Tr(x,10U,19U,30U)

#define R(x) (work[x] = Wr(work[x-2],15U,13U,10U) + work[x-7] + Wr(work[x-15],25U,14U,3U) + work[x-16])
#define sharound(a,b,c,d,e,f,g,h,x,K) h+=Tr1(e)+Ch(e,f,g)+K+x; d+=h; h+=Tr2(a)+Ma(a,b,c);
#define sharound_s(a,b,c,d,e,f,g,h,x) h+=Tr1(e)+Ch(e,f,g)+x; d+=h; h+=Tr2(a)+Ma(a,b,c);

void Sha256_round_L(uint8* s, __local uchar* data)
{
	uint work[64];

	__local uint* udata = (__local uint*)data;

	uint A = (*s).s0;
	uint B = (*s).s1;
	uint C = (*s).s2;
	uint D = (*s).s3;
	uint E = (*s).s4;
	uint F = (*s).s5;
	uint G = (*s).s6;
	uint H = (*s).s7;
#pragma unroll
	for(uint i=0; i<16; i+=8)
	{
		work[i+0] = EndianSwap(udata[i+0]);
		sharound(A,B,C,D,E,F,G,H,work[i+0],K[i+0]);
		work[i+1] = EndianSwap(udata[i+1]);
		sharound(H,A,B,C,D,E,F,G,work[i+1],K[i+1]);
		work[i+2] = EndianSwap(udata[i+2]);
		sharound(G,H,A,B,C,D,E,F,work[i+2],K[i+2]);
		work[i+3] = EndianSwap(udata[i+3]);
		sharound(F,G,H,A,B,C,D,E,work[i+3],K[i+3]);
		work[i+4] = EndianSwap(udata[i+4]);
		sharound(E,F,G,H,A,B,C,D,work[i+4],K[i+4]);
		work[i+5] = EndianSwap(udata[i+5]);
		sharound(D,E,F,G,H,A,B,C,work[i+5],K[i+5]);
		work[i+6] = EndianSwap(udata[i+6]);
		sharound(C,D,E,F,G,H,A,B,work[i+6],K[i+6]);
		work[i+7] = EndianSwap(udata[i+7]);
		sharound(B,C,D,E,F,G,H,A,work[i+7],K[i+7]);
	}
#pragma unroll
	for(uint i=16; i<64; i+=8)
	{
		sharound(A,B,C,D,E,F,G,H,R(i+0),K[i+0]);
		sharound(H,A,B,C,D,E,F,G,R(i+1),K[i+1]);
		sharound(G,H,A,B,C,D,E,F,R(i+2),K[i+2]);
		sharound(F,G,H,A,B,C,D,E,R(i+3),K[i+3]);
		sharound(E,F,G,H,A,B,C,D,R(i+4),K[i+4]);
		sharound(D,E,F,G,H,A,B,C,R(i+5),K[i+5]);
		sharound(C,D,E,F,G,H,A,B,R(i+6),K[i+6]);
		sharound(B,C,D,E,F,G,H,A,R(i+7),K[i+7]);
	}
	(*s) += (uint8)(A,B,C,D,E,F,G,H);
}
__constant uint P[61] =
{
	0xc28a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19c0174,
	0x649b69c1, 0xf9be478a, 0x0fe1edc6, 0x240ca60c, 0x4fe9346f, 0x4d1c84ab, 0x61b94f1e, 0xf6f993db,
	0xe8465162, 0xad13066f, 0xb0214c0d, 0x695a0283, 0xa0323379, 0x2bd376e9, 0xe1d0537c, 0x03a244a0,
	0xfc13a4a5, 0xfafda43e, 0x56bea8bb, 0x445ec9b6, 0x39907315, 0x8c0d4e9f, 0xc832dccc, 0xdaffb65b,
	0x1fed4f61, 0x2f646808, 0x1ff32294, 0x2634ccd7, 0xb0ebdefa, 0xd6fc592b, 0xa63c5c8f, 0xbe9fbab9,
	0x0158082c, 0x68969712, 0x51e1d7e1, 0x5cf12d0d, 0xc4be2155, 0x7d7c8a34, 0x611f2c60, 0x036324af,
	0xa4f08d87, 0x9e3e8435, 0x2c6dae30, 0x11921afc, 0xb76d720e
};

void Sha256_round_padding(uint8* s)
{
	uint tmp = (*s).s7;
#define A (*s).s0
#define B (*s).s1
#define C (*s).s2
#define D (*s).s3
#define E (*s).s4
#define F (*s).s5
#define G (*s).s6
#define H (*s).s7
	sharound_s(A,B,C,D,E,F,G,H,P[ 0]);
	sharound_s(H,A,B,C,D,E,F,G,P[ 1]);
	sharound_s(G,H,A,B,C,D,E,F,P[ 2]);
	sharound_s(F,G,H,A,B,C,D,E,P[ 3]);
	sharound_s(E,F,G,H,A,B,C,D,P[ 4]);
	sharound_s(D,E,F,G,H,A,B,C,P[ 5]);
	sharound_s(C,D,E,F,G,H,A,B,P[ 6]);
	sharound_s(B,C,D,E,F,G,H,A,P[ 7]);
	sharound_s(A,B,C,D,E,F,G,H,P[ 8]);
	sharound_s(H,A,B,C,D,E,F,G,P[ 9]);
	sharound_s(G,H,A,B,C,D,E,F,P[10]);
	sharound_s(F,G,H,A,B,C,D,E,P[11]);
	sharound_s(E,F,G,H,A,B,C,D,P[12]);
	sharound_s(D,E,F,G,H,A,B,C,P[13]);
	sharound_s(C,D,E,F,G,H,A,B,P[14]);
	sharound_s(B,C,D,E,F,G,H,A,P[15]);
	sharound_s(A,B,C,D,E,F,G,H,P[16]);
	sharound_s(H,A,B,C,D,E,F,G,P[17]);
	sharound_s(G,H,A,B,C,D,E,F,P[18]);
	sharound_s(F,G,H,A,B,C,D,E,P[19]);
	sharound_s(E,F,G,H,A,B,C,D,P[20]);
	sharound_s(D,E,F,G,H,A,B,C,P[21]);
	sharound_s(C,D,E,F,G,H,A,B,P[22]);
	sharound_s(B,C,D,E,F,G,H,A,P[23]);
	sharound_s(A,B,C,D,E,F,G,H,P[24]);
	sharound_s(H,A,B,C,D,E,F,G,P[25]);
	sharound_s(G,H,A,B,C,D,E,F,P[26]);
	sharound_s(F,G,H,A,B,C,D,E,P[27]);
	sharound_s(E,F,G,H,A,B,C,D,P[28]);
	sharound_s(D,E,F,G,H,A,B,C,P[29]);
	sharound_s(C,D,E,F,G,H,A,B,P[30]);
	sharound_s(B,C,D,E,F,G,H,A,P[31]);
	sharound_s(A,B,C,D,E,F,G,H,P[32]);
	sharound_s(H,A,B,C,D,E,F,G,P[33]);
	sharound_s(G,H,A,B,C,D,E,F,P[34]);
	sharound_s(F,G,H,A,B,C,D,E,P[35]);
	sharound_s(E,F,G,H,A,B,C,D,P[36]);
	sharound_s(D,E,F,G,H,A,B,C,P[37]);
	sharound_s(C,D,E,F,G,H,A,B,P[38]);
	sharound_s(B,C,D,E,F,G,H,A,P[39]);
	sharound_s(A,B,C,D,E,F,G,H,P[40]);
	sharound_s(H,A,B,C,D,E,F,G,P[41]);
	sharound_s(G,H,A,B,C,D,E,F,P[42]);
	sharound_s(F,G,H,A,B,C,D,E,P[43]);
	sharound_s(E,F,G,H,A,B,C,D,P[44]);
	sharound_s(D,E,F,G,H,A,B,C,P[45]);
	sharound_s(C,D,E,F,G,H,A,B,P[46]);
	sharound_s(B,C,D,E,F,G,H,A,P[47]);
	sharound_s(A,B,C,D,E,F,G,H,P[48]);
	sharound_s(H,A,B,C,D,E,F,G,P[49]);
	sharound_s(G,H,A,B,C,D,E,F,P[50]);
	sharound_s(F,G,H,A,B,C,D,E,P[51]);
	sharound_s(E,F,G,H,A,B,C,D,P[52]);
	sharound_s(D,E,F,G,H,A,B,C,P[53]);
	sharound_s(C,D,E,F,G,H,A,B,P[54]);
	sharound_s(B,C,D,E,F,G,H,A,P[55]);
	sharound_s(A,B,C,D,E,F,G,H,P[56]);
	sharound_s(H,A,B,C,D,E,F,G,P[57]);
	sharound_s(G,H,A,B,C,D,E,F,P[58]);
	sharound_s(F,G,H,A,B,C,D,E,P[59]);
	sharound_s(E,F,G,H,A,B,C,D,P[60]);
	H += tmp;
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
}

#define ROT(x,n) rotate(x,n)

#define G2(arr,val,a,b,c,d,e,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,i)					\
  val[a] += (arr[sigma[i+e]] ^ cst[sigma[i+e+1]]) + val[b];	\
  val[j] += (arr[sigma[i+n]] ^ cst[sigma[i+n+1]]) + val[k];	\
  val[o] += (arr[sigma[i+s]] ^ cst[sigma[i+s+1]]) + val[p];	\
  val[t] += (arr[sigma[i+x]] ^ cst[sigma[i+x+1]]) + val[u];	\
  val[d] = ROT( val[d] ^ val[a],32UL);				\
  val[m] = ROT( val[m] ^ val[j],32UL);				\
  val[r] = ROT( val[r] ^ val[o],32UL);				\
  val[w] = ROT( val[w] ^ val[t],32UL);				\
  val[c] += val[d];						\
  val[l] += val[m];						\
  val[q] += val[r];						\
  val[v] += val[w];						\
  val[b] = ROT( val[b] ^ val[c],39UL);				\
  val[k] = ROT( val[k] ^ val[l],39UL);				\
  val[p] = ROT( val[p] ^ val[q],39UL);				\
  val[u] = ROT( val[u] ^ val[v],39UL);				\
  val[a] += (arr[sigma[i+e+1]] ^ cst[sigma[i+e]])+val[b];	\
  val[j] += (arr[sigma[i+n+1]] ^ cst[sigma[i+n]])+val[k];	\
  val[o] += (arr[sigma[i+s+1]] ^ cst[sigma[i+s]])+val[p];	\
  val[t] += (arr[sigma[i+x+1]] ^ cst[sigma[i+x]])+val[u];	\
  val[d] = ROT( val[d] ^ val[a],48UL);				\
  val[m] = ROT( val[m] ^ val[j],48UL);				\
  val[r] = ROT( val[r] ^ val[o],48UL);				\
  val[w] = ROT( val[w] ^ val[t],48UL);				\
  val[c] += val[d];						\
  val[l] += val[m];						\
  val[q] += val[r];						\
  val[v] += val[w];						\
  val[b] = ROT( val[b] ^ val[c],53UL);				\
  val[k] = ROT( val[k] ^ val[l],53UL);				\
  val[p] = ROT( val[p] ^ val[q],53UL);				\
  val[u] = ROT( val[u] ^ val[v],53UL);

__constant ulong m2[16] = {1UL << 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0x400};

//256
uint remu320(ulong x) 
{
	uint n = (uint)(((x>>38UL) + ((x>>6)&0xFFFFFFFF)) >> 32) + (uint)((x>>38UL) + ((x>>6)&0xFFFFFFFF));
	return ((n%5)<<6)+(x&63U);
}
//96
uint remu160(ulong x) 
{
	uint n = (uint)(((x>>37UL) + ((x>>5)&0xFFFFFFFF)) >> 32) + (uint)((x>>37UL) + ((x>>5)&0xFFFFFFFF));
	return ((n%5)<<5)+(x&31U);
}
//208
uint remu316(ulong x)
{
	x = (((x >> 32)*208 + (uint)x) >> 32)*208 + (uint)((x >> 32)*208 + (uint)x);
	return ((uint)x + mul24((uint)(x>>32),208U))%316;
}

__constant ulong blakeconsts[16] = 
{
	0x6A09E667F3BCC908UL,
	0xBB67AE8584CAA73BUL,
	0x3C6EF372FE94F82BUL,
	0xA54FF53A5F1D36F1UL,
	0x510E527FADE682D1UL,
	0x9B05688C2B3E6C1FUL,
	0x1F83D9ABFB41BD6BUL,
	0x5BE0CD19137E2179UL,
};

#define PAD_MASK 0x3FFFFF

#ifdef BFI_INT
	#pragma OPENCL EXTENSION cl_amd_media_ops : enable
	#define READ_W32(offset) (amd_bytealign(work3_ptr[((offset)>>2)+1],work3_ptr[(offset)>>2],offset)&PAD_MASK)
	#define PAD32_ADD(x)       index = (x); qCount += amd_bytealign(pad8to32[(index>>2)+1],pad8to32[index>>2],index);
	#define PAD32_ADD_AND(x,y) index = (x); qCount += amd_bytealign(pad8to32[(index>>2)+1],pad8to32[index>>2],index)&(y);
	#define PAD8(x) ((uchar)amd_bytealign(0,pad8to32[((x)>>2)],x))
#else
	#define READ_W32(offset) (((work3_ptr[(offset)>>2]>>(((offset)&3)*8))+(uint)((ulong)work3_ptr[((offset)>>2)+1]<<(32-((offset)&3)*8)))&PAD_MASK)
	#define PAD32_ADD(x)       index = (x); qCount += (pad8to32[index>>2]>>((index&3)*8))+(uint)((ulong)pad8to32[(index>>2)+1]<<(32-(index&3)*8));
	#define PAD32_ADD_AND(x,y) index = (x); qCount += ((pad8to32[index>>2]>>((index&3)*8))+(uint)((ulong)pad8to32[(index>>2)+1]<<(32-(index&3)*8)))&(y);
	#define PAD8(x) ((uchar)(pad8to32[x>>2]>>((x&3)<<3)))
#endif
__kernel
__attribute__((reqd_work_group_size(WORKSIZE, 1, 1)))
void search(__global uint const*restrict in_param, __global uint* out_param, __global uint const*restrict pad8to32, uint8 shastate) 
{
	__local uchar in_all[32768];
	uint in1start = 460*64;
	uint in2start = 460*get_local_id(0);
	__local uchar* in1 = in_all+in1start;
	__local uchar* in2 = in_all+in2start;
	__local uchar* lookup1 = in_all+31744;
	uint localid = get_local_id(0);
	{
		uint i=localid;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i); i+=WORKSIZE;
		lookup1[i] = PAD8(i*i);
	}
	
	((__local uint*)in1)[localid&15] = in_param[localid&15];
#pragma unroll
	for(uint i=0; i<16; ++i)
		((__local uint*)in2)[i] = in_param[i+16];

	uint nonce = get_global_id(0);

	*(__local uint*)(in2+44) = nonce;

	ulong v[32];
	v[0]=0x6A09E667F3BCC908UL;
	v[1]=0xBB67AE8584CAA73BUL;
	v[2]=0x3C6EF372FE94F82BUL;
	v[3]=0xA54FF53A5F1D36F1UL;
	v[4]=0x510E527FADE682D1UL;
	v[5]=0x9B05688C2B3E6C1FUL;
	v[6]=0x1F83D9ABFB41BD6BUL;
	v[7]=0x5BE0CD19137E2179UL;
	v[ 8] = 0x243F6A8885A308D3UL;
	v[ 9] = 0x13198A2E03707344UL;
	v[10] = 0xA4093822299F31D0UL;
	v[11] = 0x082EFA98EC4E6C89UL;
	v[12] = 0x452821E638D01777UL;
	v[13] = 0xBE5466CF34E9086CUL;
	v[14] = 0xC0AC29B7C97C50DDUL;
	v[15] = 0x3F84D5B5B5470917UL;

#pragma unroll
	for(uint i=0; i<8;++i)  v[i+16] = U8TO64(in1 + i*8);
#pragma unroll
	for(uint i=0; i<8;++i)  v[i+24] = U8TO64(in2 + i*8);
	ulong* m = v+16;
#pragma unroll
	for(uint i=0; i<256; i+=16)
	{
		G2( m, v, 0, 4, 8,12, 0, 1, 5, 9,13, 2, 2, 6,10,14, 4, 3, 7,11,15, 6, i);
		G2( m, v, 3, 4, 9,14,14, 2, 7, 8,13,12, 0, 5,10,15, 8, 1, 6,11,12,10, i);
	}
#pragma unroll
	for(uint i=0; i<8;++i)
	{
		v[i] ^= v[i+8]^blakeconsts[i]; 
		m[i] = v[i];
	}
	v[8] = 0x243F6A8885A308D3UL;
	v[9] = 0x13198A2E03707344UL;
	v[10] = 0xA4093822299F31D0UL;
	v[11] = 0x082EFA98EC4E6C89UL;
	v[12] = 0x452821E638D01377UL;
	v[13] = 0xBE5466CF34E90C6CUL;
	v[14] = 0xC0AC29B7C97C50DDUL;
	v[15] = 0x3F84D5B5B5470917UL;

#pragma unroll
	for(uint i=0; i<256; i+=16)
	{
		G2( m2, v, 0, 4, 8,12, 0, 1, 5, 9,13, 2, 2, 6,10,14, 4, 3, 7,11,15, 6, i);
		G2( m2, v, 3, 4, 9,14,14, 2, 7, 8,13,12, 0, 5,10,15, 8, 1, 6,11,12,10, i);
	}

	__local uchar* work2 = in2+64;

#pragma unroll
	for(uint i=0; i<8;++i)
	{
		m[i] ^= v[i]^v[i+8];
		U64TO8( work2+i*8, m[i]);
	}
	
	__local uint* work3_ptr = (__local uint*)(in2+128);

	uint value = work2[15]^work2[1];
	uint inindex;
	uint index = (1+value); inindex = select(in1start,in2start,index&0x40);
	value += (uint)(select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x80))^work2[2])<<8;
	index = (2+(value>>8)); inindex = select(in1start,in2start,index&0x40);
	value += (uint)(select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x8000))^work2[3])<<16;
	index = (3+(value>>16)); inindex = select(in1start,in2start,index&0x40);
	value += (uint)(select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x800000))^work2[4])<<24;
	work3_ptr[0] = value;
#pragma unroll
	for(uint x=4;x<320;x+=4)
	{
		index = (x+(value>>24)); inindex = select(in1start,in2start,index&0x40);
		value = select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x80000000))^work2[((x+1)&0x3F)];
		index = (x+1+value); inindex = select(in1start,in2start,index&0x40);
		value += (uint)(select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x00000080))^work2[((x+2)&0x3F)]) << 8;
		index = (x+2+(value>>8)); inindex = select(in1start,in2start,index&0x40);
		value += (uint)(select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x00008000))^work2[((x+3)&0x3F)]) << 16;
		index = (x+3+(value>>16)); inindex = select(in1start,in2start,index&0x40);
		value += (uint)(select(work2[index&0x3F],in_all[inindex+(index&0x3F)],(uchar)(bool)(value&0x00800000))^work2[((x+4)&0x3F)]) << 24;
		work3_ptr[x>>2] = value;
	}

	ulong qCount = (work3_ptr[77]>>16);
	qCount += (((ulong)work3_ptr[78])<<16)+(((ulong)work3_ptr[79])<<48);

	index = (qCount+(work3_ptr[75]&0xFF))&PAD_MASK;
	uint nExtra=(PAD8(index)>>3)+512;
	uint x=1;

	work3_ptr[79] ^=* work2*0x1000000;
	__local uchar* work3 = in2+128;
	do
	{
		PAD32_ADD((uint)qCount&PAD_MASK);
		++work3[select(320U,remu320(qCount),((uint)qCount)&0x87878700)];
		index = ((uint)qCount+work3[remu160(qCount)])&PAD_MASK;
		uint qCof = qCount&0x80000000;
		qCount -= PAD8(index);
		PAD32_ADD_AND(qCount&select(0x20FAFBU,0xFFFFU,qCof),select(0xFFFFFFFFU,0xFFU,qCof));
		PAD32_ADD(((uint)qCount+work3[remu160(qCount)])&PAD_MASK);
		++work3[select(320U,remu320(qCount),((uint)qCount)&0xF0000000)];
		PAD32_ADD(READ_W32((uchar)qCount));
		work3[x%320]=(uint)(qCount)^work2[x&63];
		PAD32_ADD(((uint)(qCount>>32)+work3[x%200])&PAD_MASK);
		__local uint* qCough = (__local uint*)(work3+(remu316(qCount)&0x1FC));
#ifdef BFI_INT
		qCof = ((uint)(qCount>>32))&0x00FFFFFFU;
		qCough[0] ^= amd_bytealign(qCof,((uint)qCount)&0xFF000000U,~(uint)(qCount));
		qCough[1] ^= amd_bytealign(0U,qCof,~(uint)(qCount));
#else
		qCough[0] ^= ((qCount>>24)&0xFFFFFFFFUL) << ((qCount&3)*8);
		qCough[1] ^= ((qCount>>24)&0xFFFFFFFFUL) >> (32-(qCount&3)*8);
#endif
		x += select(0U,1U,((uint)qCount&7) == 3);
		qCount -= lookup1[x];
		x += select(1U,2U,((uint)qCount&7) == 1);
	}while(x<nExtra);

	Sha256_round_L(&shastate, in2);
	Sha256_round_L(&shastate, in2+64);
	Sha256_round_L(&shastate, in2+128);
	Sha256_round_L(&shastate, in2+192);
	Sha256_round_L(&shastate, in2+256);
	Sha256_round_L(&shastate, in2+320);
	Sha256_round_L(&shastate, in2+384);
	Sha256_round_padding(&shastate);

	if ((shastate.s7 & SHAREMASK) == 0)
	{
		out_param[nonce&0xFF] = nonce;
	}
}
