#if defined(VECTORS)
typedef uint2 u;
#elif defined(VECTORS4)
typedef uint4 u;
#else
typedef uint u;
#endif

#ifdef BITALIGN
#pragma OPENCL EXTENSION cl_amd_media_ops : enable
#define rot(x, y) amd_bitalign(x, x, y)
#else
#define rot(x, y) rotate(x, (u)(32-y))
#endif

#ifdef BFI_INT
#pragma OPENCL EXTENSION cl_amd_media_ops : enable
#define Ch(x, y, z) amd_sad(x,y,z)
#define Ma(x, y, z) amd_sad(x^z,y,z)

#ifdef BFI_INT_FIX
#define Ch2(x, y, z) (z^(x&(y^z)))
#define Ma2(x, y, z) (z^((x^z)&(y^z)))
#else
#define Ch2(x, y, z) amd_sad(x,y,z)
#define Ma2(x, y, z) amd_sad(x^z,y,z)
#endif

#else
#define Ch(x, y, z) (z^(x&(y^z)))
#define Ma(x, y, z) (z^((x^z)&(y^z)))
#define Ch2(x, y, z) Ch(x,y,z)
#define Ma2(x, y, z) Ma(x,y,z)
#endif

#define Tr(x,a,b,c) (rot(x,a)^rot(x,b)^rot(x,c))
#define Tr1(x) Tr(x,25,11,6)
#define Tr2(x) Tr(x,22,13,2)

#define Wr(x,a,b,c) (rot(x,a)^(x>>c)^rot(x,b))
#define Wr1(x) Wr(x, 17,19,10)
#define Wr2(x) Wr(x, 7,18,3)

__kernel 
__attribute__((reqd_work_group_size(WORKSIZE, 1, 1)))
void search(	const uint state1, const uint state2, const uint state3,
				const uint state4, const uint state5, const uint state6, const uint state7,
				const uint b_start, const uint c_start, const uint d_start,
				const uint f_start, const uint g_start, const uint h_start,
				const uint W16, const uint W17, const uint W18_partial,
				const uint W31_partial, const uint W32_partial, const uint b_start_k6, const uint c_start_k5,
				const uint state0p,
				const uint W16p, const uint W17p,
				const u a_start, const u e_start, const u W19_partial,
				__global uint* output
				)
{
	u work[51];
	u A,B,C,D,E,F,G,H,tmp;
	
	//first 3 rounds already done, and most of round 4 too
	A=a_start+get_global_id(0);
	E=e_start+get_global_id(0);
	//that was the rest of round 4
#if defined(VECTORS)
	work[0] = Wr2(get_global_id(0));
	work[0].y ^= 0x11002000U;
#elif defined(VECTORS4)
	work[0] = W18_partial + (Wr2(get_global_id(0))^(u)(0,0x11002000U,0x08801000U,0x19803000U));
#else
	work[0] = W18_partial + Wr2(get_global_id(0));
#endif
	work[1] = W19_partial + get_global_id(0);

	tmp=d_start   +Tr1(A)+Ch(A,b_start,c_start); H=h_start+tmp; D = tmp+Tr2(E)+Ch((u)(f_start^g_start),E,g_start);
	tmp=c_start_k5+Tr1(H)+Ch(H,A,b_start);       G=g_start+tmp; C = tmp+Tr2(D)+Ma(D,E,f_start);
	tmp=b_start_k6+Tr1(G)+Ch(G,H,A);             F=f_start+tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	tmp=A+Tr1(F)+Ch(F,G,H)+0xAB1C5ED5U;         E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);
	tmp=H+Tr1(E)+Ch(E,F,G)+0xD807AA98U;         D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);
	tmp=G+Tr1(D)+Ch(D,E,F)+0x12835B01U;         C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);
	tmp=F+Tr1(C)+Ch(C,D,E)+0x243185BEU;         B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);
	tmp=E+Tr1(B)+Ch(B,C,D)+0x550C7DC3U;         A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);
	tmp=D+Tr1(A)+Ch(A,B,C)+0x72BE5D74U;         H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);
	tmp=C+Tr1(H)+Ch(H,A,B)+0x80DEB1FEU;         G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);
	tmp=B+Tr1(G)+Ch(G,H,A)+0x9BDC06A7U;         F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);
	tmp=A+Tr1(F)+Ch(F,G,H)+0xC19BF3F4U;         E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);
	tmp=H+Tr1(E)+Ch(E,F,G)+W16p;                D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);
	tmp=G+Tr1(D)+Ch(D,E,F)+W17p;                C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);
#if defined(VECTORS)
	tmp=F+Tr1(C)+Ch(C,D,E)+0x0FC19DC6U+(work[0]+=W18_partial); B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);
#else
	tmp=F+Tr1(C)+Ch(C,D,E)+0x0FC19DC6U+W18_partial; B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);
#endif
	tmp=E+Tr1(B)+Ch(B,C,D)+0x240CA1CCU+work[1]; A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);

#ifdef BITALIGN
	work[2] = (work[0]>>10)^rot(work[0],19)^amd_bitalign(work[0]^0x10000, work[0], 17);
#else
	work[2] = Wr1(work[0])+0x80000000;
#endif
	tmp=D+Tr1(A)+Ch(A,B,C)+0x2DE92C6FU+work[2]; H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);

	work[3] = Wr1(work[1]);
	tmp=C+Tr1(H)+Ch(H,A,B)+0x4A7484AAU+work[3]; G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);

	work[4] = Wr1(work[2]) + 0x00000280U;
	tmp=B+Tr1(G)+Ch(G,H,A)+0x5CB0A9DCU+work[4]; F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	work[5] = W16 + Wr1(work[3]);
	tmp=A+Tr1(F)+Ch(F,G,H)+0x76F988DAU+work[5]; E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);

	work[6] = W17 + Wr1(work[4]);
	tmp=H+Tr1(E)+Ch(E,F,G)+0x983E5152U+work[6]; D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);

	work[7] = work[0] + Wr1(work[5]);
	tmp=G+Tr1(D)+Ch(D,E,F)+0xA831C66DU+work[7]; C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);

	work[8] = work[1] + Wr1(work[6]);
	tmp=F+Tr1(C)+Ch(C,D,E)+0xB00327C8U+work[8]; B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);

	work[9] = work[2] + Wr1(work[7]);
	tmp=E+Tr1(B)+Ch(B,C,D)+0xBF597FC7U+work[9]; A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);

	work[10] = work[3] + Wr1(work[8]);
	tmp=D+Tr1(A)+Ch(A,B,C)+0xC6E00BF3U+work[10]; H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);

	work[11] = work[4] + Wr1(work[9]);
	tmp=C+Tr1(H)+Ch(H,A,B)+0xD5A79147U+work[11]; G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);

	work[12] = 0x00a00055U + work[5] + Wr1(work[10]);
	tmp=B+Tr1(G)+Ch(G,H,A)+0x06CA6351U+work[12]; F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	work[13] = W31_partial + work[6] + Wr1(work[11]);
	tmp=A+Tr1(F)+Ch(F,G,H)+0x14292967U+work[13]; E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);

	work[14] = W32_partial + work[7] + Wr1(work[12]);
	tmp=H+Tr1(E)+Ch(E,F,G)+0x27B70A85U+work[14]; D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);

	work[15] = work[8] + W17 + Wr1(work[13]) + Wr2(work[0]);
	tmp=G+Tr1(D)+Ch(D,E,F)+0x2E1B2138U+work[15]; C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);

	work[16] = work[9] + work[0] + Wr1(work[14]) + Wr2(work[1]);
	tmp=F+Tr1(C)+Ch(C,D,E)+0x4D2C6DFCU+work[16]; B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);

	work[17] = work[10] + work[1] + Wr1(work[15]) + Wr2(work[2]);
	tmp=E+Tr1(B)+Ch(B,C,D)+0x53380D13U+work[17]; A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);

	work[18] = work[11] + work[2] + Wr1(work[16]) + Wr2(work[3]);
	tmp=D+Tr1(A)+Ch(A,B,C)+0x650A7354U+work[18]; H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);

	work[19] = work[12] + work[3] + Wr1(work[17]) + Wr2(work[4]);
	tmp=C+Tr1(H)+Ch(H,A,B)+0x766A0ABBU+work[19]; G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);

	work[20] = work[13] + work[4] + Wr1(work[18]) + Wr2(work[5]);
	tmp=B+Tr1(G)+Ch(G,H,A)+0x81C2C92EU+work[20]; F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	work[21] = work[14] + work[5] + Wr1(work[19]) + Wr2(work[6]);
	tmp=A+Tr1(F)+Ch(F,G,H)+0x92722C85U+work[21]; E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);

	work[22] = work[15] + work[6] + Wr1(work[20]) + Wr2(work[7]);
	tmp=H+Tr1(E)+Ch(E,F,G)+0xA2BFE8A1U+work[22]; D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);

	work[23] = work[16] + work[7] + Wr1(work[21]) + Wr2(work[8]);
	tmp=G+Tr1(D)+Ch(D,E,F)+0xA81A664BU+work[23]; C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);

	work[24] = work[17] + work[8] + Wr1(work[22]) + Wr2(work[9]);
	tmp=F+Tr1(C)+Ch(C,D,E)+0xC24B8B70U+work[24]; B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);

	work[25] = work[18] + work[9] + Wr1(work[23]) + Wr2(work[10]);
	tmp=E+Tr1(B)+Ch(B,C,D)+0xC76C51A3U+work[25]; A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);

	work[26] = work[19] + work[10] + Wr1(work[24]) + Wr2(work[11]);
	tmp=D+Tr1(A)+Ch(A,B,C)+0xD192E819U+work[26]; H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);

	work[27] = work[20] + work[11] + Wr1(work[25]) + Wr2(work[12]);
	tmp=C+Tr1(H)+Ch(H,A,B)+0xD6990624U+work[27]; G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);

	work[28] = work[21] + work[12] + Wr1(work[26]) + Wr2(work[13]);
	tmp=B+Tr1(G)+Ch(G,H,A)+0xF40E3585U+work[28]; F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	work[29] = work[22] + work[13] + Wr1(work[27]) + Wr2(work[14]);
	tmp=A+Tr1(F)+Ch(F,G,H)+0x106AA070U+work[29]; E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);

	work[30] = work[23] + work[14] + Wr1(work[28]) + Wr2(work[15]);
	tmp=H+Tr1(E)+Ch(E,F,G)+0x19A4C116U+work[30]; D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);

	work[31] = work[24] + work[15] + Wr1(work[29]) + Wr2(work[16]);
	tmp=G+Tr1(D)+Ch(D,E,F)+0x1E376C08U+work[31]; C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);

	work[32] = work[25] + work[16] + Wr1(work[30]) + Wr2(work[17]);
	tmp=F+Tr1(C)+Ch(C,D,E)+0x2748774CU+work[32]; B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);

	work[33] = work[26] + work[17] + Wr1(work[31]) + Wr2(work[18]);
	tmp=E+Tr1(B)+Ch(B,C,D)+0x34B0BCB5U+work[33]; A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);

	work[34] = work[27] + work[18] + Wr1(work[32]) + Wr2(work[19]);
	tmp=D+Tr1(A)+Ch(A,B,C)+0x391C0CB3U+work[34]; H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);

	work[35] = work[28] + work[19] + Wr1(work[33]) + Wr2(work[20]);
	tmp=C+Tr1(H)+Ch(H,A,B)+0x4ED8AA4AU+work[35]; G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);

	work[36] = work[29] + work[20] + Wr1(work[34]) + Wr2(work[21]);
	tmp=B+Tr1(G)+Ch(G,H,A)+0x5B9CCA4FU+work[36]; F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	work[37] = work[30] + work[21] + Wr1(work[35]) + Wr2(work[22]);
	tmp=A+Tr1(F)+Ch(F,G,H)+0x682E6FF3U+work[37]; E += tmp; A = tmp+Tr2(B)+Ma(B,C,D);

	work[38] = work[31] + work[22] + Wr1(work[36]) + Wr2(work[23]);
	tmp=H+Tr1(E)+Ch(E,F,G)+0x748F82EEU+work[38]; D += tmp; H = tmp+Tr2(A)+Ma(A,B,C);

	work[39] = work[32] + work[23] + Wr1(work[37]) + Wr2(work[24]);
	tmp=G+Tr1(D)+Ch(D,E,F)+0x78A5636FU+work[39]; C += tmp; G = tmp+Tr2(H)+Ma(H,A,B);

	work[40] = work[33] + work[24] + Wr1(work[38]) + Wr2(work[25]);
	tmp=F+Tr1(C)+Ch(C,D,E)+0x84C87814U+work[40]; B += tmp; F = tmp+Tr2(G)+Ma(G,H,A);

	work[41] = work[34] + work[25] + Wr1(work[39]) + Wr2(work[26]);
	tmp=E+Tr1(B)+Ch(B,C,D)+0x8CC70208U+work[41]; A += tmp; E = tmp+Tr2(F)+Ma(F,G,H);

	work[42] = work[35] + work[26] + Wr1(work[40]) + Wr2(work[27]);
	tmp=D+Tr1(A)+Ch(A,B,C)+0x90BEFFFAU+work[42]; H += tmp; D = tmp+Tr2(E)+Ma(E,F,G);

	work[43] = work[36] + work[27] + Wr1(work[41]) + Wr2(work[28]);
	tmp=C+Tr1(H)+Ch(H,A,B)+0xA4506CEBU+work[43]; G += tmp; C = tmp+Tr2(D)+Ma(D,E,F);

	tmp=B+Tr1(G)+Ch(G,H,A)+0xBEF9A3F7U+work[37] + work[28] + Wr1(work[42]) + Wr2(work[29]); F += tmp; B = tmp+Tr2(C)+Ma(C,D,E);

	tmp=A+Tr1(F)+Ch(F,G,H)+work[38] + work[29] + Wr1(work[43]) + Wr2(work[30]); A = tmp+Tr2(B)+Ma(B,C,D)+state0p;

	work[0]=0x67381D5EU+A;
	tmp += E+state4;
	work[4]=tmp;
	E=A+0x6340A5ABU;

	B+=state1;
	work[1]=B;
	B+=0xcd2a11aeU+Tr1(A)+Ch2(A,0x510e527fU,0x9b05688cU);
	work[5]=state5+F;
	F=B+0xC3910C8EU+Tr2(E)+Ch2(E,0xfb6feee7U,0x2a01a605U);

	C+=state2;
	work[2]=C;
	C+=0x0C2E12E0U+Tr1(B)+Ch2(B,A,0x510e527fU);
	work[6]=state6+G;
	G=C+0x4498517BU+Tr2(F)+Ma2(F,E,0x6a09e667U);

	D+=state3;
	work[3]=D;
	D+=0xA4CE148BU+Tr1(C)+Ch(C,B,A); 
	work[7]=state7+H;
	H=D+0x95F61999U+Tr2(G)+Ma(G,F,E);

	tmp+=A+Tr1(D)+Ch(D,C,B)+0x3956C25BU; E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);
	tmp=B+Tr1(E)+Ch(E,D,C)+0x59F111F1U+work[5]; F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);
	tmp=C+Tr1(F)+Ch(F,E,D)+0x923F82A4U+work[6]; G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);
	tmp=D+Tr1(G)+Ch(G,F,E)+0xAB1C5ED5U+work[7]; H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);
	tmp=E+Tr1(H)+Ch(H,G,F)+0x5807AA98U;         A += tmp; E = tmp+Tr2(D)+Ma(D,C,B);
	tmp=F+Tr1(A)+Ch(A,H,G)+0x12835B01U;         B += tmp; F = tmp+Tr2(E)+Ma(E,D,C);
	tmp=G+Tr1(B)+Ch(B,A,H)+0x243185BEU;         C += tmp; G = tmp+Tr2(F)+Ma(F,E,D);
	tmp=H+Tr1(C)+Ch(C,B,A)+0x550C7DC3U;         D += tmp; H = tmp+Tr2(G)+Ma(G,F,E);
	tmp=A+Tr1(D)+Ch(D,C,B)+0x72BE5D74U;         E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);
	tmp=B+Tr1(E)+Ch(E,D,C)+0x80DEB1FEU;         F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);
	tmp=C+Tr1(F)+Ch(F,E,D)+0x9BDC06A7U;         G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);
	tmp=D+Tr1(G)+Ch(G,F,E)+0xC19BF274U;         H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);

	work[8] = work[0] + Wr2(work[1]);
	tmp=E+Tr1(H)+Ch(H,G,F)+0xE49B69C1U+work[8]; A += tmp; E = tmp+Tr2(D)+Ma(D,C,B);

	work[9] = work[1] +0x00a00000U +  Wr2(work[2]);
	tmp=F+Tr1(A)+Ch(A,H,G)+0xEFBE4786U+work[9]; B += tmp; F = tmp+Tr2(E)+Ma(E,D,C);

	work[10] = work[2] + Wr1(work[8]) + Wr2(work[3]);
	tmp=G+Tr1(B)+Ch(B,A,H)+0x0FC19DC6U+work[10]; C += tmp; G = tmp+Tr2(F)+Ma(F,E,D);

	work[11] = work[3] + Wr1(work[9]) + Wr2(work[4]);
	tmp=H+Tr1(C)+Ch(C,B,A)+0x240CA1CCU+work[11]; D += tmp; H = tmp+Tr2(G)+Ma(G,F,E);

	work[12] = work[4] + Wr1(work[10]) + Wr2(work[5]);
	tmp=A+Tr1(D)+Ch(D,C,B)+0x2DE92C6FU+work[12]; E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);

	work[13] = work[5] + Wr1(work[11]) + Wr2(work[6]);
	tmp=B+Tr1(E)+Ch(E,D,C)+0x4A7484AAU+work[13]; F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);

	work[14] = work[6] + 0x00000100U + Wr1(work[12]) + Wr2(work[7]);
	tmp=C+Tr1(F)+Ch(F,E,D)+0x5CB0A9DCU+work[14]; G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);

	work[15] = work[8] +0x11002000U+ work[7] + Wr1(work[13]);
	tmp=D+Tr1(G)+Ch(G,F,E)+0x76F988DAU+work[15]; H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);

#ifdef BITALIGN
	work[16] = work[9] + ((work[14]>>10)^rot(work[14],19)^amd_bitalign(work[14]^0x10000, work[14], 17));
#else
	work[16] = work[9] + Wr1(work[14]) + 0x80000000;
#endif
	tmp=E+Tr1(H)+Ch(H,G,F)+0x983E5152U+work[16]; A += tmp; E = tmp+Tr2(D)+Ma(D,C,B);

	work[17] = work[10] + Wr1(work[15]);
	tmp=F+Tr1(A)+Ch(A,H,G)+0xA831C66DU+work[17]; B += tmp; F = tmp+Tr2(E)+Ma(E,D,C);

	work[18] = work[11] + Wr1(work[16]);
	tmp=G+Tr1(B)+Ch(B,A,H)+0xB00327C8U+work[18]; C += tmp; G = tmp+Tr2(F)+Ma(F,E,D);

	work[19] = work[12] + Wr1(work[17]);
	tmp=H+Tr1(C)+Ch(C,B,A)+0xBF597FC7U+work[19]; D += tmp; H = tmp+Tr2(G)+Ma(G,F,E);

	work[20] = work[13] + Wr1(work[18]);
	tmp=A+Tr1(D)+Ch(D,C,B)+0xC6E00BF3U+work[20]; E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);

	work[21] = work[14] + Wr1(work[19]);
	tmp=B+Tr1(E)+Ch(E,D,C)+0xD5A79147U+work[21]; F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);

	work[22] = work[15] + 0x00400022U+Wr1(work[20]);
	tmp=C+Tr1(F)+Ch(F,E,D)+0x06CA6351U+work[22]; G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);

	work[23] = work[16] +0x00000100U +  Wr1(work[21]) + Wr2(work[8]);
	tmp=D+Tr1(G)+Ch(G,F,E)+0x14292967U+work[23]; H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);

	work[24] = work[17] + work[8] + Wr1(work[22]) + Wr2(work[9]);
	tmp=E+Tr1(H)+Ch(H,G,F)+0x27B70A85U+work[24]; A += tmp; E = tmp+Tr2(D)+Ma(D,C,B);

	work[25] = work[18] + work[9] + Wr1(work[23]) + Wr2(work[10]);
	tmp=F+Tr1(A)+Ch(A,H,G)+0x2E1B2138U+work[25]; B += tmp; F = tmp+Tr2(E)+Ma(E,D,C);

	work[26] = work[19] + work[10] + Wr1(work[24]) + Wr2(work[11]);
	tmp=G+Tr1(B)+Ch(B,A,H)+0x4D2C6DFCU+work[26]; C += tmp; G = tmp+Tr2(F)+Ma(F,E,D);

	work[27] = work[20] + work[11] + Wr1(work[25]) + Wr2(work[12]);
	tmp=H+Tr1(C)+Ch(C,B,A)+0x53380D13U+work[27]; D += tmp; H = tmp+Tr2(G)+Ma(G,F,E);

	work[28] = work[21] + work[12] + Wr1(work[26]) + Wr2(work[13]);
	tmp=A+Tr1(D)+Ch(D,C,B)+0x650A7354U+work[28]; E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);

	work[29] = work[22] + work[13] + Wr1(work[27]) + Wr2(work[14]);
	tmp=B+Tr1(E)+Ch(E,D,C)+0x766A0ABBU+work[29]; F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);

	work[30] = work[23] + work[14] + Wr1(work[28]) + Wr2(work[15]);
	tmp=C+Tr1(F)+Ch(F,E,D)+0x81C2C92EU+work[30]; G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);

	work[31] = work[24] + work[15] + Wr1(work[29]) + Wr2(work[16]);
	tmp=D+Tr1(G)+Ch(G,F,E)+0x92722C85U+work[31]; H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);

	work[32] = work[25] + work[16] + Wr1(work[30]) + Wr2(work[17]);
	tmp=E+Tr1(H)+Ch(H,G,F)+0xA2BFE8A1U+work[32]; A += tmp; E = tmp+Tr2(D)+Ma(D,C,B);

	work[33] = work[26] + work[17] + Wr1(work[31]) + Wr2(work[18]);
	tmp=F+Tr1(A)+Ch(A,H,G)+0xA81A664BU+work[33]; B += tmp; F = tmp+Tr2(E)+Ma(E,D,C);

	work[34] = work[27] + work[18] + Wr1(work[32]) + Wr2(work[19]);
	tmp=G+Tr1(B)+Ch(B,A,H)+0xC24B8B70U+work[34]; C += tmp; G = tmp+Tr2(F)+Ma(F,E,D);

	work[35] = work[28] + work[19] + Wr1(work[33]) + Wr2(work[20]);
	tmp=H+Tr1(C)+Ch(C,B,A)+0xC76C51A3U+work[35]; D += tmp; H = tmp+Tr2(G)+Ma(G,F,E);

	work[36] = work[29] + work[20] + Wr1(work[34]) + Wr2(work[21]);
	tmp=A+Tr1(D)+Ch(D,C,B)+0xD192E819U+work[36]; E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);

	work[37] = work[30] + work[21] + Wr1(work[35]) + Wr2(work[22]);
	tmp=B+Tr1(E)+Ch(E,D,C)+0xD6990624U+work[37]; F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);

	work[38] = work[31] + work[22] + Wr1(work[36]) + Wr2(work[23]);
	tmp=C+Tr1(F)+Ch(F,E,D)+0xF40E3585U+work[38]; G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);

	work[39] = work[32] + work[23] + Wr1(work[37]) + Wr2(work[24]);
	tmp=D+Tr1(G)+Ch(G,F,E)+0x106AA070U+work[39]; H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);

	work[40] = work[33] + work[24] + Wr1(work[38]) + Wr2(work[25]);
	tmp=E+Tr1(H)+Ch(H,G,F)+0x19A4C116U+work[40]; A += tmp; E = tmp+Tr2(D)+Ma(D,C,B);

	work[41] = work[34] + work[25] + Wr1(work[39]) + Wr2(work[26]);
	tmp=F+Tr1(A)+Ch(A,H,G)+0x1E376C08U+work[41]; B += tmp; F = tmp+Tr2(E)+Ma(E,D,C);

	work[42] = work[35] + work[26] + Wr1(work[40]) + Wr2(work[27]);
	tmp=G+Tr1(B)+Ch(B,A,H)+0x2748774CU+work[42]; C += tmp; G = tmp+Tr2(F)+Ma(F,E,D);

	work[43] = work[36] + work[27] + Wr1(work[41]) + Wr2(work[28]);
	tmp=H+Tr1(C)+Ch(C,B,A)+0x34B0BCB5U+work[43]; D += tmp; H = tmp+Tr2(G)+Ma(G,F,E);

	work[44] = work[37] + work[28] + Wr1(work[42]) + Wr2(work[29]);
	tmp=A+Tr1(D)+Ch(D,C,B)+0x391C0CB3U+work[44]; E += tmp; A = tmp+Tr2(H)+Ma(H,G,F);

	work[45] = work[38] + work[29] + Wr1(work[43]) + Wr2(work[30]);
	tmp=B+Tr1(E)+Ch(E,D,C)+0x4ED8AA4AU+work[45]; F += tmp; B = tmp+Tr2(A)+Ma(A,H,G);

	work[46] = work[39] + work[30] + Wr1(work[44]) + Wr2(work[31]);
	tmp=C+Tr1(F)+Ch(F,E,D)+0x5B9CCA4FU+work[46]; G += tmp; C = tmp+Tr2(B)+Ma(B,A,H);

	work[47] = work[40] + work[31] + Wr1(work[45]) + Wr2(work[32]);
	tmp=D+Tr1(G)+Ch(G,F,E)+0x682E6FF3U+work[47]; H += tmp; D = tmp+Tr2(C)+Ma(C,B,A);

	work[48] = work[41] + work[32] + Wr1(work[46]) + Wr2(work[33]);
	tmp=E+Tr1(H)+Ch(H,G,F)+0x748F82EEU+work[48]; A += tmp; E = tmp+Tr2(D)+Ma(D,C,B)+A+ work[45] + work[36];

	work[49] = work[42] + work[33] + Wr1(work[47]) + Wr2(work[34]);
	B += F + Tr1(A)+Ch(A,H,G)+0x78A5636FU+work[49];

	work[50] = work[43] + work[34] + Wr1(work[48]) + Wr2(work[35]);
	C += G + Tr1(B)+Ch(B,A,H)+0x84C87814U+work[50];

	D += H + Tr1(C)+Ch(C,B,A)+0x8CC70208U + work[44] + work[35] + Wr1(work[49]) + Wr2(work[36]);
	E +=     Tr1(D)+Ch(D,C,B)             + Wr1(work[50]) + Wr2(work[37]);

#if defined(VECTORS)
	if (any(E==(u)(0x136032EDU,0x136032EDU)))
		*output = get_global_id(0);
#elif defined(VECTORS4)
	if (any(E==(u)(0x136032EDU,0x136032EDU,0x136032EDU,0x136032EDU)))
		*output = get_global_id(0);
#else
	if (E==0x136032EDU)
		*output = get_global_id(0);
#endif
}
