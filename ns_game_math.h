#ifndef NS_GAME_MATH_H
#define NS_GAME_MATH_H

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

struct line2
{
    v2 P1, P2;
};

struct plane
{
    v3 Position;
    v3 Normal;
};

struct rect2
{
    v2 Min, Max;
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

struct tri2
{
    v2 P1, P2, P3;
};


/* Vectors */

inline internal v2
V2(float X, float Y)
{
    v2 Result = {X, Y};
    return Result;
}

inline internal v3
V3(float X, float Y, float Z)
{
    v3 Result = {X, Y, Z};
    return Result;
}

inline internal v4
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

inline internal v2
operator-(v2 V)
{
    v2 Result = {-V.X, -V.Y};
    return Result;
}

inline internal v3
operator-(v3 V)
{
    v3 Result = {-V.X, -V.Y, -V.Z};
    return Result;
}

inline internal v2
operator+(v2 A, v2 B)
{
    v2 Result = {A.X + B.X, A.Y + B.Y};
    return Result;
}

inline internal v3
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

inline internal v2
operator-(v2 A, v2 B)
{
    v2 Result = {A.X - B.X, A.Y - B.Y};
    return Result;
}

inline internal v3
operator-(v3 A, v3 B)
{
    v3 Result = {A.X - B.X, A.Y - B.Y, A.Z - B.Z};
    return Result;
}

inline internal v2
operator*(float S, v2 V)
{
    v2 Result = {S*V.X, S*V.Y};
    return Result;
}

inline internal v3
operator*(float S, v3 V)
{
    v3 Result = {S*V.X, S*V.Y, S*V.Z};
    return Result;
}

inline internal v3
operator/(v3 V, float S)
{
    v3 Result = (1.0f / S) * V;
    return Result;
}

inline internal bool
operator==(v2 Left, v2 Right)
{
    bool Result = (Left.X == Right.X &&
                     Left.Y == Right.Y);
    return Result;
}

inline internal bool
operator!=(v2 Left, v2 Right)
{
    bool Result = (Left.X != Right.X ||
                     Left.Y != Right.Y);
    return Result;
}

inline internal bool
IsZero(v2 A)
{
    bool Result = (A.X == 0.0f && A.Y == 0.0f);
    return Result;
}

inline internal v3
Cross(v3 A, v3 B)
{
    v3 Result = {
        A.Y*B.Z - A.Z*B.Y,
        A.Z*B.X - A.X*B.Z,
        A.X*B.Y - A.Y*B.X
    };

    return Result;
}

inline internal float
GetLength(v2 *V)
{
    float Result = Sqrt(V->X*V->X + V->Y*V->Y);
    return Result;
}

inline internal float
GetLength(v2 V)
{
    float Result = GetLength(&V);
    return Result;
}

inline internal float
GetLengthSq(v2 V)
{
    float Length = GetLength(&V);
    float Result = Length*Length;
    return Result;
}

inline internal float
GetDistance(v2 A, v2 B)
{
    float Result = GetLength(B - A);
    return Result;
}

inline internal void
Normalize(v2 *V)
{
    float Length = GetLength(V);
    V->X /= Length;
    V->Y /= Length;
}

inline internal void
Normalize(v3 *V)
{
    float Length = Sqrt(V->X*V->X + V->Y*V->Y + V->Z*V->Z);
    V->X /= Length;
    V->Y /= Length;
    V->Z /= Length;
}

inline internal v2
Normalize(v2 V)
{
    Normalize(&V);
    return V;
}

inline internal v3
Normalize(v3 V)
{
    Normalize(&V);
    return V;
}

inline internal float
GetDot(v2 A, v2 B)
{
    float Result = A.X*B.X + A.Y*B.Y;
    return Result;
}

inline internal float
GetDot(v3 A, v3 B)
{
    float Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return Result;
}

inline internal float
GetAngleBetween(v2 A, v2 B)
{
    float ALengthSq = GetLengthSq(A);
    float BLengthSq = GetLengthSq(B);
    Assert(IsWithinTolerance(ALengthSq, 1.0f) && IsWithinTolerance(BLengthSq, 1.0f));

    float Result = ToDegrees(Acos(GetDot(A, B)));
    return Result;
}

inline internal float
GetAngleBetween(v3 A, v3 B)
{
    float Result = ToDegrees(Acos(GetDot(A, B)));
    return Result;
}

inline internal v2
Lerp(v2 A, float t, v2 B)
{
    v2 Result = A + t*(B - A);
    return Result;
}

inline internal v3
Lerp(v3 A, v3 B, float t)
{
    v3 Result = A + t*(B - A);
    return Result;
}

inline internal v2
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
inline internal void
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
inline internal quaternion
GetRotationBetween(v3 A, v3 B)
{
    v3 RotAxis = Cross(A, B);
    float RotHalfAngle = GetAngleBetween(A, B) / 2.0f;
    quaternion Result = {Sin(RotHalfAngle)*RotAxis, Cos(RotHalfAngle)};
    return Result;
}

/* Quaternions */

inline internal quaternion
Conjugate(quaternion Quat)
{
    quaternion Result = {-Quat.Axis, Quat.W};
    return Result;
}

inline internal quaternion
operator*(quaternion A, quaternion B)
{
    v3 ResultAxis = Cross(A.Axis, B.Axis) + A.W*B.Axis + B.W*A.Axis;
    float ResultW = A.W*B.W - GetDot(A.Axis, B.Axis);

    quaternion Result = {ResultAxis, ResultW};
    return Result;
}

inline internal void
Rotate(v3 *Vector, quaternion RotQuat)
{
    quaternion VectorQuat = {*Vector, 1.0f};
    quaternion ResultQuat = RotQuat * VectorQuat * Conjugate(RotQuat);

    Vector->X = ResultQuat.X;
    Vector->Y = ResultQuat.Y;
    Vector->Z = ResultQuat.Z;
}

inline internal v3
Rotate(v3 Vector, quaternion RotQuat)
{
    Rotate(&Vector, RotQuat);
    return Vector;
}

inline internal void
Rotate(v3 *Vector, v3 Axis, float RotationDegrees)
{
    float RotationRadians = ToRadians(RotationDegrees);
    float SinHalfAngle = Sin(RotationRadians / 2);
    float CosHalfAngle = Cos(RotationRadians / 2);

    quaternion RotQuat = {SinHalfAngle*Axis, CosHalfAngle};
    Rotate(Vector, RotQuat);
}

/* Matrices */

inline float &
mat4::operator[](int ElementIdx)
{
    return this->E1[ElementIdx];
}

inline internal mat4
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

inline internal v4
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

inline internal mat4 
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

inline internal mat4
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

inline internal void
Translate(mat4 *Matrix, v3 Translation)
{
    Matrix->m03 += Translation.X;
    Matrix->m13 += Translation.Y;
    Matrix->m23 += Translation.Z;
}

inline internal mat4
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

inline internal mat4
CreateTranslation(v3 Translation)
{
    mat4 Result = CreateTranslation(Translation.X, Translation.Y, Translation.Z);
    return Result;
}

inline internal mat4
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

inline internal void
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

inline internal void
Zero(mat4 *Matrix)
{
    for(int I = 0; I < 16; I++)
    {
        Matrix->E1[I] = 0.0f;
    }
}

/* Cut and pasted from stackoverflow. Seeing how it works without any
 * transposes, PRESUMABLY it assumes row-major ordering. */
internal mat4
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

internal inline ray2
ToRay2(line2 Line, bool NormalizeDir = true)
{
    v2 Dir = Line.P2 - Line.P1;
    if(NormalizeDir)
    {
        Normalize(&Dir);
    }
    v2 Pos = Line.P1;

    ray2 Result = {Pos, Dir};
    return Result;
}

/* Triangles */

internal inline tri2
TRI2(v2 P1, v2 P2, v2 P3)
{
    tri2 Result = {P1, P2, P3};
    return Result;
}

/* Copied and pasted from https://stackoverflow.com/a/1165943/5281200. */
inline internal
bool IsCCW(v2 A, v2 B, v2 C)
{
    float Edge1 = (B.X - A.X)*(B.Y + A.Y);
    float Edge2 = (C.X - B.X)*(C.Y + B.Y);
    float Edge3 = (A.X - C.X)*(A.Y + C.Y);
    float Sum = Edge1 + Edge2 + Edge3;
    bool Result = Sum < 0;
    return Result;
}

internal float
GetSignedArea(v2 A, v2 B, v2 C)
{
    float AxMinusCx = A.X - C.X;
    float ByMinusCy = B.Y - C.Y;
    float AyMinusCy = A.Y - C.Y;
    float BxMinusCx = B.X - C.X;

    float Result = AxMinusCx*ByMinusCy - AyMinusCy*BxMinusCx;
    return Result;
}

/* Quads */

inline v3
quad2_3d::operator[](int CornerIdx)
{
    return Corners[CornerIdx];
}

internal quad2_3d
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

/* Rectangles */

internal rect2
RectFromPosSize(v2 Position, v2 Size)
{
    rect2 Result = {
        Position,
        Position + Size,
    };
    return Result;
}

internal bool
IsInsideRectangle(v2 Point, rect2 Rectangle)
{
    bool Result = (Point.X >= Rectangle.Min.X && Point.X <= Rectangle.Max.X &&
                     Point.Y >= Rectangle.Min.Y && Point.Y <= Rectangle.Max.Y);
    return Result;
}

internal v2
GetPos(rect2 Rect)
{
    v2 Result = Rect.Min;
    return Result;
}

internal v2
GetSize(rect2 Rect)
{
    v2 Result = {
        Rect.Max.X - Rect.Min.X + 1,
        Rect.Max.Y - Rect.Min.Y + 1,
    };
    return Result;
}

/* Lines */

inline internal line2
LINE2(v2 P1, v2 P2)
{
    line2 Result = {P1, P2};
    return Result;
}

inline internal line2
LINE2(float X1, float Y1, float X2, float Y2)
{
    line2 Result = {X1, Y1, X2, Y2};
    return Result;
}

inline internal v2
Rotate90CCW(v2 V)
{
    v2 Result = {-V.Y, V.X};
    return Result;
}

/* Returns normal that is 90 degrees CCW to the line. */
inline internal v2
GetNormal(line2 Line)
{
    v2 OriginVector = Line.P2 - Line.P1;
    v2 LineNormal = Normalize(Rotate90CCW(OriginVector));
    return LineNormal;
}

inline internal void
RotateAroundCenter(line2 *Line, float AngleDegrees)
{
    v2 LineCenter = Lerp(Line->P1, 0.5f, Line->P2);
    RotateAround(&Line->P1, LineCenter, AngleDegrees);
    RotateAround(&Line->P2, LineCenter, AngleDegrees);
}

inline internal line2
RotateAroundCenter(line2 Line, float AngleDegrees)
{
    RotateAroundCenter(&Line, AngleDegrees);
    return Line;
}

internal quad2
CreateLineQuad(line2 Line, float Width)
{
    Assert(Line.P1 != Line.P2);

    float HalfWidth = Width / 2.0f;

    v2 Normal = GetNormal(Line);

    quad2 Result;
    Result.BottomLeft = Line.P1 - HalfWidth*Normal;
    Result.BottomRight = Line.P2 - HalfWidth*Normal;
    Result.TopRight = Line.P2 + HalfWidth*Normal;
    Result.TopLeft = Line.P1 + HalfWidth*Normal;

    return Result;
}

/* Calculates the value of P projected onto the line. */
internal float
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
internal float
GetDistance(v2 P, line2 Line)
{
    float t = Clamp01(GetProjectedCoord(P, Line));
    v2 LineDir = Line.P2 - Line.P1;
    v2 ClosestPointOnLine = Line.P1 + t*LineDir;
    float DistanceToLine = GetLength(P - ClosestPointOnLine);
    return DistanceToLine;
}

/* Intersections */

internal inline bool
CheckParallel(ray2 A, ray2 B)
{
    /* Ripped from "Real Time Collision Detection" pg 152. */
    float SignedTriangleArea = A.Dir.X*B.Dir.Y - A.Dir.Y*B.Dir.X;
    bool Result = (SignedTriangleArea < TOLERANCE) && (SignedTriangleArea > -TOLERANCE);
    return Result;
}

internal inline bool
CheckParallel(line2 A, line2 B)
{
    ray2 LineARay = {A.P1, A.P2 - A.P1};
    ray2 LineBRay = {B.P1, B.P2 - B.P1};
    bool Result = CheckParallel(LineARay, LineBRay);
    return Result;
}

internal bool
FindIntersection(ray2 A, ray2 B, v2 *PointOfIntersection_Out, bool BackwardsAllowed = false, float *u_Out = 0, float *v_Out = 0)
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

internal bool
FindIntersection(ray2 Ray, line2 Line, v2 *PointOfIntersection)
{
    ray2 LineRay = {Line.P1, Line.P2 - Line.P1};
    float u, v;
    if(!FindIntersection(Ray, LineRay, PointOfIntersection, false, &u, &v))
    {
        return false;
    }
    return (v >= 0.0f && v <= 1.0f);
}

internal bool
DoesIntersect(ray2 A, ray2 B)
{
    bool Result = FindIntersection(A, B, 0);
    return Result;
}

/* Copied and pasted from "Real-Time Collision Detection" pg 152. */
internal bool
FindIntersection(line2 LineA, line2 LineB, v2 *PointOfIntersection_Out)
{
    v2 PointOfIntersection = {};
    bool DoesIntersect = false;

    v2 a = LineA.P1, b = LineA.P2;
    v2 c = LineB.P1, d = LineB.P2;

    float a1 = GetSignedArea(a, b, d);
    float a2 = GetSignedArea(a, b, c);
    if(a1 != 0.0f && a2 != 0.0f && a1*a2 < 0.0f)
    {
        float a3 = GetSignedArea(c, d, a);
        float a4 = a3 + a2 - a1;
        if(a3 != 0.0f && a4 != 0.0f && a3*a4 < 0.0f)
        {
            float t = a3 / (a3 - a4);
            PointOfIntersection = a + t*(b - a);
            DoesIntersect = true;
        }
    }

    if(PointOfIntersection_Out)
    {
        *PointOfIntersection_Out = PointOfIntersection;
    }

    return DoesIntersect;
}

internal bool
DoesIntersect(line2 A, line2 B)
{
    bool Result = FindIntersection(A, B, 0);
    return Result;
}

#endif
