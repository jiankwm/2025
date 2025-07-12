# P1_client.py
import socket, pickle, random, hashlib
import gmpy2
from gmpy2 import mpz, powmod

# 群参数
p = mpz(gmpy2.next_prime(2**127))
g = mpz(3)

def H(x):
    digest = hashlib.sha256(str(x).encode()).hexdigest()
    return mpz(int(digest, 16)) % p

# 客户端 P1 的输入集合
P1_V = ["alice", "bob", "carol"]
k1 = random.randint(1, p - 1)

# 建立连接并发送数据
client = socket.socket()
client.connect(('localhost', 6666))

# 1. 计算 Z = H(v)^k1，打乱并发送
Z = [powmod(H(v), k1, p) for v in P1_V]
random.shuffle(Z)
client.sendall(pickle.dumps(Z))
print("[P1] 发送 Z 完成")

# 2. 接收 Z2、pubkey、P2_pairs
Z2, pubkey, P2_pairs = pickle.loads(client.recv(40960))
print("[P1] 接收 Z2 和加密对组完成")

# 3. 对每个 H(w)^k2 做一次幂运算，并判断是否属于 Z2
intersection_ct = []
for hw_k2, ct in P2_pairs:
    hw_k1k2 = powmod(hw_k2, k1, p)
    if hw_k1k2 in Z2:
        intersection_ct.append(ct)

# 4. 同态加密求和
if intersection_ct:
    ct_sum = intersection_ct[0]
    for ct in intersection_ct[1:]:
        ct_sum = ct_sum + ct
else:
    ct_sum = pubkey.encrypt(0)

# 5. 发送 AEnc(S_J) 给 P2
client.sendall(pickle.dumps(ct_sum))
print("[P1] 发送 AEnc(S_J) 完成")
client.close()
