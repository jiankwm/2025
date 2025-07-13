###  **基于 DDH 的私密交集求和协议（DDH-based Private Intersection-Sum Protocol）**

#### 协议目标

P_1 有一个集合 V,P2 有一个带值的集合 W,协议目标是计算出交集中的值的**加和**,这个协议允许双方在不知道对方数据内容的前提下，只得知交集中元素对应值的总和，而不会暴露各自的私有数据。

####  安全假设

- **DDH（Decisional Diffie-Hellman）假设**在群 G中成立；
- 哈希函数H 被建模为随机预言机；
- Paillier 加密为加法同态，安全对抗半诚实攻击；
- 双方按照协议执行，但可能试图推断对方私密集合。

#### 协议流程图

![81e74418-e3ac-4456-b4dd-23a899d55f4c](E:\Desktop\wangan\2025\project6\readme.assets\81e74418-e3ac-4456-b4dd-23a899d55f4c.png)

##### Setup 阶段

- 公共参数：
  - 群 G，阶为素数 p，生成元 g；
  - 哈希函数 
    $$
    H : \{0,1\}^* \rightarrow G
    $$
    
  - Paillier 加密算法
    $$
    (\text{AEnc}, \text{ADec})
    $$
    。
- 密钥生成：
  - P_1 随机选择 
    $$
    k_1 \in \mathbb{Z}_p^*
    $$
    
  - P_2 随机选择
    $$
    k_2 \in \mathbb{Z}_p^*
    $$
    
  - P_2 生成 Paillier 密钥对 (pk, sk)。

---

##### Round 1（P₁ → P₂）

1. 对每个 v_i ，计算：
   $$
   h_i = H(v_i)^{k_1} \mod p
   $$
2. 将所有 h_i打乱后作为集合 Z 发给 P_2。

---

##### Round 2（P₂ 处理并回复）

1. 对收到的每个
   $$
   h_i \in Z
   $$
   ，计算：
   $$
   z_i = h_i^{k_2} = H(v_i)^{k_1k_2} \mod p
   $$
   得到集合 $Z_2$。
2. 对 
   $$
   W = \{(w_j, t_j)\}
   $$
   中每个元素：
   - 计算
     $$
     h'_j = H(w_j)^{k_2} \mod p
     $$
     
   - 计算 
     $$
     c_j = \text{AEnc}_{pk}(t_j)
     $$
     
3. 将 $Z_2$、Paillier 公钥 pk以及打乱后的加密对组发送给 P_1

---

##### Round 3（P₁ 计算交集并求和）

1. 对每个 $(h'_j, c_j)$，计算：
   $$
   h''_j = (h'_j)^{k_1} = H(w_j)^{k_1k_2} \mod p
   $$
2. 若
   $$
   h''_j \in Z_2
   $$
   ，则认为
   $$
   w_j \in J
   $$
   ，将对应 c_j 加入结果列表。
3. 对所有交集密文求和（利用加法同态）：
   $$
   c = \sum_{j : h''_j \in Z_2} c_j = \text{AEnc}(S_J)
   $$
4. 将密文 c 发送给 P_2。

---

##### 解密输出（由 P₂ 执行）

- P_2解密：
  $$
  S_J = \text{ADec}_{sk}(c)
  $$
- 得到交集中元素的值的总和 S_J。

#### 测试结果

服务端

![984320ce-f276-4cd6-9cdf-fe6670d3945a](E:\Desktop\wangan\2025\project6\readme.assets\984320ce-f276-4cd6-9cdf-fe6670d3945a.png)

用户端

![4b5c7142-95ba-4159-8788-7811a067eee6](E:\Desktop\wangan\2025\project6\readme.assets\4b5c7142-95ba-4159-8788-7811a067eee6.png)

参考论文：https://eprint.iacr.org/2019/723.pdf