# encrypt.py
import ecc
from ecc import G, n, point_mul
from gmssl import sm3, func
from kdf import kdf
import random
from concurrent.futures import ThreadPoolExecutor

def encrypt(msg: bytes, P):
    k = random.randint(1, ecc.n - 1)

    with ThreadPoolExecutor() as executor:
        f1 = executor.submit(point_mul, k, G)
        f2 = executor.submit(point_mul, k, P)

        C1 = f1.result()
        x2, y2 = f2.result()

    x2_bytes = x2.to_bytes(32, 'big')
    y2_bytes = y2.to_bytes(32, 'big')

    t = kdf(x2_bytes + y2_bytes, len(msg))
    if int.from_bytes(t, 'big') == 0:
        raise ValueError("KDF output is zero, try another k")

    C2 = bytes([m ^ t_i for m, t_i in zip(msg, t)])

    C3 = sm3.sm3_hash(func.bytes_to_list(x2_bytes + msg + y2_bytes))
    return C1, C2, C3

def decrypt(C, d):
    C1, C2, C3 = C
    x2, y2 = point_mul(d, C1)

    x2_bytes = x2.to_bytes(32, 'big')
    y2_bytes = y2.to_bytes(32, 'big')

    t = kdf(x2_bytes + y2_bytes, len(C2))
    if int.from_bytes(t, 'big') == 0:
        raise ValueError("KDF output is zero")

    m = bytes([c ^ t_i for c, t_i in zip(C2, t)])
    u = sm3.sm3_hash(func.bytes_to_list(x2_bytes + m + y2_bytes))
    if u != C3:
        raise ValueError("C3 mismatch: invalid decryption")

    return m

