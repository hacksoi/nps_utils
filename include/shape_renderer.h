#ifndef SHAPE_RENDERER_H
#define SHAPE_RENDERER_H

#define SR_MAX_POINTS 64
#define SR_MAX_LINES 64

#define SR_NUM_POINT_ELEMENTS 6 // 6 = xyrgba
#define SR_NUM_LINE_ELEMENTS (2*6) // 2 = p1,p2; 6 = xyrgba

struct shape_renderer
{
    uint32_t WindowWidth;
    uint32_t WindowHeight;

    uint32_t ShaderProgram;
    uint32_t Vbo;
    uint32_t Vao;

    float PointData[SR_MAX_POINTS*SR_NUM_POINT_ELEMENTS];
    float LineData[SR_MAX_LINES*SR_NUM_LINE_ELEMENTS];

    uint32_t NumPoints;
    uint32_t NumLines;
};

internal bool32
InitializeShapeRenderer(shape_renderer *ShapeRenderer, uint32_t WindowWidth, uint32_t WindowHeight)
{
const char *VertexShaderSource = R"STR(
    #version 330 core
    layout(location = 0) in vec2 Pos;
    layout(location = 1) in vec4 vsColor;

    uniform vec2 WindowDimensions;

    out vec4 fsColor;

    void main()
    {
        vec2 ClipPos = ((2.0f * Pos) / WindowDimensions) - 1.0f;
        gl_Position = vec4(ClipPos, 0.0, 1.0f);

        fsColor = vsColor;
    }
    )STR";

const char *FragmentShaderSource = R"STR(
    #version 330 core
    in vec4 fsColor;

    out vec4 OutputColor;

    void main()
    {
        OutputColor = fsColor;
    }
    )STR";

    ShapeRenderer->WindowWidth = WindowWidth;
    ShapeRenderer->WindowHeight = WindowHeight;

    ShapeRenderer->ShaderProgram = CreateShaderProgramVF(VertexShaderSource, FragmentShaderSource);
    if(ShapeRenderer->ShaderProgram == 0)
    {
        return false;
    }

    ShapeRenderer->Vbo = CreateAndBindVertexBuffer();
    ShapeRenderer->Vao = CreateAndBindVertexArray();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)0); // position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)(2*sizeof(float))); // color
    glEnableVertexAttribArray(1);

    return true;
}

internal void
AddPoint(shape_renderer *ShapeRenderer, v2 P, v4 Color)
{
    if(ShapeRenderer->NumPoints < SR_MAX_POINTS)
    {
        float *InsertPoint = ShapeRenderer->PointData + SR_NUM_POINT_ELEMENTS*ShapeRenderer->NumPoints;

        InsertPoint[0] = P.X;
        InsertPoint[1] = P.Y;
        InsertPoint[2] = Color.R;
        InsertPoint[3] = Color.G;
        InsertPoint[4] = Color.B;
        InsertPoint[5] = Color.A;

        ShapeRenderer->NumPoints++;
    }
}

internal void
AddRay(shape_renderer *ShapeRenderer, ray2 Ray, float Length, v4 Color)
{
    if(ShapeRenderer->NumLines < SR_MAX_LINES)
    {
        v2 P1 = Ray.Pos;
        v2 P2 = Ray.Pos + Length*Ray.Dir;

        float *InsertPoint = ShapeRenderer->LineData + SR_NUM_LINE_ELEMENTS*ShapeRenderer->NumLines;

        InsertPoint[0] = P1.X;
        InsertPoint[1] = P1.Y;
        InsertPoint[2] = Color.R;
        InsertPoint[3] = Color.G;
        InsertPoint[4] = Color.B;
        InsertPoint[5] = Color.A;

        InsertPoint[6] = P2.X;
        InsertPoint[7] = P2.Y;
        InsertPoint[8] = Color.R;
        InsertPoint[9] = Color.G;
        InsertPoint[10] = Color.B;
        InsertPoint[11] = Color.A;

        ShapeRenderer->NumLines++;
    }
}

internal void
RenderAndFlush(shape_renderer *ShapeRenderer)
{
    glUseProgram(ShapeRenderer->ShaderProgram);
    glBindVertexArray(ShapeRenderer->Vao);

    int WindowDimensionsUniformLocation = glGetUniformLocation(ShapeRenderer->ShaderProgram, "WindowDimensions");
    //Assert(WindowDimensionsUniformLocation != -1);
    glUniform2f(WindowDimensionsUniformLocation, (float)ShapeRenderer->WindowWidth, (float)ShapeRenderer->WindowHeight);

    glPointSize(7.0f);
    glLineWidth(3.0f);

    // points
    FillVertexBuffer(ShapeRenderer->Vbo, ShapeRenderer->PointData, sizeof(ShapeRenderer->PointData));
    glDrawArrays(GL_POINTS, 0, ShapeRenderer->NumPoints);

    // lines
    FillVertexBuffer(ShapeRenderer->Vbo, ShapeRenderer->LineData, sizeof(ShapeRenderer->LineData));
    glDrawArrays(GL_LINES, 0, 2*ShapeRenderer->NumLines);

    ShapeRenderer->NumPoints = 0;
    ShapeRenderer->NumLines = 0;
}

#endif
