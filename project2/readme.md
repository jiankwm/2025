## Invisible Watermarking System (DCT-based)

任务基于 **DCT（离散余弦变换）** 和 **YCrCb 色彩空间** 实现了对彩色图像的**不可见数字水印（Invisible Watermark）**的嵌入与提取，并提供了完整的鲁棒性攻击测试。

------

###  功能特点

- 支持在彩色图像中嵌入字符串水印（不可见）；
- 嵌入位置基于图像亮度通道（Y）频域的中频系数；
- 输出图像保持为彩色，视觉效果几乎不变；
-  支持水印提取与比特错误率（BER）计算；
-  提供多种鲁棒性攻击模拟：翻转、平移、裁剪、模糊、对比度、压缩、加噪声等。

------

###  使用方法

#### 1. 安装依赖

```
pip install opencv-python numpy
```

#### 2. 运行主程序

将你要嵌入水印的彩色图片命名为 `image.png` 或修改代码路径。

```
python watermark.py
```

#### 3. 输出说明

- 生成文件：
  - `watermarked_output.png`：嵌入水印后的彩色图像
- 控制台输出：
  - 原始水印内容与提取结果
  - 初始比特错误率（BER）
  - 各类攻击下的提取结果与鲁棒性评估

------

###  鲁棒性测试支持

自动模拟以下图像攻击并提取水印，报告比特错误率（BER）：

| 攻击类型   | 支持情况 |
| ---------- | -------- |
| 水平翻转   | 支持     |
| 垂直翻转   | 支持     |
| 平移扰动   | 支持     |
| 裁剪缩放   | 支持     |
| 对比度变化 | 支持     |
| JPEG 压缩  | 支持     |
| 高斯模糊   | 支持     |
| 加噪声     | 支持     |



------

###  示例

![6d982f7d-5bcd-461e-9ef3-7814a4646924](E:\Desktop\wangan\2025\project2\readme.assets\6d982f7d-5bcd-461e-9ef3-7814a4646924.png)

#### 原始图片：

![5e1bc0b6-0821-46ef-817e-137e245be74a](E:\Desktop\wangan\2025\project2\readme.assets\5e1bc0b6-0821-46ef-817e-137e245be74a.png)

#### 输出图片：

嵌入水印后无明显可视差异：

![7b4dbb49-3064-4978-ac85-1fae77feb167](E:\Desktop\wangan\2025\project2\readme.assets\7b4dbb49-3064-4978-ac85-1fae77feb167.png)

![be2bffbe-8979-4bf1-ac10-5a97391d30e7](E:\Desktop\wangan\2025\project2\readme.assets\be2bffbe-8979-4bf1-ac10-5a97391d30e7.png)

------

###  项目结构

```
bash复制编辑watermark.py          # 主程序：嵌入、提取、水印测试
image.png             # 原始彩色图像（自备）
watermarked_output.png # 生成的彩色水印图像
```

------

###  理论基础

- 离散余弦变换（DCT）在图像压缩和水印嵌入中广泛使用；
- YCrCb 色彩空间将亮度和色度分离，有助于在人眼敏感的 Y 通道中嵌入水印；
- 使用中频系数嵌入数据，可平衡鲁棒性与不可见性。

------

。