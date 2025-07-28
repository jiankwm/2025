# 2025网络空间安全创新实践
本项目记录了2025网络空间安全创新实践课的任务作业，具体每个任务的说明文档见各个项目的文件夹内。

本课程实验由本人一人完成。

下面是各个项目题目，具体代码和配套文档见项目文件夹。

Project 1:SM4的软件实现和优化

Project 2: 编程实现图片水印嵌入和提取（可依托开源项目二次开发），并进行鲁棒性测试，包括不限于翻转、平移、截取、调对比度等

project 3: 用circom实现poseidon2哈希算法的电路
要求： 
1) poseidon2哈希算法参数参考参考文档1的Table1，用(n,t,d)=(256,3,5)或(256,2,5)
2）电路的公开输入用poseidon2哈希值，隐私输入为哈希原象，哈希算法的输入只考虑一个block即可。
3) 用Groth16算法生成证明
参考文档：
1. poseidon2哈希算法https://eprint.iacr.org/2023/323.pdf
2. circom说明文档https://docs.circom.io/
3. circom电路样例 https://github.com/iden3/circomlib

Project 4: sm3 的软件实现与优化 

Project 5: sm2 的软件实现优化 考虑到SM2用C 语言来做比较复杂，用python来做 sm2的 基础实现以及各种算法的改进尝试

Project 6:  来自报告  google password checkup，参考论文 https://eprint.iacr.org/2019/723.pdf 的 section 3.1 ，也即 Figure 2 中展示的协议，尝试实现该协议，（编程语言不限）。
