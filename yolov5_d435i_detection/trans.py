import numpy as np


def estimate_transformation(P, P_prime):
    """
    计算从 P 到 P_prime 的刚体变换矩阵 (3D)
    :param P: 3×N 的 numpy 数组，表示点在原坐标系中的坐标
    :param P_prime: 3×N 的 numpy 数组，表示点在目标坐标系中的坐标
    :return: 旋转矩阵 R (3×3) 和 平移向量 t (3×1)
    """
    # 计算质心
    c_P = np.mean(P, axis=1, keepdims=True)
    c_P_prime = np.mean(P_prime, axis=1, keepdims=True)

    # 去质心化
    Q = P - c_P
    Q_prime = P_prime - c_P_prime

    # 计算协方差矩阵
    H = Q @ Q_prime.T

    # SVD 分解
    U, S, Vt = np.linalg.svd(H)

    # 计算旋转矩阵 R
    R = Vt.T @ U.T

    # 确保 R 是合法的旋转矩阵，避免反射
    if np.linalg.det(R) < 0:
        Vt[-1, :] *= -1
        R = Vt.T @ U.T

    # 计算平移 t
    t = c_P_prime - R @ c_P

    return R, t.flatten()


if __name__ == '__main__':
    # 示例数据
    # P = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]]).T  # 3×N
    # P_prime = np.array([[2, 3, 4], [5, 6, 7], [8, 9, 10]]).T  # 3×N

    points_original = np.array([
        [-44, -60, 355],
        [-90, -89, 294],
        [-81, -66, 227]
        # [-59, -48, 245]
    ]).T
    points_target = np.array([
        [303.2, 24.3, 112.4],
        [289.9, -28.7, 83.4],
        [249, -49.6, 103.3]
        # [237.7, -25.7, 123.7]
    ]).T

    points_camera = np.array([
        [50.791, -68.359, 387],
        [131.38, -52.701, 420],
        [129.91, -85.649, 436]
    ]).T
    points_robot = np.array([
        [314.88, 200.78, 7.45],
        [291.95, 118.94, 27.12],
        [277.91, 116.12, -5.24]
    ]).T

    # R, t = estimate_transformation(P, P_prime)
    R, t = estimate_transformation(points_original, points_target)
    # R, t = estimate_transformation(points_camera, points_robot)

    print("旋转矩阵 R:")
    print(R)
    print("\n平移向量 t:")
    print(t)

    # print("\ndiff:")
    # print(points_target - (R.dot(points_original) + t))
    # print(points_robot - (R.dot(points_camera) + t))

    pos = np.array([-81, -66, 227])
    print("\n转换后的坐标：")
    print(R.dot(pos) + t)
