#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <Windows.h>

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

std::mutex mtx;
std::vector<uint32_t> V(8);
std::vector<uint32_t> W(68);

void compress(const std::string& paddedMessage, int start) {
    uint32_t T[64];
    for (int i = 0; i < 16; i++) {
        T[i] = 0x79CC4519;
    }
    for (int i = 16; i < 64; i++) {
        T[i] = ROTATE_LEFT(T[i - 16] ^ T[i - 9] ^ ROTATE_LEFT(T[i - 3], 15), 1);
    }

    for (int i = start; i < paddedMessage.size(); i += 64) {
        for (int j = 0; j < 16; j++) {
            W[j] = (static_cast<uint32_t>(paddedMessage[i + j * 4 + 0]) << 24) |
                (static_cast<uint32_t>(paddedMessage[i + j * 4 + 1]) << 16) |
                (static_cast<uint32_t>(paddedMessage[i + j * 4 + 2]) << 8) |
                (static_cast<uint32_t>(paddedMessage[i + j * 4 + 3]) << 0);
        }
        for (int j = 16; j < 68; j++) {
            W[j] = ROTATE_LEFT(W[j - 16] ^ W[j - 9] ^ ROTATE_LEFT(W[j - 3], 15), 1) ^
                W[j - 6] ^ ROTATE_LEFT(W[j - 13], 7);
        }

        uint32_t A = V[0];
        uint32_t B = V[1];
        uint32_t C = V[2];
        uint32_t D = V[3];
        uint32_t E = V[4];
        uint32_t F = V[5];
        uint32_t G = V[6];
        uint32_t H = V[7];

        for (int j = 0; j < 64; j++) {
            uint32_t SS1 = ROTATE_LEFT((ROTATE_LEFT(A, 12) + E + ROTATE_LEFT(T[j], j)), 7);
            uint32_t SS2 = SS1 ^ ROTATE_LEFT(A, 12);
            uint32_t TT1 = (A ^ B ^ C) + D + SS2 + W[j];
            uint32_t TT2 = (E ^ F ^ G) + H + SS1 + T[j];
            D = C;
            C = ROTATE_LEFT(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = ROTATE_LEFT(F, 19);
            F = E;
            E = ROTATE_LEFT(TT2, 9) ^ ROTATE_LEFT(TT2, 17);
        }

        std::lock_guard<std::mutex> lock(mtx);
        V[0] ^= A;
        V[1] ^= B;
        V[2] ^= C;
        V[3] ^= D;
        V[4] ^= E;
        V[5] ^= F;
        V[6] ^= G;
        V[7] ^= H;
    }
}

std::string sm3Hash(const std::string& message) {
    uint32_t IV[8] = {
        0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
        0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
    };

    std::string paddedMessage = message;
    paddedMessage += static_cast<char>(0x80);
    uint64_t messageLength = message.size() * 8;
    while ((paddedMessage.size() + 8) % 64 != 0) {
        paddedMessage += static_cast<char>(0x00);
    }
    for (int i = 0; i < 8; i++) {
        paddedMessage += static_cast<char>((messageLength >> ((7 - i) * 8)) & 0xFF);
    }

    std::thread threads[4];
    for (int i = 0; i < 8; i++) {
        V[i] = IV[i];
    }

    for (int i = 0; i < 4; i++) {
        threads[i] = std::thread(compress, std::ref(paddedMessage), i * 64);
    }

    for (int i = 0; i < 4; i++) {
        threads[i].join();
    }

    std::stringstream hashStream;
    for (int i = 0; i < 8; i++) {
        hashStream << std::setfill('0') << std::setw(8) << std::hex << V[i];
    }
    return hashStream.str();
}

int main() {
    std::string message = "zjy202200460109u0OLoxZg20CGLrGK882W10SNbY83JJmtgoThiIwWqR5w0QfjmHZoH3DdXo3ibmj738o88FXinFsMg34mHmVtwyyve4zOgG63WY92Fyup2bMX3qvvbKU9lpbcsxuxrDwd2bNFAqxx7S4QgR3HE2G4Ep0CH5g2GzXSvPLktsw2KzdP3UBJyDrpWzZD5DFKwX0FOSACYJ1BVXLbOC881cpArDe0cbE6CnU3BOiXju5xEcqTBL47M9V3JCXOnxdObv4fqiegP2bvqRTFd7qsAktVvYJEuCbaks0cRXxxrZXHgr0HeYtnxsOuRGWhHoNpSinhruZCxU7tO8fV5KKjcP37P6FXDZCK62S6XK2T2tod1O0empHc7EXQBKwNumyrYGMbn1drzOrJNwoUL8DhSaIOELmMCBglIa9Cu9YMlUpIPITRqQAeBi2jMXZ3Kmu79BNb1eV60lYN2fyWDU4aj3nZ6zi1RtxmK77TuF8IODSqh6QhW8795";
    LARGE_INTEGER t1, t2, tc;
    QueryPerformanceFrequency(&tc);
    QueryPerformanceCounter(&t1);
    std::string hash = sm3Hash(message);
    std::cout << "哈希值: " << hash << std::endl;
    QueryPerformanceCounter(&t2);
    double time = (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart;
    printf("runtime:%fs", time);
    return 0;
}