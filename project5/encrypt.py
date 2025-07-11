# encrypt.py
from ecc import G, point_mul, point_add
from kdf import kdf
from sm2 import hash_msg
from random import randint

def int_to_bytes(x: int, size: int = 32) -> bytes:
    return x.to_bytes(size, 'big')

def encrypt(msg: bytes, P):
    klen = len(msg)
    while True:
        k = randint(1, 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123 - 1)
        C1 = point_mul(k, G)
        x2, y2 = point_mul(k, P)
        x2_bytes = int_to_bytes(x2)
        y2_bytes = int_to_bytes(y2)
        t = kdf(x2_bytes + y2_bytes, klen)
        if all(b == 0 for b in t):
            continue
        C2 = bytes([m ^ t[i] for i, m in enumerate(msg)])
        C3 = int_to_bytes(hash_msg(x2_bytes + msg + y2_bytes))
        return (C1, C3, C2)

def decrypt(C, d):
    C1, C3, C2 = C
    x2, y2 = point_mul(d, C1)
    x2_bytes = int_to_bytes(x2)
    y2_bytes = int_to_bytes(y2)
    t = kdf(x2_bytes + y2_bytes, len(C2))
    if all(b == 0 for b in t):
        raise ValueError("KDF generated all-zero key!")
    M = bytes([c ^ t[i] for i, c in enumerate(C2)])
    u = int_to_bytes(hash_msg(x2_bytes + M + y2_bytes))
    if u != C3:
        raise ValueError("Decryption failed! Hash mismatch.")
    return M
