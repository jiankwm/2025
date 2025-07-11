# Jacobian 优化版
p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
Gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
Gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
n =  0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123

G = (Gx, Gy)

def inverse_mod(k, p):
    return pow(k, -1, p)

# ---------- Jacobian 坐标核心函数 ----------

def point_to_jacobian(P):
    x, y = P
    return (x, y, 1)

def jacobian_to_affine(P):
    X, Y, Z = P
    if Z == 0:
        return (0, 0)
    Z_inv = inverse_mod(Z, p)
    Z2_inv = pow(Z_inv, 2, p)
    Z3_inv = (Z2_inv * Z_inv) % p
    x = (X * Z2_inv) % p
    y = (Y * Z3_inv) % p
    return (x, y)

def jacobian_add(P, Q):
    if P[2] == 0:
        return Q
    if Q[2] == 0:
        return P

    X1, Y1, Z1 = P
    X2, Y2, Z2 = Q

    Z1Z1 = pow(Z1, 2, p)
    Z2Z2 = pow(Z2, 2, p)
    U1 = (X1 * Z2Z2) % p
    U2 = (X2 * Z1Z1) % p
    S1 = (Y1 * Z2 * Z2Z2) % p
    S2 = (Y2 * Z1 * Z1Z1) % p

    if U1 == U2:
        if S1 != S2:
            return (1, 1, 0)  # Infinity
        else:
            return jacobian_double(P)

    H = (U2 - U1) % p
    R = (S2 - S1) % p
    H2 = (H * H) % p
    H3 = (H * H2) % p
    U1H2 = (U1 * H2) % p
    X3 = (R * R - H3 - 2 * U1H2) % p
    Y3 = (R * (U1H2 - X3) - S1 * H3) % p
    Z3 = (H * Z1 * Z2) % p
    return (X3, Y3, Z3)

def jacobian_double(P):
    X1, Y1, Z1 = P
    A = pow(X1, 2, p)
    B = pow(Y1, 2, p)
    C = pow(B, 2, p)
    D = (2 * ((X1 + B) ** 2 - A - C)) % p
    E = (3 * A + a * pow(Z1, 4, p)) % p
    F = pow(E, 2, p)
    X3 = (F - 2 * D) % p
    Y3 = (E * (D - X3) - 8 * C) % p
    Z3 = (2 * Y1 * Z1) % p
    return (X3, Y3, Z3)

def point_mul(k, P):
    R = (1, 1, 0)  # Point at infinity in Jacobian
    Q = point_to_jacobian(P)
    while k > 0:
        if k & 1:
            R = jacobian_add(R, Q)
        Q = jacobian_double(Q)
        k >>= 1
    return jacobian_to_affine(R)
