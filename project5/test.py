# test.py
from keygen import gen_keypair
from sm2 import sign, verify

def main():
    msg = b"zjy2022004601092222233333"
    d, P = gen_keypair()
    r, s = sign(msg, d)
    print("签名: r =", hex(r), "s =", hex(s))
    valid = verify(msg, (r, s), P)
    print("验签结果:", valid)

if __name__ == "__main__":
    main()
