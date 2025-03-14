import numpy as np

# 示例数据：4 对点 (P_C in camera frame, P_B in robot base frame)
# 相机坐标
P_C = np.array([
    [-44, -60, 355],
    [-90, -89, 294],
    [-81, -66, 227],
    [-59, -48, 245]
])
# 机械臂坐标
P_B = np.array([
    [303.2, 24.3, 112.4],
    [289.9, -28.7, 83.4],
    [249, -49.6, 103.3],
    [237.7, -25.7, 123.7]
])

# 计算质心
C_C = np.mean(P_C, axis=0)
C_B = np.mean(P_B, axis=0)

# 去中心化
P_C_prime = P_C - C_C
P_B_prime = P_B - C_B

# 计算 H 矩阵
H = P_C_prime.T @ P_B_prime

# SVD 分解
U, S, Vt = np.linalg.svd(H)
R = Vt.T @ U.T

# 确保旋转矩阵的正交性
if np.linalg.det(R) < 0:
    Vt[-1, :] *= -1
    R = Vt.T @ U.T

# 计算平移向量
t = C_B - R @ C_C

# 构建 4x4 变换矩阵
T_B_C = np.eye(4)
T_B_C[:3, :3] = R
T_B_C[:3, 3] = t

print("Transformation Matrix ^B T_C:\n", T_B_C)
