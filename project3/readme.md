## 用circom实现poseidon2哈希算法的电路

这是一个关于使用 **Circom** 和 **snarkjs** 实现 Poseidon2 哈希函数零知识证明的项目。本项目展示从头开始构建一个 ZKP 电路，并为其生成和验证证明。**本实验中的 Poseidon2 哈希函数实现，包括其代码逻辑和 S-box 参数，** **仅适用于学习和实验目的，不适用于生产实践**。哈希算法参数(n,t,d)=(256,3,5)。

### 概述

本项目实现了一个简化的 **Poseidon2 哈希函数**，并将其作为 Circom 电路进行证明。用户可以提供私有输入，并在不泄露私有输入的情况下，证明该输入的 Poseidon2 哈希值与一个公开的哈希值匹配。

**环境：**

- **Circom**: 用于编写零知识证明电路的 DSL (Domain Specific Language)。
- **snarkjs**: 用于零知识证明的命令行工具，处理可信设置、证明生成和验证。
- **Node.js**: 运行 Witness 生成器。
- **Python**: 用于计算电路的预期哈希输出，以便作为公共输入。



### 核心文件结构

```
.
├── circuits/
    └── correct.txt       # Circom 电路日志版本，用于输出最终哈希值以填入input.json
│   └── poseidon2.circom  # Circom 电路定义文件
├── build/
│   ├── poseidon2.r1cs    # 编译后的 R1CS (Rank-1 Constraint System) 文件
│   ├── poseidon2.wasm    # 编译后的 WebAssembly 文件，用于 Witness 生成
│   ├── poseidon2_js/     # Witness 生成相关 JS 文件和 WASM 模块
│   │   └── generate_witness.js
│   │   └── poseidon2.wasm
│   │   └── witness_calculator.js
│   ├── witness.wtns      # 生成的 Witness 文件
│   ├── poseidon2_final.zkey # 最终的证明密钥 (Phase 2)
│   ├── verification_key.json # 导出的验证密钥
│   ├── proof.json        # 生成的零知识证明文件
│   └── public.json       # 包含公共输入/输出的文件
├── input.json            # 电路输入文件 (私有和公共)

```



### 步骤

按照以下步骤来编译电路、生成 Witness、创建证明并验证它。

#### 1. 环境准备

确保已安装以下工具：

- **Node.js**: 用于运行 `snarkjs` 和 Witness 生成脚本。
  - 安装: https://nodejs.org/
- **Circom**: Circom 编译器。
  - 安装: `npm install -g circom` 
- **snarkjs**: 用于零知识证明的 CLI 工具。
  - 安装: `npm install -g snarkjs`

#### 2. 编译电路

首先，将 Circom 电路编译成 R1CS 和 WebAssembly 文件。

Bash

```
circom circuits/poseidon2.circom --r1cs --wasm --sym -o build/ --prime bn128
```

- `--r1cs`: 生成 R1CS 约束系统文件 (`.r1cs`)。
- `--wasm`: 生成 WebAssembly 文件 (`.wasm`) 和相关的 JavaScript 助手文件，用于 Witness 生成。
- `--sym`: 生成 `.sym` 文件，用于调试。
- `-o build/`: 将输出文件存放到 `build/` 目录。
- `--prime bn128`: 使用 BN128 椭圆曲线的素数域进行计算。



#### 3. 准备输入文件 (`input.json`)

电路需要两个输入：私有输入 `in_private` 和公共输出 `hash_output`。`hash_output` 必须是 `in_private` 经过 Poseidon2 哈希后的实际结果。

**`input.json` 示例：**

JSON

```
{
  "in_private": ["10", "20"],
  "hash_output": "978769251912840039204148718141236740445251797222770407128323859497250111719"
}
```

**重要提示：** `hash_output` 的值必须与电路实际计算出的值精确匹配。你可以通过运行 Circom 电路编译后的 Witness 生成器（即使会报错断言失败），并观察日志中最后轮次的 `Out State[0]` 值来获取这个准确的哈希值。

![ebe47582-4eb2-4693-85e8-d2a36b8bc7cb](E:\Desktop\wangan\2025\project3\readme.assets\ebe47582-4eb2-4693-85e8-d2a36b8bc7cb.png)

#### 4. 生成 Witness

Witness 包含了所有电路内部信号的具体值，是生成证明的基础。

```
node build/poseidon2_js/generate_witness.js build/poseidon2_js/poseidon2.wasm input.json build/witness.wtns
```

- `build/poseidon2_js/generate_witness.js`: 由 Circom 自动生成的 Witness 生成脚本。
- `build/poseidon2_js/poseidon2.wasm`: 编译后的 WebAssembly 电路。
- `input.json`: 包含私有输入和预期公共输出。
- `build/witness.wtns`: 生成的 Witness 文件。



#### 5. 生成证明 (Groth16 协议)

我们将使用 `snarkjs` 的 Groth16 协议来生成零知识证明。

##### a. 启动可信设置 (Trusted Setup - Phase 2)

这个步骤会根据 `.r1cs` 文件和通用的 `powersOfTau` 文件创建一个特定于电路的证明密钥 (`.zkey`)。

首先，需要一个 `powersOfTau` 文件。本项目所需的约束数量通常可以使用 `powersOfTau28_hez_final_12.ptau`。如果你的电路约束数量超过 2^12，则可能需要更大的 `.ptau` 文件。

可以通过 `snarkjs info -r build/poseidon2.r1cs` 查看约束数量。

```
# 执行 Phase 2 设置，使用合适的 .ptau 文件
# 如果没有 powersOfTau28_hez_final_12.ptau，snarkjs 可能会尝试下载，或你需要手动下载
snarkjs groth16 setup build/poseidon2.r1cs powersOfTau28_hez_final_12.ptau build/poseidon2_0000.zkey
```

![57914491-33f2-462b-bba3-945fa7cfb803](E:\Desktop\wangan\2025\project3\readme.assets\57914491-33f2-462b-bba3-945fa7cfb803.png)

##### b. 贡献随机性 (可选但推荐)

为了增加设置的安全性，可以向 `.zkey` 文件贡献一些随机性。

```
snarkjs zkey contribute build/poseidon2_0000.zkey build/poseidon2_final.zkey --name="Your Project Contributor" -v
```

系统会提示你输入一些随机字符串，输入后按回车。

![84cfe993-c5c3-4559-b077-4a9d9d3932b1](E:\Desktop\wangan\2025\project3\readme.assets\84cfe993-c5c3-4559-b077-4a9d9d3932b1.png)

##### c. 导出验证密钥

从最终的 `.zkey` 文件中提取出一个公共的验证密钥，用于任何人验证证明。

```
snarkjs zkey export verificationkey build/poseidon2_final.zkey build/verification_key.json
```

##### d. 生成证明

现在，使用 Witness 和最终的 `.zkey` 文件来生成零知识证明。

```
snarkjs groth16 prove build/poseidon2_final.zkey build/witness.wtns build/proof.json build/public.json
```

- `build/proof.json`: 生成的证明文件。
- `build/public.json`: 包含公开输入和输出的文件，这是证明验证者需要的。

![fd04d466-e0e9-4c7e-b5cf-b225e82b16ce](E:\Desktop\wangan\2025\project3\readme.assets\fd04d466-e0e9-4c7e-b5cf-b225e82b16ce.png)

#### 6. 验证证明

最后一步是验证生成的证明是否有效。

```
snarkjs groth16 verify build/verification_key.json build/public.json build/proof.json
```

如果验证成功，将在控制台看到 `OK`。这表明证明是有效的，并且秘密输入符合电路定义的逻辑，且哈希结果与公开输出匹配。

![95dec137-6b29-4ba3-b97f-5bae0c59de31](E:\Desktop\wangan\2025\project3\readme.assets\95dec137-6b29-4ba3-b97f-5bae0c59de31.png)