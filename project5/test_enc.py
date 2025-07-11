# test_enc.py
from keygen import gen_keypair
from encrypt import encrypt, decrypt

def main():
    msg = b"zjy2022004601092222233333"
    d, P = gen_keypair()
    print("明文:", msg)
    C = encrypt(msg, P)
    print("密文:")
    print("C1 =", C[0])
    print("C3 =", C[1].hex())
    print("C2 =", C[2].hex())

    m2 = decrypt(C, d)
    print("解密结果:", m2)

    print("正确解密:", m2 == msg)

if __name__ == "__main__":
    main()
