import cv2
import numpy as np
import os

BLOCK_SIZE = 8
ALPHA = 15  # 水印强度

# --- 工具函数 ---
def string_to_bits(s):
    return [int(bit) for char in s for bit in bin(ord(char))[2:].zfill(8)]

def bits_to_string(bits):
    chars = []
    for b in range(0, len(bits), 8):
        byte = bits[b:b+8]
        if len(byte) < 8:
            break
        chars.append(chr(int(''.join(map(str, byte)), 2)))
    return ''.join(chars)

def calculate_ber(bits1, bits2):
    return sum(b1 != b2 for b1, b2 in zip(bits1, bits2)) / len(bits1)

# --- 彩色图水印嵌入 ---
def embed_watermark_color(image_path, watermark_text, output_path="watermarked_output.png"):
    img_color = cv2.imread(image_path)
    if img_color is None:
        print(f"读取图像失败: {image_path}")
        return

    img_ycc = cv2.cvtColor(img_color, cv2.COLOR_BGR2YCrCb)
    Y, Cr, Cb = cv2.split(img_ycc)

    h, w = Y.shape
    Y = Y[:h - h % BLOCK_SIZE, :w - w % BLOCK_SIZE]

    wm_bits = string_to_bits(watermark_text)
    bit_idx = 0
    watermarked_Y = Y.astype(np.float32).copy()

    for i in range(0, Y.shape[0], BLOCK_SIZE):
        for j in range(0, Y.shape[1], BLOCK_SIZE):
            if bit_idx >= len(wm_bits):
                break
            block = watermarked_Y[i:i+BLOCK_SIZE, j:j+BLOCK_SIZE]
            dct_block = cv2.dct(block)
            dct_block[4, 3] += ALPHA if wm_bits[bit_idx] else -ALPHA
            watermarked_Y[i:i+BLOCK_SIZE, j:j+BLOCK_SIZE] = cv2.idct(dct_block)
            bit_idx += 1
        if bit_idx >= len(wm_bits):
            break

    watermarked_Y = np.clip(watermarked_Y, 0, 255).astype(np.uint8)
    watermarked_ycc = cv2.merge([watermarked_Y, Cr, Cb])
    watermarked_bgr = cv2.cvtColor(watermarked_ycc, cv2.COLOR_YCrCb2BGR)
    cv2.imwrite(output_path, watermarked_bgr)
    print(f"嵌入完成: {output_path}")
    return len(wm_bits)

# --- 彩色图水印提取 ---
def extract_watermark_color(image_path, wm_bit_length):
    img_color = cv2.imread(image_path)
    if img_color is None:
        print(f"读取图像失败: {image_path}")
        return "", []

    img_ycc = cv2.cvtColor(img_color, cv2.COLOR_BGR2YCrCb)
    Y, _, _ = cv2.split(img_ycc)

    h, w = Y.shape
    Y = Y[:h - h % BLOCK_SIZE, :w - w % BLOCK_SIZE]

    bits = []
    bit_idx = 0
    for i in range(0, Y.shape[0], BLOCK_SIZE):
        for j in range(0, Y.shape[1], BLOCK_SIZE):
            if bit_idx >= wm_bit_length:
                break
            block = Y[i:i+BLOCK_SIZE, j:j+BLOCK_SIZE].astype(np.float32)
            dct_block = cv2.dct(block)
            bits.append(1 if dct_block[4, 3] > 0 else 0)
            bit_idx += 1
        if bit_idx >= wm_bit_length:
            break

    return bits_to_string(bits), bits

# --- 鲁棒性测试 ---
def robustness_test_color(original_bits, wm_bit_length, watermarked_path):
    img = cv2.imread(watermarked_path)
    if img is None:
        print("鲁棒性测试失败：无法读取水印图像")
        return

    tests = []

    # 1. 翻转
    tests.append(("水平翻转", cv2.flip(img, 1)))
    tests.append(("垂直翻转", cv2.flip(img, 0)))

    # 2. 平移
    M = np.float32([[1, 0, 20], [0, 1, 20]])
    shifted = cv2.warpAffine(img, M, (img.shape[1], img.shape[0]))
    tests.append(("平移(20,20)", shifted))

    # 3. 裁剪中心区域
    h, w = img.shape[:2]
    crop = img[h//4:h*3//4, w//4:w*3//4]
    crop = cv2.resize(crop, (w, h))
    tests.append(("中心裁剪+缩放", crop))

    # 4. 对比度调节
    contrast = cv2.convertScaleAbs(img, alpha=1.5, beta=0)
    tests.append(("增加对比度", contrast))

    # 5. JPEG 压缩
    cv2.imwrite("temp.jpg", img, [int(cv2.IMWRITE_JPEG_QUALITY), 50])
    jpeg_img = cv2.imread("temp.jpg")
    os.remove("temp.jpg")
    tests.append(("JPEG压缩(质量50%)", jpeg_img))

    # 6. 模糊
    blur = cv2.GaussianBlur(img, (5,5), 0)
    tests.append(("高斯模糊", blur))

    # 7. 噪声
    noise = np.random.normal(0, 15, img.shape).astype(np.uint8)
    noisy = cv2.add(img, noise)
    tests.append(("加高斯噪声", noisy))

    print("\n--- 鲁棒性测试 ---")
    for name, attacked_img in tests:
        temp_path = "temp_attack.png"
        cv2.imwrite(temp_path, attacked_img)
        extracted_text, extracted_bits = extract_watermark_color(temp_path, wm_bit_length)
        ber = calculate_ber(original_bits, extracted_bits)
        print(f"{name:<16s} | 提取: {extracted_text:<30s} | BER: {ber:.4f}")
    if os.path.exists("temp_attack.png"):
        os.remove("temp_attack.png")


if __name__ == "__main__":
    input_image = "image.png"  # 原始图片路径
    watermark_text = "The sun that will eventually rise"

    wm_len = embed_watermark_color(input_image, watermark_text, "watermarked_output.png")

    extracted_text, extracted_bits = extract_watermark_color("watermarked_output.png", wm_len)
    print(f"\n原始水印: {watermark_text}")
    print(f"提取水印: {extracted_text}")
    original_bits = string_to_bits(watermark_text)
    print(f"BER: {calculate_ber(original_bits, extracted_bits):.4f}")

    # 进行鲁棒性测试
    robustness_test_color(original_bits, wm_len, "watermarked_output.png")
