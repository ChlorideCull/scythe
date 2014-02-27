typedef uint uint32_t;
typedef ulong uint64_t;
typedef uchar uint8_t;

typedef uint uint32;
typedef ulong uint64;

#define U8TO32(p) \
  (((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) | \
   ((uint32_t)((p)[2]) <<  8) | ((uint32_t)((p)[3])      ))
#define U8TO64(p) \
  (((uint64_t)U8TO32(p) << 32) | (uint64_t)U8TO32((p) + 4))
#define U32TO8(p, v) \
    (p)[0] = (uint8_t)((v) >> 24); (p)[1] = (uint8_t)((v) >> 16); \
    (p)[2] = (uint8_t)((v) >>  8); (p)[3] = (uint8_t)((v)      ); 
#define U64TO8(p, v) \
    U32TO8((p),     (uint32_t)((v) >> 32));	\
    U32TO8((p) + 4, (uint32_t)((v)      )); 

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

/*typedef struct  { 
  uint64_t h[8];
  uint8_t buf[128];
} state;*/

__constant uint8_t sigma[256] = 
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

__constant uint64_t cst[16] = 
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

uint rotl(uint x, uint y)
{
	return (x<<y)|(x>>(32-y));
}

#define Ch(x, y, z) (z ^ (x & (y ^ z)))
#define Ma(x, y, z) ((y & z) | (x & (y | z)))

#define Tr(x,a,b,c) (rotl(x,a)^rotl(x,b)^rotl(x,c))

#define R(x) (work[x] = (rotl(work[x-2],15)^rotl(work[x-2],13)^((work[x-2])>>10)) + work[x-7] + (rotl(work[x-15],25)^rotl(work[x-15],14)^((work[x-15])>>3)) + work[x-16])
#define sharound(a,b,c,d,e,f,g,h,x,K) h+=Tr(e,7,21,26)+Ch(e,f,g)+K+x; d+=h; h+=Tr(a,10,19,30)+Ma(a,b,c);
#define sharound_s(a,b,c,d,e,f,g,h,x) h+=Tr(e,7,21,26)+Ch(e,f,g)+x; d+=h; h+=Tr(a,10,19,30)+Ma(a,b,c);

uint EndianSwap(uint n)
{
	return ((n&0xFF)<<24) | ((n&0xFF00)<<8) | ((n&0xFF0000)>>8) | ((n&0xFF000000)>>24);
}

void Sha256_round(uint* s, unsigned char* data)
{
	uint work[64];

	uint* udata = (uint*)data;
	for(uint i=0; i<16; ++i)
	{
		work[i] = EndianSwap(udata[i]);
	}

	uint A = s[0];
	uint B = s[1];
	uint C = s[2];
	uint D = s[3];
	uint E = s[4];
	uint F = s[5];
	uint G = s[6];
	uint H = s[7];
	sharound(A,B,C,D,E,F,G,H,work[0],K[0]);
	sharound(H,A,B,C,D,E,F,G,work[1],K[1]);
	sharound(G,H,A,B,C,D,E,F,work[2],K[2]);
	sharound(F,G,H,A,B,C,D,E,work[3],K[3]);
	sharound(E,F,G,H,A,B,C,D,work[4],K[4]);
	sharound(D,E,F,G,H,A,B,C,work[5],K[5]);
	sharound(C,D,E,F,G,H,A,B,work[6],K[6]);
	sharound(B,C,D,E,F,G,H,A,work[7],K[7]);
	sharound(A,B,C,D,E,F,G,H,work[8],K[8]);
	sharound(H,A,B,C,D,E,F,G,work[9],K[9]);
	sharound(G,H,A,B,C,D,E,F,work[10],K[10]);
	sharound(F,G,H,A,B,C,D,E,work[11],K[11]);
	sharound(E,F,G,H,A,B,C,D,work[12],K[12]);
	sharound(D,E,F,G,H,A,B,C,work[13],K[13]);
	sharound(C,D,E,F,G,H,A,B,work[14],K[14]);
	sharound(B,C,D,E,F,G,H,A,work[15],K[15]);
	sharound(A,B,C,D,E,F,G,H,R(16),K[16]);
	sharound(H,A,B,C,D,E,F,G,R(17),K[17]);
	sharound(G,H,A,B,C,D,E,F,R(18),K[18]);
	sharound(F,G,H,A,B,C,D,E,R(19),K[19]);
	sharound(E,F,G,H,A,B,C,D,R(20),K[20]);
	sharound(D,E,F,G,H,A,B,C,R(21),K[21]);
	sharound(C,D,E,F,G,H,A,B,R(22),K[22]);
	sharound(B,C,D,E,F,G,H,A,R(23),K[23]);
	sharound(A,B,C,D,E,F,G,H,R(24),K[24]);
	sharound(H,A,B,C,D,E,F,G,R(25),K[25]);
	sharound(G,H,A,B,C,D,E,F,R(26),K[26]);
	sharound(F,G,H,A,B,C,D,E,R(27),K[27]);
	sharound(E,F,G,H,A,B,C,D,R(28),K[28]);
	sharound(D,E,F,G,H,A,B,C,R(29),K[29]);
	sharound(C,D,E,F,G,H,A,B,R(30),K[30]);
	sharound(B,C,D,E,F,G,H,A,R(31),K[31]);
	sharound(A,B,C,D,E,F,G,H,R(32),K[32]);
	sharound(H,A,B,C,D,E,F,G,R(33),K[33]);
	sharound(G,H,A,B,C,D,E,F,R(34),K[34]);
	sharound(F,G,H,A,B,C,D,E,R(35),K[35]);
	sharound(E,F,G,H,A,B,C,D,R(36),K[36]);
	sharound(D,E,F,G,H,A,B,C,R(37),K[37]);
	sharound(C,D,E,F,G,H,A,B,R(38),K[38]);
	sharound(B,C,D,E,F,G,H,A,R(39),K[39]);
	sharound(A,B,C,D,E,F,G,H,R(40),K[40]);
	sharound(H,A,B,C,D,E,F,G,R(41),K[41]);
	sharound(G,H,A,B,C,D,E,F,R(42),K[42]);
	sharound(F,G,H,A,B,C,D,E,R(43),K[43]);
	sharound(E,F,G,H,A,B,C,D,R(44),K[44]);
	sharound(D,E,F,G,H,A,B,C,R(45),K[45]);
	sharound(C,D,E,F,G,H,A,B,R(46),K[46]);
	sharound(B,C,D,E,F,G,H,A,R(47),K[47]);
	sharound(A,B,C,D,E,F,G,H,R(48),K[48]);
	sharound(H,A,B,C,D,E,F,G,R(49),K[49]);
	sharound(G,H,A,B,C,D,E,F,R(50),K[50]);
	sharound(F,G,H,A,B,C,D,E,R(51),K[51]);
	sharound(E,F,G,H,A,B,C,D,R(52),K[52]);
	sharound(D,E,F,G,H,A,B,C,R(53),K[53]);
	sharound(C,D,E,F,G,H,A,B,R(54),K[54]);
	sharound(B,C,D,E,F,G,H,A,R(55),K[55]);
	sharound(A,B,C,D,E,F,G,H,R(56),K[56]);
	sharound(H,A,B,C,D,E,F,G,R(57),K[57]);
	sharound(G,H,A,B,C,D,E,F,R(58),K[58]);
	sharound(F,G,H,A,B,C,D,E,R(59),K[59]);
	sharound(E,F,G,H,A,B,C,D,R(60),K[60]);
	sharound(D,E,F,G,H,A,B,C,R(61),K[61]);
	sharound(C,D,E,F,G,H,A,B,R(62),K[62]);
	sharound(B,C,D,E,F,G,H,A,R(63),K[63]);

	s[0] += A;
	s[1] += B;
	s[2] +=	C;
	s[3] += D;
	s[4] += E;
	s[5] += F;
	s[6] += G;
	s[7] += H;
}

__constant uint P[64] =
{
	0xc28a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19c0174,
	0x649b69c1, 0xf9be478a, 0x0fe1edc6, 0x240ca60c, 0x4fe9346f, 0x4d1c84ab, 0x61b94f1e, 0xf6f993db,
	0xe8465162, 0xad13066f, 0xb0214c0d, 0x695a0283, 0xa0323379, 0x2bd376e9, 0xe1d0537c, 0x03a244a0,
	0xfc13a4a5, 0xfafda43e, 0x56bea8bb, 0x445ec9b6, 0x39907315, 0x8c0d4e9f, 0xc832dccc, 0xdaffb65b,
	0x1fed4f61, 0x2f646808, 0x1ff32294, 0x2634ccd7, 0xb0ebdefa, 0xd6fc592b, 0xa63c5c8f, 0xbe9fbab9,
	0x0158082c, 0x68969712, 0x51e1d7e1, 0x5cf12d0d, 0xc4be2155, 0x7d7c8a34, 0x611f2c60, 0x036324af,
	0xa4f08d87, 0x9e3e8435, 0x2c6dae30, 0x11921afc, 0xb76d720e, 0x245f3661, 0xc3a65ecb, 0x43b9e908
};

void Sha256_round_padding(uint* s)
{
	uint A = s[0];
	uint B = s[1];
	uint C = s[2];
	uint D = s[3];
	uint E = s[4];
	uint F = s[5];
	uint G = s[6];
	uint H = s[7];
	sharound_s(A,B,C,D,E,F,G,H,P[0]);
	sharound_s(H,A,B,C,D,E,F,G,P[1]);
	sharound_s(G,H,A,B,C,D,E,F,P[2]);
	sharound_s(F,G,H,A,B,C,D,E,P[3]);
	sharound_s(E,F,G,H,A,B,C,D,P[4]);
	sharound_s(D,E,F,G,H,A,B,C,P[5]);
	sharound_s(C,D,E,F,G,H,A,B,P[6]);
	sharound_s(B,C,D,E,F,G,H,A,P[7]);
	sharound_s(A,B,C,D,E,F,G,H,P[8]);
	sharound_s(H,A,B,C,D,E,F,G,P[9]);
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

	s[7] += H;
}



#define ROT(x,n) (((x)<<(64-n))|( (x)>>(n)))

#define G(m,a,b,c,d,e,i)					\
  v[a] += (m[sigma[i+e]] ^ cst[sigma[i+e+1]]) + v[b];	\
  v[d] = ROT( v[d] ^ v[a],32);				\
  v[c] += v[d];						\
  v[b] = ROT( v[b] ^ v[c],25);				\
  v[a] += (m[sigma[i+e+1]] ^ cst[sigma[i+e]])+v[b];	\
  v[d] = ROT( v[d] ^ v[a],16);				\
  v[c] += v[d];						\
  v[b] = ROT( v[b] ^ v[c],11);				

//assumes input is 512 bytes
__kernel void search(__global uint8_t* in_param, __global uint* out_param, __global uint8_t* pad) 
{
	uchar in[512];
	for(uint i=0; i<128; ++i)
	in[i] = in_param[i];

	uint nonce = get_global_id(0);
	
	*(uint*)(in+108) = nonce;

	uint64_t h[8];
	h[0]=0x6A09E667F3BCC908UL;
	h[1]=0xBB67AE8584CAA73BUL;
	h[2]=0x3C6EF372FE94F82BUL;
	h[3]=0xA54FF53A5F1D36F1UL;
	h[4]=0x510E527FADE682D1UL;
	h[5]=0x9B05688C2B3E6C1FUL;
	h[6]=0x1F83D9ABFB41BD6BUL;
	h[7]=0x5BE0CD19137E2179UL;

	uint64_t v[16];
	for(uint i=0; i< 8;++i)  v[i] = h[i];
	v[ 8] = 0x243F6A8885A308D3UL;
	v[ 9] = 0x13198A2E03707344UL;
	v[10] = 0xA4093822299F31D0UL;
	v[11] = 0x082EFA98EC4E6C89UL;
	v[12] = 0x452821E638D01777UL;
	v[13] = 0xBE5466CF34E9086CUL;
	v[14] = 0xC0AC29B7C97C50DDUL;
	v[15] = 0x3F84D5B5B5470917UL;

	{
		uint64_t m[16];
		for(uint i=0; i<16;++i)  m[i] = U8TO64(in + i*8);
		uint i=0;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
		i+=16;
		G( m, 0, 4, 8,12, 0, i); G( m, 1, 5, 9,13, 2, i); G( m, 2, 6,10,14, 4, i); G( m, 3, 7,11,15, 6, i);
		G( m, 3, 4, 9,14,14, i); G( m, 2, 7, 8,13,12, i); G( m, 0, 5,10,15, 8, i); G( m, 1, 6,11,12,10, i);
	} 

	for(uint i=0; i<16;++i)  h[i&7] ^= v[i]; 

	for(uint i=0; i< 8;++i)  v[i] = h[i];
	v[8] = 0x243F6A8885A308D3UL;
	v[9] = 0x13198A2E03707344UL;
	v[10] = 0xA4093822299F31D0UL;
	v[11] = 0x082EFA98EC4E6C89UL;
	v[12] = 0x452821E638D01377UL;
	v[13] = 0xBE5466CF34E90C6CUL;
	v[14] = 0xC0AC29B7C97C50DDUL;
	v[15] = 0x3F84D5B5B5470917UL;

	{
		uint64_t m2[16] = {1UL << 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0x400};
		uint i=0;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
		i+=16;
		G(m2, 0, 4, 8,12, 0, i); G(m2, 1, 5, 9,13, 2, i); G(m2, 2, 6,10,14, 4, i); G(m2, 3, 7,11,15, 6, i);
		G(m2, 3, 4, 9,14,14, i); G(m2, 2, 7, 8,13,12, i); G(m2, 0, 5,10,15, 8, i); G(m2, 1, 6,11,12,10, i);
	} 

	for(uint i=0; i<16;++i)  h[i&7] ^= v[i];

	uint8_t* work2 = in+128;

	U64TO8( work2 + 0, h[0]);
	U64TO8( work2 + 8, h[1]);
	U64TO8( work2 +16, h[2]);
	U64TO8( work2 +24, h[3]);
	U64TO8( work2 +32, h[4]);
	U64TO8( work2 +40, h[5]);
	U64TO8( work2 +48, h[6]);
	U64TO8( work2 +56, h[7]);
	
	uint8_t* work3 = work2+64;
//a = x-1, b = x, c = x&63
#define WORKINIT(a,b,c)   work3[a] ^= work2[c]; \
		if(work3[a]&0x80) work3[b]=in[(b+work3[a])&0x7F]; \
		else              work3[b]=work2[(b+work3[a])&0x3F];

	
	work3[0] = work2[15];
	WORKINIT(0,1,1);
	WORKINIT(1,2,2);
	WORKINIT(2,3,3);
	for(int x=4;x<64;++x)
	{
		WORKINIT(x-1,x,x);
		++x;
		WORKINIT(x-1,x,x);
		++x;
		WORKINIT(x-1,x,x);
		++x;
		WORKINIT(x-1,x,x);
	}
	for(int x=64;x<320;++x)
	{
		WORKINIT(x-1,x,x&63);
		++x;
		WORKINIT(x-1,x,x&63);
		++x;
		WORKINIT(x-1,x,x&63);
		++x;
		WORKINIT(x-1,x,x&63);
	}

	#define READ_PAD32_R(offset) ((uint)pad[offset] | (((uint)pad[offset+1])<<8) | (((uint)pad[offset+2])<<16) | (((uint)pad[offset+3])<<24))
	
	#define READ_W32(offset) ((uint)work3[offset] + (((uint)work3[(offset)+1])<<8) + (((uint)work3[(offset)+2]&0x3F)<<16))

	ushort* shortptr = (ushort*)(work3+310);
	uint64 qCount = shortptr[0];
	qCount |= ((uint64)shortptr[3])<<48;
	uint* uintptr = (uint*)(work3+312);
	qCount |= ((uint64)*uintptr)<<16;

	uint nExtra=(pad[(qCount+work3[300])&0x3FFFFF]>>3)+512;
	for(uint x=1;x<nExtra;++x)
	{
		uint res = qCount&0x3FFFFF;
		qCount+= READ_PAD32_R(res);
		if(qCount&0x87878700)        ++work3[qCount%320];

		qCount-= pad[(qCount+work3[qCount%160])&0x3FFFFF];
		if(qCount&0x80000000)   { qCount+= pad[qCount&0xFFFF]; }
		else                    { res = qCount&0x20FAFB; qCount+= READ_PAD32_R(res); }

		res = (qCount+work3[qCount%160]) & 0x3FFFFF;
		qCount+= READ_PAD32_R(res);
		if(qCount&0xF0000000)        ++work3[qCount%320];

		res = READ_W32(qCount&0xFF);
		qCount+= READ_PAD32_R(res);
		work3[x%320]=work2[x&63]^(qCount&0xFF);

		res = ((qCount>>32)+work3[x%200]) & 0x3FFFFF;
		qCount+= READ_PAD32_R(res);
		
		//this is an ingenious optimization. gives +2.5% :-)
		if (qCount&3)
		{
			uchar* ram = work3+qCount%316;
			ram[0] ^= (qCount>>24)&0xFF;
			ram[1] ^= (qCount>>32)&0xFF;
			ram[2] ^= (qCount>>40)&0xFF;
			ram[3] ^= (qCount>>48)&0xFF;
			if((qCount&7)==3) ++x;
		}
		else
		{
			uint* ram = (uint*)(work3+qCount%316);
			*ram ^= qCount>>24;
		}

		qCount-= pad[x*x];
		if((qCount&0x07)==0x01) ++x;
	}

	uint s[8]= {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

	Sha256_round(s, in);
	Sha256_round(s, in+64);
	Sha256_round(s, in+128);
	Sha256_round(s, in+192);
	Sha256_round(s, in+256);
	Sha256_round(s, in+320);
	Sha256_round(s, in+384);
	Sha256_round(s, in+448);
	Sha256_round_padding(s);
	
	if ((s[7] & 0x80FFFF) == 0)
	{
		out_param[nonce&0xFF] = get_global_id(0);
	}
}
