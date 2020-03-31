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

GPUProgram gpuProgram; // vertex and fragment shaders

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

	Circle(vec2 centre, float r, float fromAngle = -M_PI, float toAngle = M_PI) : centre(centre), r(r), fromAngle(fromAngle), toAngle(toAngle) {}

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
		float pathLen = 0;

		for (float angle = fromAngle; angle < toAngle; angle += angleStep) {
			vec2 c(cos(angle) * r + centre.x, sin(angle) * r + centre.y);
			if (!coords.empty()) {
				vec2 prev = coords[coords.size() - 1];
				pathLen += length(prev - c) / (1 - dot(c, c));
			}
			coords.push_back(c);
		}

		printf("oldalhossz: %f\n", pathLen);

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

class Buffer2F {
	unsigned int vao = 0, vbo = 0;
	std::vector<vec2> coords;

public:
	void add(vec2 v) {
		deleteBuffer();
		coords.push_back(v);
	}

	const std::vector<vec2>& getCoords() const {
		return coords;
	}

	void clear() {
		coords.clear();
		deleteBuffer();
	}

	void createBuffer() {
		deleteBuffer();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(vec2), &coords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	void deleteBuffer() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		vao = vbo = 0;
	}

	void draw(GLenum mode) const {
		glBindVertexArray(vao);
		glDrawArrays(mode, 0, coords.size());
	}

	~Buffer2F() {
		deleteBuffer();
	}
};

class Polygon {
	Buffer2F boundsBuffer;
	Buffer2F fillBuffer;

public:

	void addLine(vec2 p1, vec2 p2) {
		vec2 centre;

		float a = p1.x;
		float b = p1.y;
		float c = p2.x - p1.x;
		float d = p2.y - p1.y;
		float e = (length(p1) + 1) / 2;
		float f = dot((p1 + p2) / 2, p2 - p1);

		float det = a * d - b * c;
		centre.x = d * e + -b * f;
		centre.y = -c * e + a * f;
		centre = centre / det;

		float r = length(centre - p1);

		vec2 cp1 = p1 - centre;
		vec2 cp2 = p2 - centre;
		float a1 = atan2(cp1.y, cp1.x);
		float a2 = atan2(cp2.y, cp2.x);

		if (abs(a2 - a1) >= M_PI)
			if (a1 < a2)
				a1 +=  2 * M_PI;
			else
				a2 +=  2 * M_PI;

		float pathLen = 0;

		bool incr = a1 < a2;
		for (float angle = a1; incr ? angle < a2 : angle > a2; angle += incr ? angleStep : -angleStep) {
			vec2 c(cos(angle) * r + centre.x, sin(angle) * r + centre.y);
			if (!boundsBuffer.getCoords().empty()) {
				vec2 prev = boundsBuffer.getCoords()[boundsBuffer.getCoords().size() - 1];
				pathLen += length(prev - c) / (1 - dot(c, c));
			}
			boundsBuffer.add(c);
		}

		printf("oldalhossz: %f\n", pathLen);
	}

	void clear() {
		boundsBuffer.clear();
		fillBuffer.clear();
	}

	void createBuffer() {
		boundsBuffer.createBuffer();
		fillBuffer.createBuffer();
	}

	void draw() const {
		gpuProgram.setUniform(vec3(0, 0, 1), "color");
		boundsBuffer.draw(GL_LINE_STRIP);
		gpuProgram.setUniform(vec3(0, 1, 0), "color");
		fillBuffer.draw(GL_TRIANGLES);
	}

	~Polygon() {
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

Circle circle(vec2(0, 0), 1);
std::vector<vec2> clickPoints;
Polygon polygon;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	circle.createBuffer();
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
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
	polygon.draw();

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
			polygon.clear();
		}

		clickPoints.push_back(vec2(cX, cY));
		if (clickPoints.size() == 3) {
			polygon.addLine(clickPoints[0], clickPoints[1]);
			polygon.addLine(clickPoints[1], clickPoints[2]);
			polygon.addLine(clickPoints[2], clickPoints[0]);
			polygon.createBuffer();

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
