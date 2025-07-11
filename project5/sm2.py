# sm2.py
import hashlib
import random
from ecc import n, G, point_mul, point_add
from gmssl import sm3, func

def hash_msg(msg: bytes) -> int:
    digest = sm3.sm3_hash(func.bytes_to_list(msg))
    return int(digest, 16)

def sign(msg: bytes, d: int):
    e = hash_msg(msg)
    while True:
        k = random.randint(1, n - 1)
        x1, y1 = point_mul(k, G)
        r = (e + x1) % n
        if r == 0 or r + k == n:
            continue
        s = (inverse_mod(1 + d, n) * (k - r * d)) % n
        if s != 0:
            break
    return (r, s)

def verify(msg: bytes, sig: tuple, P):
    r, s = sig
    e = hash_msg(msg)
    t = (r + s) % n
    if t == 0:
        return False
    x1, y1 = point_add(point_mul(s, G), point_mul(t, P))
    R = (e + x1) % n
    return R == r

def inverse_mod(k, mod):
    return pow(k, -1, mod)
