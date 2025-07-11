# kdf.py
from gmssl import sm3, func

def kdf(Z: bytes, klen: int) -> bytes:
    ct = 1
    key = b''
    while len(key) < klen:
        msg = Z + ct.to_bytes(4, 'big')
        digest = sm3.sm3_hash(func.bytes_to_list(msg))
        key += bytes.fromhex(digest)
        ct += 1
    return key[:klen]

