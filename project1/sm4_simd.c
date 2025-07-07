#include <stdio.h>
#include <windows.h>    // For QueryPerformanceFrequency and QueryPerformanceCounter
#include <string.h>     // For memcpy
#include <immintrin.h>  // For SSE/AVX intrinsics (Intel specific)

// Macro for alignment, important for SIMD
#ifdef _MSC_VER
#define ALIGN(x) __declspec(align(x))
#else
#define ALIGN(x) __attribute__((aligned(x)))
#endif

#define u8 unsigned char
#define u32 unsigned long

// S盒
const u8 Sbox[256] = {
	0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
	0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
	0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
	0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
	0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
	0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
	0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
	0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
	0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
	0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
	0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
	0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
	0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
	0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
	0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
	0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};

// 密钥扩展算法的常数FK
const u32 FK[4] = {
	0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc
};

// 密钥扩展算法的固定参数CK
const u32 CK[32] = {
	0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
	0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
	0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
	0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
	0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
	0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
	0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
	0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

// 循环左移函数 (SIMD 版本)
static inline __m128i _mm_rotl_epi32(__m128i x, const int shift) {
    return _mm_or_si128(_mm_slli_epi32(x, shift), _mm_srli_epi32(x, 32 - shift));
}

// 线性变换L (SIMD 版本)
static inline __m128i functionL1_SIMD(__m128i a) {
    // a ^ (a <<< 2) ^ (a <<< 10) ^ (a <<< 18) ^ (a <<< 24)
    __m128i temp = _mm_xor_si128(a, _mm_rotl_epi32(a, 2));
    temp = _mm_xor_si128(temp, _mm_rotl_epi32(a, 10));
    temp = _mm_xor_si128(temp, _mm_rotl_epi32(a, 18));
    temp = _mm_xor_si128(temp, _mm_rotl_epi32(a, 24));
    return temp;
}

// 线性变换L' (SIMD 版本)
static inline __m128i functionL2_SIMD(__m128i a) {
    // a ^ (a <<< 13) ^ (a <<< 23)
    __m128i temp = _mm_xor_si128(a, _mm_rotl_epi32(a, 13));
    temp = _mm_xor_si128(temp, _mm_rotl_epi32(a, 23));
    return temp;
}

// S盒查找函数 (SIMD 版本)
// 使用 _mm_shuffle_epi8 实现。
// 注意：这仍然不是“纯粹”的 SIMD S盒查找，因为它将__m128i解包到u8数组中进行标量查找。
// 真正的纯SIMD S盒查找需要将S盒预处理为多组16字节的查找表，
// 然后通过_mm_shuffle_epi8结合位掩码和多次查找来实现。
// 这里的实现是为了兼容性和简洁性，可能不是性能最佳的S盒SIMD方案。
static inline __m128i functionB_SIMD(__m128i b_in) {
    ALIGN(16) u8 temp_bytes[16]; // 确保数组对齐

    // 将__m128i存入u8数组
    _mm_store_si128((__m128i*)temp_bytes, b_in);

    // 对每个字节进行S盒查找（这部分是标量操作）
    for (int i = 0; i < 16; ++i) {
        temp_bytes[i] = Sbox[temp_bytes[i]];
    }
    // 将结果加载回__m128i
    return _mm_load_si128((__m128i*)temp_bytes);
}


// 合成变换T (SIMD 版本)
// mode: 1 for L1, 2 for L2
static inline __m128i functionT_SIMD(__m128i a, short mode) {
    __m128i b_transformed = functionB_SIMD(a);
    if (mode == 1) { // 明文的T，调用L1
        return functionL1_SIMD(b_transformed);
    } else { // 密钥的T，调用L2
        return functionL2_SIMD(b_transformed);
    }
}


/*
	密钥扩展算法第一步
	参数：	MK[4]：密钥  K[4]:中间数据，保存结果	（FK[4]：常数）
	返回值：无
*/
void extendFirst(u32 MK[], u32 K[]) {
    // Load MK and FK into __m128i registers
    __m128i mk_vec = _mm_loadu_si128((__m128i*)MK);
    __m128i fk_vec = _mm_loadu_si128((__m128i*)FK);

    // K = MK ^ FK
    __m128i k_vec = _mm_xor_si128(mk_vec, fk_vec);

    // Store result back to K
    _mm_storeu_si128((__m128i*)K, k_vec);
}

/*
	密钥扩展算法第二步
	参数：	RK[32]：轮密钥，保存结果    K[4]：中间数据 （CK[32]：固定参数）
	返回值：无
*/
void extendSecond(u32 RK[], u32 K[]) {
    // K is loaded and stored as 4 u32s, but SIMD operations are on __m128i
    u32 temp_K_scalar[4];
    memcpy(temp_K_scalar, K, sizeof(u32) * 4); // Work on a local copy

    for (int i = 0; i < 32; i++) {
        // Calculate the input to the T function: K[i+1] ^ K[i+2] ^ K[i+3] ^ CK[i]
        u32 t_input_scalar = temp_K_scalar[(i + 1) % 4] ^ temp_K_scalar[(i + 2) % 4] ^ temp_K_scalar[(i + 3) % 4] ^ CK[i];

        // Convert scalar input to a __m128i for SIMD T-function
        // Replicate the 32-bit scalar into all 4 lanes of a __m128i.
        __m128i t_input_vec = _mm_set1_epi32(t_input_scalar);

        // Apply T function (SIMD version)
        __m128i t_result_vec = functionT_SIMD(t_input_vec, 2); // mode 2 for L'

        // Extract the result from the first lane (or any lane, since they are identical)
        u32 t_result_scalar = _mm_cvtsi128_si32(t_result_vec); // Extract lower 32 bits

        temp_K_scalar[(i + 4) % 4] = temp_K_scalar[i % 4] ^ t_result_scalar;
        RK[i] = temp_K_scalar[(i + 4) % 4];
    }
    // No need to copy back to K, as K is not used after this function in the getRK flow.
}

/*
	密钥扩展算法
	参数：	MK[4]：密钥     K[4]：中间数据    RK[32]：轮密钥，保存结果
	返回值：无
*/
void getRK(u32 MK[], u32 K[], u32 RK[]) {
	extendFirst(MK, K);
	extendSecond(RK, K);
}

/*
	迭代32次 (SIMD 优化版)
	参数：	u32 X[4]：迭代对象，保存结果    u32 RK[32]：轮密钥
	返回值：无
*/
void iterate32_SIMD(u32 X_in_out[], u32 RK[]) {
    // Load X into 4 separate u32 variables, as the state array X is processed sequentially.
    // We will extract/replicate to/from __m128i as needed for T function.
    u32 X[4];
    memcpy(X, X_in_out, sizeof(u32) * 4); // Work on a local copy

    for (short i = 0; i < 32; i++) {
        // Calculate input for F function: (X[(i+1)%4] ^ X[(i+2)%4] ^ X[(i+3)%4] ^ RK[i])
        u32 f_input_scalar = X[(i + 1) % 4] ^ X[(i + 2) % 4] ^ X[(i + 3) % 4] ^ RK[i];

        // Replicate the scalar into a __m128i for functionT_SIMD
        __m128i f_input_vec = _mm_set1_epi32(f_input_scalar);

        // Apply T function (mode 1 for L)
        __m128i t_result_vec = functionT_SIMD(f_input_vec, 1);

        // Extract the result (it's the same in all lanes)
        u32 t_result_scalar = _mm_cvtsi128_si32(t_result_vec);

        // Calculate X[(i + 4) % 4]
        X[(i + 4) % 4] = X[i % 4] ^ t_result_scalar;
    }

    // Store final X values back to X_in_out (which will be Y_out or X_decrypted)
    // The order needs to be X35, X34, X33, X32 as per SM4 specification (reverse output)
    X_in_out[0] = X[3]; // X35 (which is X[ (3+32)%4 ] = X[3])
    X_in_out[1] = X[2]; // X34
    X_in_out[2] = X[1]; // X33
    X_in_out[3] = X[0]; // X32
}


/*
	反转函数 (在 SIMD 优化版中不再需要独立调用，其功能并入 iterate32_SIMD 的存储逻辑)
	但为了保持接口一致性，暂时保留。
*/
void reverse(u32 X[], u32 Y[]) {
	short i;
	for (i = 0; i < 4; i++) {
		Y[i] = X[4 - 1 - i];
	}
}

/*
	加密算法 (SIMD 优化版)
	参数：	u32 X[4]：明文    u32 RK[32]：轮密钥    u32 Y[4]：密文，保存结果
	返回值：无
*/
void encryptSM4_SIMD(u32 X_in[], u32 RK[], u32 Y_out[]) {
    // Note: iterate32_SIMD already performs the reversal implicitly in its final store.
    u32 X_temp[4];
    memcpy(X_temp, X_in, sizeof(u32) * 4); // Operate on a copy
	iterate32_SIMD(X_temp, RK);
    memcpy(Y_out, X_temp, sizeof(u32) * 4); // Copy result to Y_out
}

/*
	解密算法 (SIMD 优化版)
	参数： 	u32 X[4]：密文    u32 RK[32]：轮密钥    u32 Y[4]：明文，保存结果
	返回值：无
*/
void decryptSM4_SIMD(u32 X_in[], u32 RK[], u32 Y_out[]) {
	short i;
	u32 reverseRK[32];
	for (i = 0; i < 32; i++) {
		reverseRK[i] = RK[32 - 1 - i];
	}
    u32 X_temp[4];
    memcpy(X_temp, X_in, sizeof(u32) * 4); // Operate on a copy
	iterate32_SIMD(X_temp, reverseRK);
    memcpy(Y_out, X_temp, sizeof(u32) * 4); // Copy result to Y_out
}


// --- Original functions for comparison (copied from your original code) ---
// These are not optimized with T-Tables or SIMD.
u32 functionB_original(u32 b) {
	u8 a[4];
	a[0] = b / 0x1000000;
	a[1] = b / 0x10000;
	a[2] = b / 0x100;
	a[3] = b;
	b = Sbox[a[0]] * 0x1000000 + Sbox[a[1]] * 0x10000 + Sbox[a[2]] * 0x100 + Sbox[a[3]];
	return b;
}

u32 loopLeft_original(u32 a, short length) {
	short i;
	for (i = 0; i < length; i++) {
		a = a * 2 + a / 0x80000000;
	}
	return a;
}

u32 functionL1_original(u32 a) {
	return a ^ loopLeft_original(a, 2) ^ loopLeft_original(a, 10) ^ loopLeft_original(a, 18) ^ loopLeft_original(a, 24);
}

u32 functionL2_original(u32 a) {
	return a ^ loopLeft_original(a, 13) ^ loopLeft_original(a, 23);
}

u32 functionT_original(u32 a, short mode) {
	return mode == 1 ? functionL1_original(functionB_original(a)) : functionL2_original(functionB_original(a));
}

void iterate32_original(u32 X[], u32 RK[]) {
    short i;
    u32 temp_X[4];
    memcpy(temp_X, X, sizeof(u32) * 4);
    for (i = 0; i < 32; i++) {
        temp_X[(i + 4) % 4] = temp_X[i % 4] ^ functionT_original(temp_X[(i + 1) % 4] ^ temp_X[(i + 2) % 4] ^ temp_X[(i + 3) % 4] ^ RK[i], 1);
    }
    memcpy(X, temp_X, sizeof(u32) * 4);
}

void encryptSM4_original(u32 X_in[], u32 RK[], u32 Y_out[]) {
    u32 X_temp[4];
    memcpy(X_temp, X_in, sizeof(u32) * 4);
	iterate32_original(X_temp, RK);
	reverse(X_temp, Y_out);
}

void decryptSM4_original(u32 X_in[], u32 RK[], u32 Y_out[]) {
	short i;
	u32 reverseRK[32];
	for (i = 0; i < 32; i++) {
		reverseRK[i] = RK[32 - 1 - i];
	}
    u32 X_temp[4];
    memcpy(X_temp, X_in, sizeof(u32) * 4);
	iterate32_original(X_temp, reverseRK);
	reverse(X_temp, Y_out);
}

/*
	测试数据：
	明文：	01234567 89abcdef fedcba98 76543210
	密钥：	01234567 89abcdef fedcba98 76543210
	密文：	681edf34 d206965e 86b3e94f 536e4246
*/
int main(void) {
    u32 X_plaintext[4] = {0x01234567, 0x89abcdef, 0xfedcba98, 0x76543210}; // 明文
    u32 MK_key[4]      = {0x01234567, 0x89abcdef, 0xfedcba98, 0x76543210}; // 密钥
    u32 RK[32];      // 轮密钥
    u32 K[4];        // 中间数据
    u32 Y_ciphertext_simd[4]; // SIMD版密文
    u32 X_decrypted_simd[4];  // SIMD版解密后的明文

    u32 Y_ciphertext_original[4]; // 原始版密文
    u32 X_decrypted_original[4];  // 原始版解密后的明文

    long long NUM_TESTS = 1000000; // 进行一百万次加密/解密测试，以获得稳定时间

    printf("SM4 SIMD 优化版与原始版性能对比\n");
    printf("----------------------------------\n");

    // --- 密钥扩展 (两版共用) ---
    getRK(MK_key, K, RK); // 轮密钥生成，这部分在两种实现中是通用的，且通常不是性能瓶颈。

    // --- SIMD 优化版测试 ---
    printf("SIMD 优化版:\n");
    LARGE_INTEGER t1_simd, t2_simd, tc;
    QueryPerformanceFrequency(&tc);
    QueryPerformanceCounter(&t1_simd);

    for (long long i = 0; i < NUM_TESTS; ++i) {
        u32 temp_X[4];
        memcpy(temp_X, X_plaintext, sizeof(u32) * 4); // 重置明文
        encryptSM4_SIMD(temp_X, RK, Y_ciphertext_simd); // 加密

        memcpy(temp_X, Y_ciphertext_simd, sizeof(u32) * 4); // 重置密文
        decryptSM4_SIMD(temp_X, RK, X_decrypted_simd);   // 解密
    }
    QueryPerformanceCounter(&t2_simd);
    double time_simd = (double)(t2_simd.QuadPart - t1_simd.QuadPart) / (double)tc.QuadPart;

    printf("  加密明文: %08x %08x %08x %08x\n", X_plaintext[0], X_plaintext[1], X_plaintext[2], X_plaintext[3]);
    printf("  加密密钥: %08x %08x %08x %08x\n", MK_key[0], MK_key[1], MK_key[2], MK_key[3]);
    printf("  生成的密文: %08x %08x %08x %08x\n", Y_ciphertext_simd[0], Y_ciphertext_simd[1], Y_ciphertext_simd[2], Y_ciphertext_simd[3]);
    printf("  解密后的明文: %08x %08x %08x %08x\n", X_decrypted_simd[0], X_decrypted_simd[1], X_decrypted_simd[2], X_decrypted_simd[3]);
    printf("  %lld 次加密/解密总时间: %fs\n", NUM_TESTS, time_simd);
    printf("  平均每次加密/解密时间: %e s\n", time_simd / NUM_TESTS);

    printf("\n");

    // --- 原始版 (未优化) 测试 ---
    printf("原始版 (未SIMD优化):\n");
    LARGE_INTEGER t1_orig, t2_orig;
    QueryPerformanceCounter(&t1_orig);

    for (long long i = 0; i < NUM_TESTS; ++i) {
        u32 temp_X[4];
        memcpy(temp_X, X_plaintext, sizeof(u32) * 4); // 重置明文
        encryptSM4_original(temp_X, RK, Y_ciphertext_original); // 加密

        memcpy(temp_X, Y_ciphertext_original, sizeof(u32) * 4); // 重置密文
        decryptSM4_original(temp_X, RK, X_decrypted_original);   // 解密
    }
    QueryPerformanceCounter(&t2_orig);
    double time_orig = (double)(t2_orig.QuadPart - t1_orig.QuadPart) / (double)tc.QuadPart;

    printf("  加密明文: %08x %08x %08x %08x\n", X_plaintext[0], X_plaintext[1], X_plaintext[2], X_plaintext[3]);
    printf("  加密密钥: %08x %08x %08x %08x\n", MK_key[0], MK_key[1], MK_key[2], MK_key[3]);
    printf("  生成的密文: %08x %08x %08x %08x\n", Y_ciphertext_original[0], Y_ciphertext_original[1], Y_ciphertext_original[2], Y_ciphertext_original[3]);
    printf("  解密后的明文: %08x %08x %08x %08x\n", X_decrypted_original[0], X_decrypted_original[1], X_decrypted_original[2], X_decrypted_original[3]);
    printf("  %lld 次加密/解密总时间: %fs\n", NUM_TESTS, time_orig);
    printf("  平均每次加密/解密时间: %e s\n", time_orig / NUM_TESTS);

    printf("\n");
    printf("优化效果：SIMD版比原始版快 %.2f%%\n", ((time_orig - time_simd) / time_orig) * 100.0);

    return 0;
}