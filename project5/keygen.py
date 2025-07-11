# keygen.py
import random
from ecc import n, G, point_mul

def gen_keypair():
    d = random.randint(1, n-1)
    P = point_mul(d, G)
    return d, P
