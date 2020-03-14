//=============================================================================================
// Mintaprogram: Zold haromszog. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    :
// Neptun :
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const float angleStep = 0.001f;

class Circle {
	vec2 centre;
	float r;
	float fromAngle, toAngle;

	unsigned int vao = 0, vbo = 0;
	int coordCount = 0;

	Circle(const Circle& c) = default;
	Circle& operator=(const Circle& c) = default;

public:

	Circle(vec2 centre, float r, float fromAngle = -M_PI, float toAngle = M_PI) : centre(centre), r(r), fromAngle(fromAngle), toAngle(toAngle) {
		if (toAngle < fromAngle)
			toAngle += 2 * M_PI;
	}

	Circle(Circle&& c) : Circle(c) {
		c.vao = 0;
		c.vbo = 0;
	}

	Circle& operator=(Circle&& c) {
		*this = c;
		c.vao = 0;
		c.vbo = 0;
		return *this;
	}

	void createBuffer() {
		std::vector<vec2> coords;

		for (float angle = fromAngle; angle < toAngle; angle += angleStep) {
			vec2 c(cos(angle) * r + centre.x, sin(angle) * r + centre.y);
			coords.push_back(c);
		}

		coordCount = coords.size();

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(vec2), &coords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	void draw() const {
		glBindVertexArray(vao);
		glDrawArrays(GL_LINE_STRIP, 0, coordCount);
	}

	vec2 getCentre() const {
		return centre;
	}

	float getRadius() const {
		return r;
	}

	unsigned int getVBO() const {
		return vbo;
	}

	int getCoordCount() const {
		return coordCount;
	}

	~Circle() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
};

class Triangle {
	unsigned int vao = 0, vbo = 0;
	std::vector<int> coordCounts;

	Triangle(const Triangle& c) = default;
	Triangle& operator=(const Triangle& c) = default;

public:
	Triangle() {}

	void build(std::vector<Circle>& circles, std::vector<vec2>& points) {
		vec2 normal1 = points[0] - circles[0].getCentre();
		vec3 line1(normal1.x, normal1.y, -dot(normal1, points[0]));

		vec2 normal2 = points[1] - circles[1].getCentre();
		vec3 line2(normal2.x, normal2.y, -dot(normal2, points[1]));

		vec3 centerPointv3 = cross(line1, line2);
		vec2 centerPoint = vec2(centerPointv3.x / centerPointv3.z, centerPointv3.y / centerPointv3.z);

		int coordCount = 0;
		for (auto& circle : circles)
			coordCount += circle.getCoordCount() + 1;

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
		glBufferData(GL_COPY_WRITE_BUFFER, coordCount * sizeof(vec2), nullptr, GL_DYNAMIC_DRAW);

		int offset = 0;
		coordCounts.clear();
		for (auto& circle : circles) {
			int len;

			len = sizeof(vec2);
			glBufferSubData(GL_COPY_WRITE_BUFFER, offset, len, &centerPoint);
			offset += len;

			len = circle.getCoordCount() * sizeof(vec2);
			glBindBuffer(GL_COPY_READ_BUFFER, circle.getVBO());
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, len);
			offset += len;

			coordCounts.push_back(circle.getCoordCount() + 1);
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	void draw() const {
		glBindVertexArray(vao);
		int start = 0;
		for (int coordCount : coordCounts) {
			glDrawArrays(GL_TRIANGLE_FAN, start, coordCount);
			start += coordCount;
		}
	}

	Triangle(Triangle&& t) : Triangle(t) {
		t.vao = 0;
		t.vbo = 0;
	}

	Triangle& operator=(Triangle&& t) {
		*this = t;
		t.vao = 0;
		t.vbo = 0;
		return *this;
	}

	~Triangle() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
};

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders

Circle circle(vec2(0, 0), 1);
std::vector<Circle> clickCircles;
std::vector<vec2> clickPoints;
Triangle triangle;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	circle.createBuffer();
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

Circle calcCircleLine(vec2 p1, vec2 p2) {
	float lengthDiff = length(p2) - length(p1);
	float xDiff = p2.x - p1.x;
	float yDiff = p2.y - p1.y;

	vec2 c;

	c.y = (p1.x * lengthDiff - xDiff * (1 + length(p1))) /
	      (p1.x * yDiff - p1.y * xDiff) / 2;

	c.x = (-c.y * yDiff + lengthDiff / 2) / xDiff;

	float r = length(c - p1);

	vec2 cp1 = p1 - c;
	vec2 cp2 = p2 - c;
	float a1 = atan2(cp1.y, cp1.x);
	float a2 = atan2(cp2.y, cp2.x);

	return Circle(vec2(c.x, c.y), r, abs(a2 - a1) < M_PI ? std::min(a1, a2) : std::max(a1, a2), abs(a2 - a1) < M_PI ? std::max(a1, a2) : std::min(a1, a2));
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

	float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix,
							  0, 1, 0, 0,    // row-major!
							  0, 0, 1, 0,
							  0, 0, 0, 1 };

	int location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

	gpuProgram.setUniform(vec3(1, 1, 1), "color");
	circle.draw();
	gpuProgram.setUniform(vec3(0, 1, 0), "color");
	triangle.draw();
	gpuProgram.setUniform(vec3(0, 0, 1), "color");
	for (auto& clickCircle : clickCircles)
		clickCircle.draw();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;

	if (state == GLUT_DOWN) {
		if (clickPoints.size() >= 3) {
			clickPoints.clear();
			clickCircles.clear();
		}

		clickPoints.push_back(vec2(cX, cY));
		if (clickPoints.size() == 3) {
			clickCircles.push_back(calcCircleLine(clickPoints[0], clickPoints[1]));
			clickCircles.push_back(calcCircleLine(clickPoints[1], clickPoints[2]));
			clickCircles.push_back(calcCircleLine(clickPoints[2], clickPoints[0]));
			for (auto& clickCircle : clickCircles)
				clickCircle.createBuffer();

			triangle.build(clickCircles, clickPoints);
			glutPostRedisplay();
		}
	}

	const char * buttonStat;
	switch (state) {
	case GLUT_DOWN: buttonStat = "pressed"; break;
	case GLUT_UP:   buttonStat = "released"; break;
	}

	switch (button) {
	case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
