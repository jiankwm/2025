## SM2 算法实现与优化（Python）

本项目基于国密算法 SM2，实现了完整的加解密流程，并对其进行了性能优化，包括Jacobian 坐标加速、并行计算等优化策略。

---

### 项目结构

```bash
.
├── ecc.py          # 椭圆曲线基本操作
├── ecc_jacobian.py # 椭圆曲线基本操作（支持 Jacobian 坐标点乘）
├── keygen.py       # 密钥生成
├── encrypt.py      # 加密与解密函数
├── encrypt_thread.py      # 加密与解密函数（并行化）
├── kdf.py          # 密钥派生函数 KDF
├── test_enc.py     # 测试脚本（包含性能测试）
```

#### 1. 基础实现

- 使用椭圆曲线参数实现 SM2 基本加解密
- 原始点乘为 Affine 坐标，性能较低

#### 2. 替换哈希函数为 SM3

- 使用 `gmssl.sm3` 替代 Python hashlib（非国密）
- 提升合规性并统一哈希接口

####  3. 使用 Jacobian 坐标优化点乘

- 将 `point_mul()` 改为支持 Jacobian 坐标
- 减少模逆操作，点乘效率显著提升

####  4. 加密中并行计算 `k*G` 和 `k*P`

- 使用 `ThreadPoolExecutor` 同时计算两个点乘
- 平均加密性能提升 30~40%

#### 5. 添加性能测试模块

- 在 `test_enc.py` 中记录加解密时间
- 便于评估不同优化效果

------

###  性能测试对比

| 实现版本         | 加密时间 | 解密时间 |
| ---------------- | -------- | -------- |
| 原始 Affine 点乘 | 19.81 ms | 12.75ms  |
| Jacobian         | 14.60ms  | 8.80 ms  |
| Jacobian + 并行  | 10.00ms  | 5.89 ms  |

> 注：测试环境为 Windows + Python 3.10，消息长度约 125 字节

未优化：![46062ace-8e3b-4d40-a17f-ca35ef97dacb](E:\Desktop\wangan\2025\project5\readme.assets\46062ace-8e3b-4d40-a17f-ca35ef97dacb.png)

使用Jacobian 优化版点乘：

![4429a666-8f25-484f-8f51-f9ce9fc63a82](E:\Desktop\wangan\2025\project5\readme.assets\4429a666-8f25-484f-8f51-f9ce9fc63a82.png)

在此基础上对加密并行化：

![8fefd91b-92a7-406b-9ffc-e0e2181161a6](E:\Desktop\wangan\2025\project5\readme.assets\8fefd91b-92a7-406b-9ffc-e0e2181161a6.png)