#ifndef NS_GAME_MATH_H
#define NS_GAME_MATH_H

#include "ns_common.h"
#include "ns_math.h"

union v2
{
    float Components[2];

    struct
    {
        float X, Y;
    };

    inline float &operator[](int ComponentIdx);
    inline void operator+=(v2 A);
    inline void operator-=(v2 A);
    inline void operator*=(float A);
};

union v3
{
    float Components[3];

    struct
    {
        float X, Y, Z;
    };

    inline float &operator[](int ComponentIdx);
    inline void operator+=(v3 A);
    inline void operator-=(v3 A);
};

union v4
{
    float Components[4];

    struct
    {
        float X, Y, Z, W;
    };

    struct
    {
        float R, G, B, A;
    };

    struct
    {
        v3 XYZ;
        float _Ignored0;
    };

    inline float &operator[](int ComponentIdx);
};

struct quaternion
{
    union
    {
        v3 Axis;

        struct
        {
            float X, Y, Z;
        };
    };

    float W;
};

struct ray2
{
    v2 Pos;
    v2 Dir;
};

struct ray3
{
    v3 Pos;
    v3 Dir;
};

union mat4
{
    float E1[16];
    float E2[4][4];

    struct
    {
        float m00, m01, m02, m03;
        float m10, m11, m12, m13;
        float m20, m21, m22, m23;
        float m30, m31, m32, m33;
    };

    float &operator[](int Element1Idx);
};

union line2
{
    struct
    {
        v2 P1, P2;
    };
    v2 Verts[2];

    line2 operator-();
    void operator+=(v2);
    void operator-=(v2);
};

struct plane
{
    v3 Position;
    v3 Normal;
};

struct rect2
{
    v2 Min, Max;

    void operator-=(v2);
};

struct quad2
{
    v2 BottomLeft;
    v2 BottomRight;
    v2 TopRight;
    v2 TopLeft;
};

#define RECT_BOTTOMLEFT 0
#define RECT_BOTTOMRIGHT 1
#define RECT_TOPRIGHT 2
#define RECT_TOPLEFT 3
global uint32_t GlobalRectIndices[] = {
    RECT_BOTTOMLEFT, RECT_BOTTOMRIGHT, RECT_TOPRIGHT,
    RECT_BOTTOMLEFT, RECT_TOPRIGHT, RECT_TOPLEFT,
};

struct pentagon
{
    v2 P1, P2, P3, P4, P5;
};

/* A quad in 3D */
union quad2_3d
{
    v3 Corners[4];

    struct
    {
        v3 BottomLeft;
        v3 BottomRight;
        v3 TopRight;
        v3 TopLeft;
    };

    v3 operator[](int CornerIdx);
};

union tri2
{
    struct
    {
        v2 P1, P2, P3;
    };
    v2 Verts[3];

    inline void operator*=(float);
    inline void operator+=(v2 A);
    inline void operator-=(v2 A);
};


/* Vectors */

void
Print(v2 V)
{
    Printf("%f, %f\n", V.X, V.Y);
}

inline v2
V2(float X, float Y)
{
    v2 Result = {X, Y};
    return Result;
}

inline v2
V2(int X, int Y)
{
    v2 Result = V2((float)X, (float)Y);
    return Result;
}

inline v3
V3(float X, float Y, float Z)
{
    v3 Result = {X, Y, Z};
    return Result;
}

inline v4
V4(float X, float Y, float Z, float W)
{
    v4 Result = {X, Y, Z, W};
    return Result;
}

inline float &
v2::operator[](int ComponentIdx)
{
    return this->Components[ComponentIdx];
}

inline float &
v3::operator[](int ComponentIdx)
{
    return this->Components[ComponentIdx];
}

inline float &
v4::operator[](int ComponentIdx)
{
    return this->Components[ComponentIdx];
}

inline v2
operator-(v2 V)
{
    v2 Result = {-V.X, -V.Y};
    return Result;
}

inline v3
operator-(v3 V)
{
    v3 Result = {-V.X, -V.Y, -V.Z};
    return Result;
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result = {A.X + B.X, A.Y + B.Y};
    return Result;
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result = {A.X + B.X, A.Y + B.Y, A.Z + B.Z};
    return Result;
}

inline void
v2::operator+=(v2 A)
{
    this->X += A.X;
    this->Y += A.Y;
}

inline void
v3::operator+=(v3 A)
{
    this->X += A.X;
    this->Y += A.Y;
    this->Z += A.Z;
}

inline void
v2::operator*=(float A)
{
    this->X *= A;
    this->Y *= A;
}

inline void
v2::operator-=(v2 A)
{
    this->X -= A.X;
    this->Y -= A.Y;
}

inline void
v3::operator-=(v3 A)
{
    this->X -= A.X;
    this->Y -= A.Y;
    this->Z -= A.Z;
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result = {A.X - B.X, A.Y - B.Y};
    return Result;
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result = {A.X - B.X, A.Y - B.Y, A.Z - B.Z};
    return Result;
}

inline v2
operator*(float S, v2 V)
{
    v2 Result = {S*V.X, S*V.Y};
    return Result;
}

inline v3
operator*(float S, v3 V)
{
    v3 Result = {S*V.X, S*V.Y, S*V.Z};
    return Result;
}

inline v3
operator/(v3 V, float S)
{
    v3 Result = (1.0f/S) * V;
    return Result;
}

inline bool
operator==(v2 A, v2 B)
{
    bool Result = (A.X == B.X && 
                   A.Y == B.Y);
    return Result;
}

inline bool
operator!=(v2 A, v2 B)
{
    bool Result = !(A == B);
    return Result;
}

bool
AreEqualTolerance(v2 A, v2 B)
{
    bool Result = IsWithinTolerance(A.X, B.X) && IsWithinTolerance(A.Y, B.Y);
    return Result;
}

inline bool
IsZero(v2 A)
{
    bool Result = (A.X == 0.0f && A.Y == 0.0f);
    return Result;
}

inline v3
DoCross(v3 A, v3 B)
{
    v3 Result = {
        A.Y*B.Z - A.Z*B.Y,
        A.Z*B.X - A.X*B.Z,
        A.X*B.Y - A.Y*B.X
    };
    return Result;
}

/* Returns sign of Z-coord of cross product. */
inline float
DoCross(v2 A, v2 B)
{
    v3 CrossProduct = DoCross(V3(A.X, A.Y, 0.0f), V3(B.X, B.Y, 0.0f));
    float Result = CrossProduct.Z;
    return Result;
}

inline float
GetLength(v2 *V)
{
    float Result = Sqrt(V->X*V->X + V->Y*V->Y);
    return Result;
}

inline float
GetLength(v2 V)
{
    float Result = GetLength(&V);
    return Result;
}

inline float
GetLengthSq(v2 V)
{
    float Length = GetLength(&V);
    float Result = Length*Length;
    return Result;
}

inline float
GetDistance(v2 A, v2 B)
{
    float Result = GetLength(B - A);
    return Result;
}

inline void
Normalize(v2 *V)
{
    float Length = GetLength(V);
    V->X /= Length;
    V->Y /= Length;
}

inline void
Normalize(v3 *V)
{
    float Length = Sqrt(V->X*V->X + V->Y*V->Y + V->Z*V->Z);
    V->X /= Length;
    V->Y /= Length;
    V->Z /= Length;
}

inline v2
Normalize(v2 V)
{
    Normalize(&V);
    return V;
}

inline v3
Normalize(v3 V)
{
    Normalize(&V);
    return V;
}

inline float
GetDot(v2 A, v2 B)
{
    float Result = A.X*B.X + A.Y*B.Y;
    return Result;
}

inline float
GetDot(v3 A, v3 B)
{
    float Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

enum angle_direction
{
    AngleDirection_None, AngleDirection_CW, AngleDirection_CCW
};

angle_direction GetDirectionBetween(v2 A, v2 B)
{
    float CrossProduct = DoCross(A, B);
    Assert(CrossProduct != 0.0f);
    angle_direction Result = CrossProduct < 0.0f ? AngleDirection_CW : AngleDirection_CCW;
    return Result;
}

v2 GetDirection(line2);
angle_direction GetDirectionBetween(line2 A, line2 B)
{
    v2 ADir = GetDirection(A);
    v2 BDir = GetDirection(B);
    angle_direction Result = GetDirectionBetween(ADir, BDir);
    return Result;
}

float GetAngleBetween(v2 A, v2 B, angle_direction DesiredAngleDirection = AngleDirection_None)
{
    /* Assert. */
    {
        float ALengthSq = GetLengthSq(A);
        float BLengthSq = GetLengthSq(B);
        Assert(IsWithinTolerance(ALengthSq, 1.0f) && IsWithinTolerance(BLengthSq, 1.0f));
    }

    float AngleBetween;
    if (A == B)
    {
        AngleBetween = 0.0f;
    }
    else if (A == -B)
    {
        AngleBetween = M_PI;
    }
    else
    {
        AngleBetween = Acos(GetDot(A, B));
        if (DesiredAngleDirection != AngleDirection_None)
        {
            angle_direction AngleDirection = GetDirectionBetween(A, B);
            if (AngleDirection != DesiredAngleDirection)
            {
                AngleBetween = M_2PI - AngleBetween;
            }
        }
    }
    return AngleBetween;
}

inline float
GetAngleBetween(v3 A, v3 B)
{
    float Result = ToDegrees(Acos(GetDot(A, B)));
    return Result;
}

inline v2
Lerp(v2 A, float t, v2 B)
{
    v2 Result = A + t*(B - A);
    return Result;
}

inline v3
Lerp(v3 A, v3 B, float t)
{
    v3 Result = A + t*(B - A);
    return Result;
}

inline v2
Rotate(v2 V, float AngleDegrees)
{
    float AngleRadians = ToRadians(AngleDegrees);
    float sin = Sin(AngleRadians);
    float cos = Cos(AngleRadians);

    v2 Result = {
        V.X*cos - V.Y*sin,
        V.X*sin + V.Y*cos,
    };
    return Result;
}

/* Rotates V around P. */
inline void
RotateAround(v2 *V, v2 P, float AngleDegrees)
{ 
    float AngleRadians = ToRadians(AngleDegrees);
    float sin = Sin(AngleRadians);
    float cos = Cos(AngleRadians);

    *V -= P;
    float TmpX = V->X;
    V->X = V->X * cos - V->Y * sin;
    V->Y = TmpX * sin + V->Y * cos;
    *V += P;
}

/* Returns the quaternion that rotates A to B. */
inline quaternion
GetRotationBetween(v3 A, v3 B)
{
    v3 RotAxis = DoCross(A, B);
    float RotHalfAngle = GetAngleBetween(A, B) / 2.0f;
    quaternion Result = {Sin(RotHalfAngle)*RotAxis, Cos(RotHalfAngle)};
    return Result;
}

float GetSignedArea(v2, v2, v2);
bool CheckCollinear(v2 A, v2 B, v2 C)
{
    float Area = GetSignedArea(A, B, C);
    bool Result = (Area == 0.0f);
    return Result;
}

/* Quaternions */

inline quaternion
Conjugate(quaternion Quat)
{
    quaternion Result = {-Quat.Axis, Quat.W};
    return Result;
}

inline quaternion
operator*(quaternion A, quaternion B)
{
    v3 ResultAxis = DoCross(A.Axis, B.Axis) + A.W*B.Axis + B.W*A.Axis;
    float ResultW = A.W*B.W - GetDot(A.Axis, B.Axis);

    quaternion Result = {ResultAxis, ResultW};
    return Result;
}

inline void
Rotate(v3 *Vector, quaternion RotQuat)
{
    quaternion VectorQuat = {*Vector, 1.0f};
    quaternion ResultQuat = RotQuat * VectorQuat * Conjugate(RotQuat);

    Vector->X = ResultQuat.X;
    Vector->Y = ResultQuat.Y;
    Vector->Z = ResultQuat.Z;
}

inline v3
Rotate(v3 Vector, quaternion RotQuat)
{
    Rotate(&Vector, RotQuat);
    return Vector;
}

inline void
Rotate(v3 *Vector, v3 Axis, float RotationDegrees)
{
    float RotationRadians = ToRadians(RotationDegrees);
    float SinHalfAngle = Sin(RotationRadians / 2);
    float CosHalfAngle = Cos(RotationRadians / 2);

    quaternion RotQuat = {SinHalfAngle*Axis, CosHalfAngle};
    Rotate(Vector, RotQuat);
}

inline v2
Bezier(v2 A, v2 B, v2 C, float t)
{
    float t2 = t*t;
    float mt = 1 - t;
    float mt2 = mt*mt;
    v2 Result = mt2*A + 2*mt*t*B + t2*C;
    return Result;
}

inline v2
Bezier(v2 A, v2 B, v2 C, v2 D, float t)
{
    float t2 = t*t;
    float t3 = t*t2;
    float mt = 1 - t;
    float mt2 = mt*mt;
    float mt3 = mt*mt2;
    v2 Result = mt3*A + 3*mt2*t*B + 3*mt*t2*C + t3*D;
    return Result;
}

inline v2
Bezier(v2 A, v2 B, v2 C, v2 D, v2 E, float t)
{
    float t2 = t*t;
    float t3 = t*t2;
    float t4 = t*t3;
    float mt = 1 - t;
    float mt2 = mt*mt;
    float mt3 = mt*mt2;
    float mt4 = mt*mt3;
    v2 Result = mt4*A + 4*mt3*t*B + 6*mt2*t2*C + 4*mt*t3*D + t4*E;
    return Result;
}

/* Matrices */

inline float &
mat4::operator[](int ElementIdx)
{
    return this->E1[ElementIdx];
}

inline mat4
operator*(mat4 Left, mat4 Right)
{
    mat4 Result = {};

    for(int ResultRow = 0; ResultRow < 4; ResultRow++)
    {
        for(int ResultCol = 0; ResultCol < 4; ResultCol++)
        {
            for(int I = 0; I < 4; I++)
            {
                Result.E2[ResultRow][ResultCol] += Right.E2[I][ResultCol] * Left.E2[ResultRow][I];
            }
        }
    }

    return Result;
}

inline v4
operator*(mat4 Matrix, v4 Vector)
{
    v4 Result = {};
    for(int Row = 0; Row < 4; Row++)
    {
        for(int Col = 0; Col < 4; Col++)
        {
            Result[Row] += Vector[Col]*Matrix.E2[Row][Col];
        }
    }

    return Result;
}

inline mat4 
CreateIdentity()
{
    mat4 Result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    return Result;
}

inline mat4
CreateProjection(float AspectRatio, float FieldOfView, float Near, float Far)
{
    float FieldOfViewRads = ToRadians(FieldOfView);
    float Top = Near * Tan(FieldOfViewRads);
    float Right = Top * AspectRatio;

    mat4 Result = {
        Near / Right, 0, 0, 0,
        0, Near / Top, 0, 0,
        0, 0, -(Far + Near) / (Far - Near), -2*Far*Near / (Far - Near),
        0, 0, -1, 0,
    };

    return Result;
}

inline void
Translate(mat4 *Matrix, v3 Translation)
{
    Matrix->m03 += Translation.X;
    Matrix->m13 += Translation.Y;
    Matrix->m23 += Translation.Z;
}

inline mat4
CreateTranslation(float X, float Y, float Z)
{
    mat4 Result = {
        1, 0, 0, X,
        0, 1, 0, Y,
        0, 0, 1, Z,
        0, 0, 0, 1
    };
    return Result;
}

inline mat4
CreateTranslation(v3 Translation)
{
    mat4 Result = CreateTranslation(Translation.X, Translation.Y, Translation.Z);
    return Result;
}

inline mat4
CreateView(v3 Right, v3 Up, v3 Forward, v3 Pos)
{
    mat4 Rotation = {
        Right.X, Right.Y, Right.Z, 0,
        Up.X, Up.Y, Up.Z, 0,
        -Forward.X, -Forward.Y, -Forward.Z, 0,
        0, 0, 0, 1
    };

    mat4 Result = Rotation * CreateTranslation(-Pos);
    return Result;
}

inline void
Transpose(mat4 *Matrix)
{
    for(int Row = 0; Row < 4; Row++)
    {
        for(int Col = 0; Col < 4; Col++)
        {
            Swap(&Matrix->E2[Row][Col], &Matrix->E2[Col][Row]);
        }
    }
}

inline void
Zero(mat4 *Matrix)
{
    for(int I = 0; I < 16; I++)
    {
        Matrix->E1[I] = 0.0f;
    }
}

/* Cut and pasted from stackoverflow. Seeing how it works without any
 * transposes, PRESUMABLY it assumes row-major ordering. */
mat4
Invert(mat4 Matrix)
{
    mat4 Result;

    Result[0] = Matrix[5]  * Matrix[10] * Matrix[15] - 
        Matrix[5]  * Matrix[11] * Matrix[14] - 
        Matrix[9]  * Matrix[6]  * Matrix[15] + 
        Matrix[9]  * Matrix[7]  * Matrix[14] +
        Matrix[13] * Matrix[6]  * Matrix[11] - 
        Matrix[13] * Matrix[7]  * Matrix[10];

    Result[4] = -Matrix[4]  * Matrix[10] * Matrix[15] + 
        Matrix[4]  * Matrix[11] * Matrix[14] + 
        Matrix[8]  * Matrix[6]  * Matrix[15] - 
        Matrix[8]  * Matrix[7]  * Matrix[14] - 
        Matrix[12] * Matrix[6]  * Matrix[11] + 
        Matrix[12] * Matrix[7]  * Matrix[10];

    Result[8] = Matrix[4]  * Matrix[9] * Matrix[15] - 
        Matrix[4]  * Matrix[11] * Matrix[13] - 
        Matrix[8]  * Matrix[5] * Matrix[15] + 
        Matrix[8]  * Matrix[7] * Matrix[13] + 
        Matrix[12] * Matrix[5] * Matrix[11] - 
        Matrix[12] * Matrix[7] * Matrix[9];

    Result[12] = -Matrix[4]  * Matrix[9] * Matrix[14] + 
        Matrix[4]  * Matrix[10] * Matrix[13] +
        Matrix[8]  * Matrix[5] * Matrix[14] - 
        Matrix[8]  * Matrix[6] * Matrix[13] - 
        Matrix[12] * Matrix[5] * Matrix[10] + 
        Matrix[12] * Matrix[6] * Matrix[9];

    Result[1] = -Matrix[1]  * Matrix[10] * Matrix[15] + 
        Matrix[1]  * Matrix[11] * Matrix[14] + 
        Matrix[9]  * Matrix[2] * Matrix[15] - 
        Matrix[9]  * Matrix[3] * Matrix[14] - 
        Matrix[13] * Matrix[2] * Matrix[11] + 
        Matrix[13] * Matrix[3] * Matrix[10];

    Result[5] = Matrix[0]  * Matrix[10] * Matrix[15] - 
        Matrix[0]  * Matrix[11] * Matrix[14] - 
        Matrix[8]  * Matrix[2] * Matrix[15] + 
        Matrix[8]  * Matrix[3] * Matrix[14] + 
        Matrix[12] * Matrix[2] * Matrix[11] - 
        Matrix[12] * Matrix[3] * Matrix[10];

    Result[9] = -Matrix[0]  * Matrix[9] * Matrix[15] + 
        Matrix[0]  * Matrix[11] * Matrix[13] + 
        Matrix[8]  * Matrix[1] * Matrix[15] - 
        Matrix[8]  * Matrix[3] * Matrix[13] - 
        Matrix[12] * Matrix[1] * Matrix[11] + 
        Matrix[12] * Matrix[3] * Matrix[9];

    Result[13] = Matrix[0]  * Matrix[9] * Matrix[14] - 
        Matrix[0]  * Matrix[10] * Matrix[13] - 
        Matrix[8]  * Matrix[1] * Matrix[14] + 
        Matrix[8]  * Matrix[2] * Matrix[13] + 
        Matrix[12] * Matrix[1] * Matrix[10] - 
        Matrix[12] * Matrix[2] * Matrix[9];

    Result[2] = Matrix[1]  * Matrix[6] * Matrix[15] - 
        Matrix[1]  * Matrix[7] * Matrix[14] - 
        Matrix[5]  * Matrix[2] * Matrix[15] + 
        Matrix[5]  * Matrix[3] * Matrix[14] + 
        Matrix[13] * Matrix[2] * Matrix[7] - 
        Matrix[13] * Matrix[3] * Matrix[6];

    Result[6] = -Matrix[0]  * Matrix[6] * Matrix[15] + 
        Matrix[0]  * Matrix[7] * Matrix[14] + 
        Matrix[4]  * Matrix[2] * Matrix[15] - 
        Matrix[4]  * Matrix[3] * Matrix[14] - 
        Matrix[12] * Matrix[2] * Matrix[7] + 
        Matrix[12] * Matrix[3] * Matrix[6];

    Result[10] = Matrix[0]  * Matrix[5] * Matrix[15] - 
        Matrix[0]  * Matrix[7] * Matrix[13] - 
        Matrix[4]  * Matrix[1] * Matrix[15] + 
        Matrix[4]  * Matrix[3] * Matrix[13] + 
        Matrix[12] * Matrix[1] * Matrix[7] - 
        Matrix[12] * Matrix[3] * Matrix[5];

    Result[14] = -Matrix[0]  * Matrix[5] * Matrix[14] + 
        Matrix[0]  * Matrix[6] * Matrix[13] + 
        Matrix[4]  * Matrix[1] * Matrix[14] - 
        Matrix[4]  * Matrix[2] * Matrix[13] - 
        Matrix[12] * Matrix[1] * Matrix[6] + 
        Matrix[12] * Matrix[2] * Matrix[5];

    Result[3] = -Matrix[1] * Matrix[6] * Matrix[11] + 
        Matrix[1] * Matrix[7] * Matrix[10] + 
        Matrix[5] * Matrix[2] * Matrix[11] - 
        Matrix[5] * Matrix[3] * Matrix[10] - 
        Matrix[9] * Matrix[2] * Matrix[7] + 
        Matrix[9] * Matrix[3] * Matrix[6];

    Result[7] = Matrix[0] * Matrix[6] * Matrix[11] - 
        Matrix[0] * Matrix[7] * Matrix[10] - 
        Matrix[4] * Matrix[2] * Matrix[11] + 
        Matrix[4] * Matrix[3] * Matrix[10] + 
        Matrix[8] * Matrix[2] * Matrix[7] - 
        Matrix[8] * Matrix[3] * Matrix[6];

    Result[11] = -Matrix[0] * Matrix[5] * Matrix[11] + 
        Matrix[0] * Matrix[7] * Matrix[9] + 
        Matrix[4] * Matrix[1] * Matrix[11] - 
        Matrix[4] * Matrix[3] * Matrix[9] - 
        Matrix[8] * Matrix[1] * Matrix[7] + 
        Matrix[8] * Matrix[3] * Matrix[5];

    Result[15] = Matrix[0] * Matrix[5] * Matrix[10] - 
        Matrix[0] * Matrix[6] * Matrix[9] - 
        Matrix[4] * Matrix[1] * Matrix[10] + 
        Matrix[4] * Matrix[2] * Matrix[9] + 
        Matrix[8] * Matrix[1] * Matrix[6] - 
        Matrix[8] * Matrix[2] * Matrix[5];

    float Determinant = Matrix[0] * Result[0] + Matrix[1] * Result[4] + Matrix[2] * Result[8] + Matrix[3] * Result[12];
    if(Determinant != 0.0f)
    {
        Determinant = 1.0f / Determinant;
        for(int I = 0; I < 16; I++)
        {
            Result[I] *= Determinant;
        }
    }
    else
    {
        Zero(&Result);
    }

    return Result;
}

/* Rays */

ray2
RAY2(v2 Pos, v2 Dir)
{
    ray2 Result = {Pos, Dir};
    return Result;
}

inline ray2
ToRay2(line2 Line, bool NormalizeDir = true)
{
    v2 Dir = Line.P2 - Line.P1;
    if (NormalizeDir)
    {
        Normalize(&Dir);
    }
    v2 Pos = Line.P1;

    ray2 Result = {Pos, Dir};
    return Result;
}

bool
IsValid(ray2 Ray)
{
    bool Result = !IsZero(Ray.Dir);
    return Result;
}

void
ReverseDirection(ray2 *Ray)
{
    Ray->Dir = -Ray->Dir;
}

/* Lines */

inline line2
LINE2(v2 P1, v2 P2)
{
    line2 Result = {P1, P2};
    return Result;
}

inline line2
LINE2(float X1, float Y1, float X2, float Y2)
{
    line2 Result = {X1, Y1, X2, Y2};
    return Result;
}

line2 line2::operator-()
{
    line2 Result = LINE2(this->P2, this->P1);
    return Result;
}

bool
operator==(line2 A, line2 B)
{
    /* For all intents and purposes, we compare both orderings. */
    return ((A.P1 == B.P1 && A.P2 == B.P2) ||
            (A.P1 == B.P2 && A.P2 == B.P1));
}

bool
operator!=(line2 A, line2 B)
{
    bool Result = !(A == B);
    return Result;
}

void
line2::operator+=(v2 V)
{
    this->P1 += V;
    this->P2 += V;
}

void
line2::operator-=(v2 V)
{
    this->P1 -= V;
    this->P2 -= V;
}

void
EnsureEndPointIsFirst(line2 *Line, v2 Endpoint)
{
    /* This point should be one of the endpoints. */
    Assert(Line->P1 == Endpoint || Line->P2 == Endpoint);
    if (Line->P1 != Endpoint)
    {
        v2 Tmp = Line->P1;
        Line->P1 = Line->P2;
        Line->P2 = Tmp;
    }
}

bool CheckContainsSameVertex(line2 L, v2 V)
{
    bool Result = L.P1 == V || L.P2 == V;
    return Result;
}

inline v2
Rotate90CCW(v2 V)
{
    v2 Result = {-V.Y, V.X};
    return Result;
}

/* Returns normal that is 90 degrees CCW to the line. */
inline v2
GetNormal(line2 Line)
{
    v2 OriginVector = Line.P2 - Line.P1;
    v2 Rotated = Rotate90CCW(OriginVector);
    v2 LineNormal = Normalize(Rotated);
    Assert(LineNormal.X == LineNormal.X && LineNormal.Y == LineNormal.Y);
    return LineNormal;
}

inline void
RotateAroundCenter(line2 *Line, float AngleDegrees)
{
    v2 LineCenter = Lerp(Line->P1, 0.5f, Line->P2);
    RotateAround(&Line->P1, LineCenter, AngleDegrees);
    RotateAround(&Line->P2, LineCenter, AngleDegrees);
}

inline line2
RotateAroundCenter(line2 Line, float AngleDegrees)
{
    RotateAroundCenter(&Line, AngleDegrees);
    return Line;
}

quad2
CreateLineQuad(line2 Line, float Width)
{
    Assert(Line.P1 != Line.P2);

    float HalfWidth = Width/2.0f;

    v2 Normal = GetNormal(Line);

    quad2 Result;
    Result.BottomLeft = Line.P1 - HalfWidth*Normal;
    Result.BottomRight = Line.P2 - HalfWidth*Normal;
    Result.TopRight = Line.P2 + HalfWidth*Normal;
    Result.TopLeft = Line.P1 + HalfWidth*Normal;

    return Result;
}

v2
GetPointAtPercentage(line2 Line, float Percentage)
{
    v2 Point = Lerp(Line.P1, Percentage, Line.P2);
    return Point;
}

/* Calculates the value of P projected onto the line. */
float
GetProjectedCoord(v2 P, line2 Line)
{
    v2 LineDir = Line.P2 - Line.P1;
    v2 RelP = P - Line.P1;

    float RelPDotLineDir = GetDot(RelP, LineDir);
    float LineDirLenSq = GetDot(LineDir, LineDir);

    float ProjectedCoord = RelPDotLineDir / LineDirLenSq;
    return ProjectedCoord;
}

/* Calculates point's distance to line. */
float
GetDistance(v2 P, line2 Line)
{
    float t = Clamp01(GetProjectedCoord(P, Line));
    v2 LineDir = Line.P2 - Line.P1;
    v2 ClosestPointOnLine = Line.P1 + t*LineDir;
    float DistanceToLine = GetLength(P - ClosestPointOnLine);
    return DistanceToLine;
}

v2
GetDirection(line2 Line)
{
    v2 Direction = Line.P2 - Line.P1;
    Normalize(&Direction);
    return Direction;
}

float GetLengthSq(line2 Line)
{
    v2 Difference = Line.P2 - Line.P1;
    float Result = GetLengthSq(Difference);
    return Result;
}

float GetLength(line2 Line)
{
    float LengthSq = GetLengthSq(Line);
    float Result = Sqrt(LengthSq);
    return Result;
}

float GetAngleBetween(line2 A, line2 B, angle_direction AngleDirection = AngleDirection_None)
{
    v2 ADiff = Normalize(A.P2 - A.P1);
    v2 BDiff = Normalize(B.P2 - B.P1);
    float Result = GetAngleBetween(ADiff, BDiff, AngleDirection);
    return Result;
}

bool HasSameVertex(line2 A, line2 B)
{
    bool Result = (A.P1 == B.P1 || A.P1 == B.P2 ||
                   A.P2 == B.P1 || A.P2 == B.P2);
    return Result;
}

v2 GetCenter(line2 L)
{
    v2 Result = L.P1 + 0.5f*(L.P2 - L.P1);
    return Result;
}

/* Triangles */

inline tri2
TRI2(v2 P1, v2 P2, v2 P3)
{
    tri2 Result = {P1, P2, P3};
    return Result;
}

/* For all intents and purposes, this is what it be. */
bool operator==(tri2 A, tri2 B)
{
    v2 BVertices[3];
    int NumBVertices = 3;
    memcpy(BVertices, B.Verts, sizeof(BVertices));

    bool Result = true;
    for (int I = 0; I < 3; I++)
    {
        bool HasVertex = false;
        for (int J = 0; J < NumBVertices; J++)
        {
            if (A.Verts[I] == BVertices[J])
            {
                HasVertex = true;
                BVertices[J] = BVertices[--NumBVertices];
                break;
            }
        }
        if (!HasVertex)
        {
            Result = false;
            break;
        }
    }
    return Result;
}

bool operator!=(tri2 A, tri2 B)
{
    bool Result = !(A == B);
    return Result;
}

void tri2::operator+=(v2 V)
{
    this->P1 += V;
    this->P2 += V;
    this->P3 += V;
}

void tri2::operator-=(v2 V)
{
    this->P1 -= V;
    this->P2 -= V;
    this->P3 -= V;
}

void tri2::operator*=(float A)
{
    this->P1 *= A;
    this->P2 *= A;
    this->P3 *= A;
}

v2
GetVertex(tri2 Tri, int Idx)
{
    v2 Result;
    switch (Idx)
    {
        case 0: Result = Tri.P1; break;
        case 1: Result = Tri.P2; break;
        default: Result = Tri.P3; break;
    }
    return Result;
}

int
GetIndexOf(tri2 Tri, v2 V)
{
    int Result = -1;
    for (int I = 0; I < ArrayCount(Tri.Verts); I++)
    {
        if (Tri.Verts[I] == V)
        {
            Result = I;
            break;
        }
    }
    Assert(Result != -1);
    return Result;
}

inline internal
bool IsCCW(v2 A, v2 B, v2 C)
{
    /* Copied and pasted from https://stackoverflow.com/a/1165943/5281200. */

    float Edge1 = (B.X - A.X)*(B.Y + A.Y);
    float Edge2 = (C.X - B.X)*(C.Y + B.Y);
    float Edge3 = (A.X - C.X)*(A.Y + C.Y);
    float Sum = Edge1 + Edge2 + Edge3;
    bool Result = Sum < 0;
    return Result;
}

float
GetSignedArea(v2 A, v2 B, v2 C)
{
    float AxMinusCx = A.X - C.X;
    float ByMinusCy = B.Y - C.Y;
    float AyMinusCy = A.Y - C.Y;
    float BxMinusCx = B.X - C.X;

    float Result = AxMinusCx*ByMinusCy - AyMinusCy*BxMinusCx;
    return Result;
}

bool CheckCollinear(tri2 Tri)
{
    float Area = GetSignedArea(Tri.P1, Tri.P2, Tri.P3);
    bool Result = (Area == 0.0f);
    return Result;
}

void
Split(tri2 Tri, v2 P, tri2 *OutputTris)
{
    OutputTris[0] = TRI2(Tri.P1, Tri.P2, P);
    OutputTris[1] = TRI2(Tri.P1, Tri.P3, P);
    OutputTris[2] = TRI2(Tri.P2, Tri.P3, P);
}

bool CheckHasAnyCommonVertices(tri2 A, tri2 B, v2 *CommonVertex_Out = NULL)
{
    bool Result = false;
    for (int I = 0; I < 3; I++)
    {
        for (int J = 0; J < 3; J++)
        {
            if (A.Verts[I] == B.Verts[J])
            {
                Result = true;
                if (CommonVertex_Out)
                {
                    *CommonVertex_Out = A.Verts[I];
                }
                goto end;
            }
        }
    }
end:
    return Result;
}

line2 LINE2(v2, v2);
void
GetEdges(tri2 Tri, line2 *OutputEdges)
{
    OutputEdges[0] = LINE2(Tri.P1, Tri.P2);
    OutputEdges[1] = LINE2(Tri.P2, Tri.P3);
    OutputEdges[2] = LINE2(Tri.P3, Tri.P1);
}

line2
GetEdge(tri2 Tri, int Index)
{
    line2 Edges[3];
    GetEdges(Tri, Edges);
    return Edges[Index];
}

bool operator==(line2, line2);
bool CheckHasCommonEdge(tri2 A, tri2 B, line2 *CommonEdge_Out = NULL)
{
    line2 AEdges[3], BEdges[3];
    GetEdges(A, AEdges);
    GetEdges(B, BEdges);
    bool Result = false;
    for (int AEdgeIdx = 0; AEdgeIdx < ArrayCount(AEdges); AEdgeIdx++)
    {
        line2 AEdge = AEdges[AEdgeIdx];
        for (int BEdgeIdx = 0; BEdgeIdx < ArrayCount(BEdges); BEdgeIdx++)
        {
            line2 BEdge = BEdges[BEdgeIdx];
            if (AEdge == BEdge)
            {
                if (CommonEdge_Out)
                {
                    *CommonEdge_Out = AEdge;
                }
                Result = true;
                goto end;
            }
        }
    }
end:
    return Result;
}

void
GetEdgesExcluding(tri2 Tri, line2 EdgeToExclude, line2 *OutputEdges)
{
    line2 Edges[3];
    GetEdges(Tri, Edges);
    if (Edges[0] == EdgeToExclude)
    {
        OutputEdges[0] = Edges[1];
        OutputEdges[1] = Edges[2];
    }
    else if(Edges[1] == EdgeToExclude)
    {
        OutputEdges[0] = Edges[0];
        OutputEdges[1] = Edges[2];
    }
    else
    {
        Assert(Edges[2] == EdgeToExclude);
        OutputEdges[0] = Edges[0];
        OutputEdges[1] = Edges[1];
    }
}

bool CheckContainsSameVertex(tri2 Tri, v2 V)
{
    bool Result = (Tri.P1 == V || Tri.P2 == V || Tri.P3 == V);
    return Result;
}

line2 GetEdgeExcluding(tri2 Tri, v2 Vertex)
{
    Assert(CheckContainsSameVertex(Tri, Vertex));

    line2 Result = {};
    line2 Edges[3];
    GetEdges(Tri, Edges);
    for (int I = 0; I < 3; I++)
    {
        if (!CheckContainsSameVertex(Edges[I], Vertex))
        {
            Result = Edges[I];
            break;
        }
    }
    return Result;
}

v2 GetVertexExcluding(tri2 Tri, line2 EdgeToExclude)
{
    v2 Result;
    if (!CheckContainsSameVertex(EdgeToExclude, Tri.P1))
    {
        Result = Tri.P1;
    }
    else if (!CheckContainsSameVertex(EdgeToExclude, Tri.P2))
    {
        Result = Tri.P2;
    }
    else
    {
        Assert(!CheckContainsSameVertex(EdgeToExclude, Tri.P3));
        Result = Tri.P3;
    }
    return Result;
}

/* Note that this is just one definition of the center of the triangle. */
v2 GetCenter(tri2 Tri)
{
    v2 Result = (1.0f/3.0f)*(Tri.P1 + Tri.P2 + Tri.P3);
    return Result;
}

bool CheckContains(tri2 Tri, line2 Edge)
{
    line2 Edges[3];
    GetEdges(Tri, Edges);
    bool Result = Edges[0] == Edge || Edges[1] == Edge || Edges[2] == Edge;
    return Result;
}

bool CheckContainsOneVertexOf(tri2 Tri, line2 Edge, v2 *SharedVertex, v2 *NonSharedVertex)
{
    bool Result;

    /* Caller only expects one vertex of the edge to be shared with the triangle, not the whole edge. Make sure that's true.  */
    if (!CheckContains(Tri, Edge))
    {
        if (CheckContainsSameVertex(Tri, Edge.P1))
        {
            *SharedVertex = Edge.P1;
            *NonSharedVertex = Edge.P2;
        }
        else
        {
            *SharedVertex = Edge.P2;
            *NonSharedVertex = Edge.P1;
        }
        Result = true;
    }
    else
    {
        Result = false;
    }
    return Result;
}

bool CheckHasAnyCommonVertices(tri2 Tri, line2 Edge)
{
    bool Result = (CheckContainsSameVertex(Tri, Edge.P1) || CheckContainsSameVertex(Tri, Edge.P2));
    return Result;
}

bool CheckContainsVertex(tri2 Tri, v2 V)
{
    bool Result = (Tri.P1 == V || Tri.P2 == V || Tri.P3 == V);
    return Result;
}

/* Rectangles */

rect2
RECT2(v2 Min, v2 Max)
{
    rect2 Result;
    Result.Min = Min;
    Result.Max = Max;
    return Result;
}

/* Don't delete this. Useful for scaling with respect to the origin. */
rect2
operator*(float A, rect2 B)
{
    rect2 Result;
    Result.Min = A*B.Min;
    Result.Max = A*B.Max;
    return Result;
}

void rect2::operator-=(v2 A)
{
    this->Max -= A;
    this->Min -= A;
}

rect2
RectFromPosSize(v2 Position, v2 Size)
{
    rect2 Result = {
        Position,
        Position + Size,
    };
    return Result;
}

bool
IsValid(rect2 R)
{
    bool Result = R.Min != R.Max;
    return Result;
}

quad2
QuadFromPosSize(v2 Position, v2 Size)
{
    v2 BottomLeft = { Position.X, Position.Y };
    v2 BottomRight = BottomLeft + V2(Size.X, 0.0f);
    v2 TopLeft = BottomLeft + V2(0.0f, Size.Y);
    v2 TopRight = BottomLeft + Size;
    quad2 Result = { BottomLeft, BottomRight, TopRight, TopLeft };
    return Result;
}

v2
GetPos(rect2 Rect)
{
    v2 Result = Rect.Min;
    return Result;
}

float
GetWidth(rect2 Rect)
{
    float Result = Rect.Max.X - Rect.Min.X;
    return Result;
}

float
GetHeight(rect2 Rect)
{
    float Result = Rect.Max.Y - Rect.Min.Y;
    return Result;
}

v2
GetSize(rect2 *Rect)
{
    v2 Result = Rect->Max - Rect->Min;
    return Result;
}

v2
GetSize(rect2 Rect)
{
    v2 Result = GetSize(&Rect);
    return Result;
}

v2
GetDim(rect2 Rect)
{
    v2 Result = GetSize(&Rect);
    return Result;
}

v2
GetDim(rect2 *Rect)
{
    v2 Result = GetSize(Rect);
    return Result;
}

v2
GetTopLeft(rect2 Rect)
{
    v2 Result = V2(Rect.Min.X, Rect.Max.Y);
    return Result;
}

v2
GetTopRight(rect2 Rect)
{
    v2 Result = Rect.Max;
    return Result;
}

v2
GetBottomLeft(rect2 Rect)
{
    v2 Result = Rect.Min;
    return Result;
}

v2
GetBottomRight(rect2 Rect)
{
    v2 Result = V2(Rect.Max.X, Rect.Min.Y);
    return Result;
}

void
GetCorners(rect2 Rect, v2 *Corners)
{
    Corners[0] = GetBottomLeft(Rect);
    Corners[1] = GetBottomRight(Rect);
    Corners[2] = GetTopRight(Rect);
    Corners[3] = GetTopLeft(Rect);
}

v2
GetCenter(rect2 *Rect)
{
    v2 Result = Rect->Min + 0.5f*(Rect->Max - Rect->Min);
    return Result;
}

v2
GetCenter(rect2 Rect)
{
    v2 Result = GetCenter(&Rect);
    return Result;
}

void
Scale(rect2 *Rect, float Scale)
{
    v2 Center = GetCenter(Rect);

    Rect->Min -= Center;
    Rect->Max -= Center;

    Rect->Min *= Scale;
    Rect->Max *= Scale;

    Rect->Min += Center;
    Rect->Max += Center;
}

void
Expand(rect2 *Rect, float ExpansionScalar)
{
    float HalfAmount = ExpansionScalar/2.0f;
    v2 Expansion = V2(ExpansionScalar, ExpansionScalar);
    Rect->Min -= Expansion;
    Rect->Max += Expansion;
}

/* Creates a bounding box of the given points. */
rect2 CreateBoundingBox(v2 *Points, int NumPoints)
{
    rect2 Result = RECT2(Points[0], Points[0]);
    for (int I = 0; I < NumPoints; I++)
    {
        v2 P = Points[I];
        if (P.X < Result.Min.X) 
            Result.Min.X = P.X;
        if (P.Y < Result.Min.Y) 
            Result.Min.Y = P.Y;
        if (P.X > Result.Max.X) 
            Result.Max.X = P.X;
        if (P.Y > Result.Max.Y) 
            Result.Max.Y = P.Y;
    }
    Assert(IsValid(Result));
    return Result;
}

/* Quads */

#if 0
quad2
operator*(float A, quad2 B)
{
    quad2 Result = {};
    Result.BottomLeft = A*B.BottomLeft;
    Result.BottomRight = A*B.BottomRight;
    Result.TopRight = A*B.TopRight;
    Result.TopLeft = A*B.TopLeft;
    return Result;
}
#endif

quad2
QUAD2(v2 BottomLeft, v2 BottomRight, v2 TopRight, v2 TopLeft)
{
    quad2 Result;
    Result.BottomLeft = BottomLeft;
    Result.BottomRight = BottomRight;
    Result.TopRight = TopRight;
    Result.TopLeft = TopLeft;
    return Result;
}

quad2
QUAD2(rect2 Rect)
{
    quad2 Result;
    Result.BottomLeft = GetBottomLeft(Rect);
    Result.BottomRight = GetBottomRight(Rect);
    Result.TopRight = GetTopRight(Rect);
    Result.TopLeft = GetTopLeft(Rect);
    return Result;
}

inline v3
quad2_3d::operator[](int CornerIdx)
{
    return Corners[CornerIdx];
}

void
GetCorners(quad2 Quad, v2 *Corners)
{
    Corners[0] = Quad.BottomLeft;
    Corners[1] = Quad.BottomRight;
    Corners[2] = Quad.TopRight;
    Corners[3] = Quad.TopLeft;
}

quad2_3d
GetPlaneCorners(plane Plane, float Size)
{
    quad2_3d Result;

    v3 OriginalNormal = V3(0.0f, 1.0f, 0.0f);
    quaternion PlaneNormalRotation = GetRotationBetween(OriginalNormal, Plane.Normal);

    v3 OriginalPlaneTopRight = Normalize(V3(1.0f, 0.0f, -1.0f));
    v3 OriginalPlaneBottomRight = Normalize(V3(1.0f, 0.0f, 1.0f));

    Result.TopRight = Size*Rotate(OriginalPlaneTopRight, PlaneNormalRotation);
    Result.BottomLeft = -Result.TopRight;
    Result.BottomRight = Size*Rotate(OriginalPlaneBottomRight, PlaneNormalRotation);
    Result.TopLeft = -Result.BottomRight;

    Result.TopRight += Plane.Position;
    Result.BottomLeft += Plane.Position;
    Result.BottomRight += Plane.Position;
    Result.TopLeft += Plane.Position;

    return Result;
}

/* Intersections */

bool CheckContains(line2 L, v2 V)
{
    bool Result = (L.P1 == V ||
                   L.P2 == V);
    return Result;
}

/* Returns */
bool
GetAdjacentEdges(tri2 Tri, v2 Vertex, line2 *Out_Edge0, line2 *Out_Edge1)
{
    bool Result = CheckContainsVertex(Tri, Vertex);
    if (Result)
    {
        line2 Edges[3];
        GetEdges(Tri, Edges);
        line2 *AdjacentEdges[] = {Out_Edge0, Out_Edge1};
        int NumAdjacentEdges = 0;
        for (int I = 0; I < ArrayCount(Edges); I++)
        {
            if (CheckContains(Edges[I], Vertex))
            {
                Assert(NumAdjacentEdges < ArrayCount(AdjacentEdges));
                *AdjacentEdges[NumAdjacentEdges++] = Edges[I];
            }
        }
        Assert(NumAdjacentEdges == 2);
    }
    return Result;
}

bool CheckInside(v2 Point, tri2 Tri, bool *IsPointOnEdge_Out = NULL)
{
    /* Thanks http://blackpawn.com/texts/pointinpoly/. */

    v2 v0 = Tri.P2 - Tri.P1;
    v2 v1 = Tri.P3 - Tri.P1;
    v2 v2 = Point - Tri.P1;

    /* Compute dot products. */
    float dot00 = GetDot(v0, v0);
    float dot01 = GetDot(v0, v1);
    float dot02 = GetDot(v0, v2);
    float dot11 = GetDot(v1, v1);
    float dot12 = GetDot(v1, v2);

    /* Compute barycentric coordinates. */
    float invDenom = 1.0f/(dot00*dot11 - dot01*dot01);
    float u = invDenom*(dot11*dot02 - dot01*dot12);
    float v = invDenom*(dot00*dot12 - dot01*dot02);
    float uv = u + v;

    /* Check if point is in triangle. */
    bool Result = (u >= 0) && (v >= 0) && (uv <= 1.0f || IsWithinTolerance(uv, 1.0f));
    if (IsPointOnEdge_Out)
    {
        if (Result)
        {
            *IsPointOnEdge_Out = (u == 0) || (v == 0) || IsWithinTolerance(uv, 1.0f);
        }
        else
        {
            *IsPointOnEdge_Out = false;
        }
    }
    return Result;
}

bool
IsInside(v2 Point, rect2 Rectangle)
{
    bool Result = (Point.X >= Rectangle.Min.X && Point.X <= Rectangle.Max.X &&
                   Point.Y >= Rectangle.Min.Y && Point.Y <= Rectangle.Max.Y);
    return Result;
}

v2
GetIntersectionYPlane(ray2 A, float Y)
{
    Assert(IsValid(A));
    /* Get v such that [A.Pos.Y + v*A.Dir.Y == Y]. */
    float v = (Y/A.Dir.Y) - (A.Pos.Y/A.Dir.Y);
    v2 Result = A.Pos + v*A.Dir;
    return Result;
}

inline bool
CheckParallel(ray2 A, ray2 B)
{
    /* Ripped from "Real Time Collision Detection" pg 152. */
    float SignedTriangleArea = A.Dir.X*B.Dir.Y - A.Dir.Y*B.Dir.X;
    bool Result = (SignedTriangleArea < TOLERANCE) && (SignedTriangleArea > -TOLERANCE);
    return Result;
}

inline bool
CheckParallel(line2 A, line2 B)
{
    ray2 LineARay = {A.P1, A.P2 - A.P1};
    ray2 LineBRay = {B.P1, B.P2 - B.P1};
    bool Result = CheckParallel(LineARay, LineBRay);
    return Result;
}

bool
GetIntersection(ray2 A, ray2 B, v2 *PointOfIntersection_Out, bool BackwardsAllowed = false, float *u_Out = 0, float *v_Out = 0)
{
    v2 PointOfIntersection = {};
    float u = 0, v = 0;
    bool DoesIntersect = false;

    /* The following three lines were copied and pasted. */
    float dx = B.Pos.X - A.Pos.X;
    float dy = B.Pos.Y - A.Pos.Y;
    float det = B.Dir.X * A.Dir.Y - B.Dir.Y * A.Dir.X;

    bool NonParallel = (det != 0.0f);
    if(NonParallel)
    {
        /* Find u and v such that (A.Pos + u*A.Dir == B.Pos + v*B.Dir). */
        u = (dy * B.Dir.X - dx * B.Dir.Y) / det;
        v = (dy * A.Dir.X - dx * A.Dir.Y) / det;

        if(BackwardsAllowed ||
           (u >= 0.0f && v >= 0.0f))
        {
            PointOfIntersection = A.Pos + u*A.Dir;
            DoesIntersect = true;
        }
    }
    else
    {
        /* Check if rays point at each other. */

        if(A.Dir.X == 0.0f /*&& B.Dir.X == 0.0f*/)
        {
            DoesIntersect = (A.Pos.X == B.Pos.X);
        }
        else if(A.Dir.Y == 0.0f /*&& B.Dir.Y == 0.0f*/)
        {
            DoesIntersect = (A.Pos.Y == B.Pos.Y);
        }
        else
        {
            /* Try to find t such that (A.Pos + t*A.Dir = B.Pos). */
            float tx = (B.Pos.X - A.Pos.X) / A.Dir.X;
            float ty = (B.Pos.Y - A.Pos.Y) / A.Dir.Y;

            float Difference = Abs(tx - ty);
            DoesIntersect = (Difference < TOLERANCE);
        }

        if(DoesIntersect)
        {
            PointOfIntersection = Lerp(A.Pos, 0.5f, B.Pos);
            DoesIntersect = true;
        }
    }

    if(PointOfIntersection_Out)
    {
        *PointOfIntersection_Out = PointOfIntersection;
    }

    if(u_Out)
    {
        *u_Out = u;
    }

    if(v_Out)
    {
        *v_Out = v;
    }

    return DoesIntersect;
}

bool GetIntersection(ray2 Ray, line2 Line, v2 *PointOfIntersection)
{
    ray2 LineRay = {Line.P1, Line.P2 - Line.P1};
    float u, v;
    if(!GetIntersection(Ray, LineRay, PointOfIntersection, false, &u, &v))
    {
        return false;
    }
    return (v >= 0.0f && v <= 1.0f);
}

bool Intersects(ray2 Ray, line2 Line)
{
    bool Result = GetIntersection(Ray, Line, NULL);
    return Result;
}

bool DoesIntersect(ray2 A, ray2 B)
{
    bool Result = GetIntersection(A, B, 0);
    return Result;
}

/* Note that if LineA and LineB share a vertex, this returns true. */
bool GetIntersection(line2 LineA, line2 LineB, v2 *PointOfIntersection_Out, bool DoEndpointsCountAsIntersection = false, bool *Overlaps_Out = NULL)
{
    /* Copied and pasted from "Real-Time Collision Detection" pg 152. */

    v2 PointOfIntersection = {};
    bool DoesIntersect = false;
    if (Overlaps_Out)
    {
        *Overlaps_Out = false;
    }

    v2 a = LineA.P1, b = LineA.P2;
    v2 c = LineB.P1, d = LineB.P2;

    float a1 = GetSignedArea(a, b, d);
    float a2 = GetSignedArea(a, b, c);

    /* Check for collinear lines (all points lie on the same line). */
    if (a1 == 0.0f && a2 == 0.0f)
    {
        /* They are collinear. They only intersect if they also overlap. */

        /* Check X. */
        {
            /* Ensure a < b and c < d */
            {
                if (b.X < a.X)
                {
                    float Tmp = a.X;
                    a.X = b.X;
                    b.X = Tmp;
                }

                if (d.X < c.X)
                {
                    float Tmp = c.X;
                    c.X = d.X;
                    d.X = Tmp;
                }
            }
            if (b.X > c.X && a.X < d.X)
            {
                Assert(PointOfIntersection_Out == NULL);
                DoesIntersect = true;
                *Overlaps_Out = true;
                if (Overlaps_Out)
                {
                    *Overlaps_Out = true;
                }
            }
        }

        /* Check Y. */
        {
            /* Ensure a < b and c < d */
            {
                if (b.Y < a.Y)
                {
                    float Tmp = a.Y;
                    a.Y = b.Y;
                    b.Y = Tmp;
                }

                if (d.Y < c.Y)
                {
                    float Tmp = c.Y;
                    c.Y = d.Y;
                    d.Y = Tmp;
                }
            }
            if (b.Y > c.Y && a.Y < d.Y)
            {
                Assert(PointOfIntersection_Out == NULL);
                DoesIntersect = true;
                if (Overlaps_Out)
                {
                    *Overlaps_Out = true;
                }
            }
        }
    }
    else if(a1 != 0.0f && a2 != 0.0f && a1*a2 < 0.0f)
    {
        float a3 = GetSignedArea(c, d, a);
        float a4 = a3 + a2 - a1;
        if(a3 != 0.0f && a4 != 0.0f && a3*a4 < 0.0f)
        {
            float t = a3 / (a3 - a4);
            bool EndpointsOverlap = (t == 1.0f || t == 0.0f);
            if (!EndpointsOverlap ||
                (EndpointsOverlap && DoEndpointsCountAsIntersection))
            {
                PointOfIntersection = a + t*(b - a);
                DoesIntersect = true;
            }
        }
    }

    if(PointOfIntersection_Out)
    {
        *PointOfIntersection_Out = PointOfIntersection;
    }

    return DoesIntersect;
}

bool
DoesIntersect(line2 A, line2 B)
{
    bool Result = GetIntersection(A, B, NULL);
    return Result;
}

bool
Intersects(line2 A, line2 B, bool *Overlaps_Out = NULL)
{
    bool Result = GetIntersection(A, B, NULL, NULL, Overlaps_Out);
    return Result;
}

v2 GetVector(line2 L)
{
    v2 V = L.P2 - L.P1;
    return V;
}

float DoCross(line2 A, line2 B)
{
    v2 AVec = GetVector(A);
    v2 BVec = GetVector(B);
    float CrossProduct = DoCross(AVec, BVec);
    return CrossProduct;
}

bool CheckAreParallel(line2 A, line2 B)
{
    float CrossProduct = DoCross(A, B);
    bool Result = (CrossProduct == 0.0f);
    return Result;
}

bool
Intersects(line2 Line, tri2 Triangle)
{
    line2 Edges[3];
    GetEdges(Triangle, Edges);
    bool Result = false;
    for (int I = 0; I < 3; I++)
    {
        line2 Edge = Edges[I];
        if (Intersects(Line, Edge) || CheckAreParallel(Line, Edge))
        {
            Result = true;
            break;
        }
    }
    return Result;
}

bool
Intersects(tri2 Triangle, line2 Line)
{
    bool Result = Intersects(Line, Triangle);
    return Result;
}

/* Misc */

/* Note that this returns false for proven-to-be equilateral triangles (i.e. I tried a triangle whose sides were exactly 1.0f
   (I directly compared them), yet one of the angles was slightly off). */
void DebugEnsureEquilateral(tri2 Tri)
{
    line2 Edges[3];
    GetEdges(Tri, Edges);

    float Angles[3] = {
        /* Top middle corner. */
        GetAngleBetween(Edges[0], -Edges[2]),
        /* Bottom right corner. */
        GetAngleBetween(Edges[1], -Edges[0]),
        /* Bottom left corner. */
        GetAngleBetween(Edges[2], -Edges[1]),
    };
    float Lengths[] = {GetLength(Edges[0]), GetLength(Edges[1]), GetLength(Edges[2])};

    Assert((Lengths[0] == Lengths[1]) && (Lengths[1] == Lengths[2]));
    Assert(Angles[0] == Angles[1] && Angles[1] == Angles[2]);
}

/* A super triangle is a equilateral triangle that encapsulates all specified points. We also add some optional padding. */
tri2 CreateSuperTriangle(v2 *Points, int NumPoints, float Padding = 50.0f)
{
    rect2 BoundingBox = CreateBoundingBox(Points, NumPoints);
    Expand(&BoundingBox, Padding);

    v2 TopLeft = GetTopLeft(BoundingBox);
    v2 TopRight = GetTopRight(BoundingBox);
    v2 TopLeftDir = Rotate(V2(1.0f, 0.0f), 60.0f);
    v2 TopRightDir = Rotate(V2(-1.0f, 0.0f), -60.0f);
    ray2 TopLeftRay = RAY2(TopLeft, TopLeftDir);
    ray2 TopRightRay = RAY2(TopRight, TopRightDir);

    tri2 Result;
    bool DidIntersect = GetIntersection(TopLeftRay, TopRightRay, &Result.P1);
    Assert(DidIntersect);
    /* Get bottom left and right. */
    ReverseDirection(&TopLeftRay);
    ReverseDirection(&TopRightRay);
    Result.P2 = GetIntersectionYPlane(TopLeftRay, BoundingBox.Min.Y);
    Result.P3 = GetIntersectionYPlane(TopRightRay, BoundingBox.Min.Y);

    return Result;
}

void CreateConvexHull(v2 *Points, int NumPoints, v2 *ConvexHullPoints, int ConvexHullPointsSize, int *NumConvexHullPoints_Out)
{
    int NumConvexHullPoints = 0;
    int ConvexHullFrontIdx = 0;

    int LeftMostPointIdx = FindSmallest(Points, NumPoints, [](v2 A, v2 B) -> bool { return A.X > B.X; });
    v2 LeftMostPoint = Points[LeftMostPointIdx];
    ConvexHullPoints[NumConvexHullPoints++] = LeftMostPoint;
    v2 BelowLeftMostPoint = LeftMostPoint - V2(0.0f, -1.0f);

    /* Move the left most point to the front of the original array. It's okay. User gave us permission. */
    v2 Tmp = Points[ConvexHullFrontIdx];
    Points[ConvexHullFrontIdx] = Points[LeftMostPointIdx];
    Points[LeftMostPointIdx] = Tmp;
    ConvexHullFrontIdx++;

    line2 LastEdge = LINE2(BelowLeftMostPoint, LeftMostPoint);
    while (1)
    {
        v2 LastPoint = ConvexHullPoints[NumConvexHullPoints - 1];
        v2 LastEdgeDir = GetDirection(LastEdge);

        /* Find next point in convex hull. */
        int SmallestAngleIdx = -1;
        float SmallestAngle = FLT_MAX;
        for (int PointIdx = 0; PointIdx < NumPoints; PointIdx++)
        {
            v2 Point = Points[PointIdx];
            if (Point == LastPoint)
            {
                continue;
            }

            line2 LastPointToNewPoint = LINE2(LastPoint, Point);
            v2 LastPointToNewPointDir = GetDirection(LastPointToNewPoint);
            float Angle = GetAngleBetween(LastEdgeDir, LastPointToNewPointDir);
            if (Angle < SmallestAngle)
            {
                SmallestAngleIdx = PointIdx;
                SmallestAngle = Angle;
            }
            else if (Angle == SmallestAngle)
            {
                /* Pick the closest one. */

                float CurDist = GetDistance(LastPoint, Points[SmallestAngleIdx]);
                float Distance = GetDistance(LastPoint, Point);
                Assert(Distance != CurDist);
                if (Distance < CurDist)
                {
                    SmallestAngleIdx = PointIdx;
                    SmallestAngle = Angle;
                }
            }
        }
        Assert(SmallestAngleIdx != -1);

        v2 NewPoint = Points[SmallestAngleIdx];
        if (NewPoint == ConvexHullPoints[0])
        {
            /* We're done. */
            break;
        }

        Assert(NumConvexHullPoints < ConvexHullPointsSize);
        {
            /* Move this point to the front of the original array. It's okay. User gave us permission. */
            Tmp = Points[ConvexHullFrontIdx];
            Points[ConvexHullFrontIdx] = Points[SmallestAngleIdx];
            Points[SmallestAngleIdx] = Tmp;
            ConvexHullFrontIdx++;

            /* Now add it. */
            ConvexHullPoints[NumConvexHullPoints++] = NewPoint;
        }

        LastEdge = LINE2(ConvexHullPoints[NumConvexHullPoints - 2], ConvexHullPoints[NumConvexHullPoints - 1]);
    }

    Assert(NumConvexHullPoints > 2);
    *NumConvexHullPoints_Out = NumConvexHullPoints;
}

/* Print functions. */

void Printf(v2 V, bool NewLine = true)
{
    if (NewLine)
    {
        Printf("{ %f, %f }\n", V.X, V.Y);
    }
    else
    {
        Printf("{ %f, %f }", V.X, V.Y);
    }
}

void Printf(line2 Line)
{
    Printf(Line.P1, false);
    Printf(", ");
    Printf(Line.P2, true);
}

void
Print(quad2 Quad)
{
    Printf("Quad:\n");
    Print(Quad.BottomLeft);
    Print(Quad.BottomRight);
    Print(Quad.TopRight);
    Print(Quad.TopLeft);
}

#endif
