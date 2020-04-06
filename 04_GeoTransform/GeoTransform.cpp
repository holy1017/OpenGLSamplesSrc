#include	<iostream>
#include    <cmath>
using namespace std;

//#define FREEGLUT_STATIC
//#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "gTransform.h"

struct VecColor4f {	//  ���� ǥ���ϱ� ���� ����ü
    float r, g, b, a;
    VecColor4f() {}
    VecColor4f(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
};

struct Vertex {
    Vec3f       mCoord;
    VecColor4f  mColor;
};

enum { TRIANGLE, N_VBOs };
GLuint VBO[N_VBOs];			// ������ ���� ��ü
GLuint uGTransLocation;     // ���Ϻ�ȯ ��� �������� ��ġ

static const char* pVS =	// ������ ���̴� �ҽ�
"#version 430                                                      \n"
"layout (location = 0) in vec3 Position;                           \n"
"layout (location = 1) in vec4 VColor;                             \n"
"out vec4 Color;                                                   \n"
"uniform    mat4    gTransform;                                    \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    gl_Position = gTransform * vec4(Position, 1.0);               \n"
"    Color = VColor;                                               \n"
"}";

static const char* pFS =	// �����׸�Ʈ ���̴� �ҽ�
"#version 430                                                      \n"
"in vec4 Color;                                                    \n"
"out vec4 FragColor;                                               \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    FragColor = Color;                                            \n"
"}";

static void AddShader(GLuint shaderProg, const char* pShaderSrc, GLint ShaderType)
{   // ���̴� ����
    GLuint shader = glCreateShader(ShaderType);
    if (!shader) {
        cerr << "���� - Shader ����(" << ShaderType << ")" << endl;
        exit(0);
    }
    // ���̴� ������
    const GLchar* src[1] = { pShaderSrc };
    const GLint len[1] = { strlen(pShaderSrc) };
    glShaderSource(shader, 1, src, len);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {		// ������ ���� �߻�
        GLchar infoLog[256];
        glGetShaderInfoLog(shader, 256, NULL, infoLog);
        cerr << "���� - Shader ������(" << ShaderType << "): " << infoLog << endl;
        exit(1);
    }
    // ���̴� ���α׷��� �����ϵ� ���̴��� �߰�
    glAttachShader(shaderProg, shader);
}

static void SetUpShaders()
{   // ���̴� ���α׷� ��ü ����
    GLuint shaderProg = glCreateProgram();
    if (!shaderProg) {
        cerr << "���� - Shader ���α׷� ����" << endl;
        exit(1);
    }

    // ������ ���̴� �� �����׸�Ʈ ���̴� ����
    AddShader(shaderProg, pVS, GL_VERTEX_SHADER);
    AddShader(shaderProg, pFS, GL_FRAGMENT_SHADER);

    GLint success = 0;
    GLchar errLog[256];

    glLinkProgram(shaderProg);		// ���̴� ���α׷� ��ũ
    glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProg, sizeof(errLog), NULL, errLog);
        cerr << "���� - Shader ���α׷� ��ũ: " << errLog << endl;
        exit(1);
    }

    glValidateProgram(shaderProg);	// ���α׷� ��ü�� ��ȿ���� �˻�
    glGetProgramiv(shaderProg, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProg, sizeof(errLog), NULL, errLog);
        cerr << "Invalid shader program: " << errLog << endl;
        exit(1);
    }
    glUseProgram(shaderProg);		// ���� ���̴� ���α׷� ��ü�� ����
    uGTransLocation = glGetUniformLocation(shaderProg, "gTransform");
    if (uGTransLocation == 0xFFFFFFFF) {
        cerr << "Uniform location not found!!" << endl;
        exit(1);
    }
}

static void RenderCB()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);	// ������� ȭ�� ����

    const   float   PI = 3.141593f;
    TransMat4f  tTrans, sTrans, rTrans;
    static  float   rot = 0.0f;
    float   dx, dy, s;
    float   aRad = rot * PI / 180.0f;
    dx = cos(aRad) * 0.5f;
    dy = s = sin(aRad) * 0.5f;
    rTrans.rotateZ(aRad);
    sTrans.scale(Vec3f((s + 1.0f)*0.1f, (s + 1.0f)*0.1f, (s + 1.0f)*0.1f));
    tTrans.translate(Vec3f(dx, dy, 0.0f));
    if ((rot += 0.5) >= 360.0f)  rot = 0.0f;

    TransMat4f  tMat(tTrans * sTrans * rTrans);

    glUniformMatrix4fv(uGTransLocation, 1, GL_TRUE, tMat.getPMat());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[TRIANGLE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vec3f));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);

    glLineWidth(5.0f);
    glDrawArrays(GL_LINE_LOOP, 0, 3);
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glutSwapBuffers();
}

static void InitVBOs()
{
    Vertex Vertices[3] = {		//�ﰢ���� ������ ��ǥ �� ��
        { Vec3f(-5.0f, -5.0f, 0.0f), VecColor4f(1.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(5.0f, -5.0f, 0.0f), VecColor4f(0.0f, 1.0f, 0.0f, 1.0f) },
        { Vec3f(0.0f, 5.0f, 0.0f), VecColor4f(0.0f, 0.0f, 1.0f, 1.0f) } };
    // ������ ���۸� �����Ͽ� �ﰢ���� ������ ���� ����
    glGenBuffers(N_VBOs, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[TRIANGLE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 100);
    glutInitWindowSize(400, 400);
    glutCreateWindow("���Ϻ�ȯ");

    GLenum s = glewInit();
    if (s != GLEW_OK) {
        cerr << "���� - " << glewGetErrorString(s) << endl;
        return 1;
    }

    SetUpShaders();
    InitVBOs();

    glutDisplayFunc(RenderCB);
    glutIdleFunc(RenderCB);
    glutMainLoop();

    return 0;
}
