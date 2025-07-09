pragma circom 2.2.2;

// S-box 函数：x^d (这里 d=5)
function sbox(x) {
    return x * x * x * x * x;
}

// --- 简化的 MDS 矩阵和轮常数函数 (仅用于简单实验，无密码学安全性) ---
// 这些函数现在在全局作用域，可以直接被任何模板或函数调用
function getMDSMatrixSimple(T_STATE_WIDTH) {
    var mds[T_STATE_WIDTH][T_STATE_WIDTH];
    // 使用传入的 T_STATE_WIDTH 来定义矩阵大小
    for (var i = 0; i < T_STATE_WIDTH; i++) {
        for (var j = 0; j < T_STATE_WIDTH; j++) {
            if (i == j) {
                mds[i][j] = 2;
            } else {
                mds[i][j] = 1;
            }
        }
    }
    return mds;
}

function getRoundConstantsSimple(TOTAL_ROUNDS, T_STATE_WIDTH) {
    var rc[TOTAL_ROUNDS][T_STATE_WIDTH];
    for (var r = 0; r < TOTAL_ROUNDS; r++) {
        for (var i = 0; i < T_STATE_WIDTH; i++) {
            rc[r][i] = (r * T_STATE_WIDTH + i + 1); // 示例轮常数
        }
    }
    return rc;
}

// --- Poseidon2Round 子组件 ---
// 实现 Poseidon2 的单轮逻辑
template Poseidon2Round(roundType, roundIdx, T_STATE_WIDTH, TOTAL_ROUNDS) { // 接收参数
    signal input in_state[T_STATE_WIDTH];
    signal output out_state[T_STATE_WIDTH];

    // 在需要使用时，调用函数获取MDS和RC矩阵，传入T_STATE_WIDTH和TOTAL_ROUNDS
    var MDS = getMDSMatrixSimple(T_STATE_WIDTH);
    var roundConstants = getRoundConstantsSimple(TOTAL_ROUNDS, T_STATE_WIDTH);

    // 1. Add Round Constant
    signal state_after_rc[T_STATE_WIDTH];
    for (var i = 0; i < T_STATE_WIDTH; i++) {
        state_after_rc[i] <== in_state[i] + roundConstants[roundIdx][i];
    }

    // 2. S-box Application
    signal state_after_sbox[T_STATE_WIDTH];
    if (roundType == 0) { // Full S-box
        for (var i = 0; i < T_STATE_WIDTH; i++) {
            state_after_sbox[i] <== sbox(state_after_rc[i]);
        }
    } else { // Partial S-box - only on the first element
        state_after_sbox[0] <== sbox(state_after_rc[0]);
        for (var i = 1; i < T_STATE_WIDTH; i++) { // Remaining elements remain unchanged
            state_after_sbox[i] <== state_after_rc[i];
        }
    }

    // 3. MDS Matrix Multiplication
    for (var i = 0; i < T_STATE_WIDTH; i++) {
        out_state[i] <== 0; // Initialize for summation
        for (var j = 0; j < T_STATE_WIDTH; j++) {
            out_state[i] += MDS[i][j] * state_after_sbox[j];
        }
    }
}

// --- Poseidon2Hash 主组件 ---
// 实现整个 Poseidon2 哈希算法
template Poseidon2Hash() {
    // 将参数定义在模板内部
    var RF_FULL_ROUNDS = 8;
    var PARTIAL_ROUNDS = 56;
    var TOTAL_ROUNDS = RF_FULL_ROUNDS + PARTIAL_ROUNDS; // 8 + 56 = 64 轮
    var T_STATE_WIDTH = 3; // t=3

    signal input in_private[T_STATE_WIDTH - 1]; // 私有输入，包含两个元素 (t-1)
    signal  input public hash_output; // 公开输入：哈希值

    signal current_state[T_STATE_WIDTH];
    // 假设输入是哈希原象的两个部分，第三个元素填充为0
    current_state[0] <== in_private[0];
    current_state[1] <== in_private[1];
    current_state[2] <== 0; // 填充或容量元素

    component rounds[TOTAL_ROUNDS];

    for (var r = 0; r < TOTAL_ROUNDS; r++) {
        var roundType = (r < (RF_FULL_ROUNDS / 2)) || (r >= TOTAL_ROUNDS - (RF_FULL_ROUNDS / 2)) ? 0 : 1;
        
        // 实例化 Poseidon2Round 组件，传入所需的参数
        rounds[r] = Poseidon2Round(roundType, r, T_STATE_WIDTH, TOTAL_ROUNDS);
        
        for (var i = 0; i < T_STATE_WIDTH; i++) {
            rounds[r].in_state[i] <== current_state[i];
        }

        for (var i = 0; i < T_STATE_WIDTH; i++) {
            current_state[i] <== rounds[r].out_state[i];
        }
    }

    // 将最终哈希结果的第一个元素与公开输入哈希值进行约束
    hash_output === current_state[0];
}

// 定义主组件
component main = Poseidon2Hash();