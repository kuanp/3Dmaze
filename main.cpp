using namespace std;

#include "main.h"
#ifdef WIN32
#define ssize_t SSIZE_T
#endif

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cmath>

#include "util.h"
#include "SimpleImage.h"

// file locations
std::string vertexShader;
std::string fragmentShader;

SimpleShaderProgram *shader;
GLuint textureID;

// Storages
vector<Point3f> vertices;
vector<Point3f> normals;
vector<Point2f> textures;
vector<Triangle3f> faces;

// obj processing cases
const int VERT_ONLY = 0;
const int NORMAL = 1;
const int TEXTURE = 2;
const int ALL = 3;
int inputCase;
int xstart;
int ystart;

const int REG_SHADER = 0;
const int CONT_SHADER = 1;
int curShader = REG_SHADER;

Point3f uiTranslations = Point3f(0, 0, -1);
Point2f uiRotations = Point2f(0, 0);
Point3f uiScale = Point3f(5,5,5);
// Renders a vertex. Note, have to be enclosed by begin and end.
void renderVertex(Point3f vert, Point3f n, Point2f t) {
    if (inputCase == ALL || inputCase == TEXTURE) {
        glTexCoord2f(t.x, t.y);
    }

    glNormal3f(n.x, n.y, n.z);
    glVertex3f(vert.x, vert.y, vert.z);
}

void DrawWithShader(){
    shader->Bind();

    // draws whatever was in faces. If faces was empty
    // (i.e. nothing was inputted, just draw the teapot);
    if (faces.size() > 0) {
	glBegin(GL_TRIANGLES);
	for(auto& tri: faces) {
	    renderVertex(tri.a, tri.a_normal, tri.a_texture);
	    renderVertex(tri.b, tri.b_normal, tri.b_texture);
	    renderVertex(tri.c, tri.c_normal, tri.c_texture);
	}
	glEnd();
	glFlush();
    } else {
	glutSolidTeapot(1.0);
    }

    shader->UnBind();
}

void DisplayCallback(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 //   glMatrixMode(GL_PROJECTION);
 //   glLoadIdentity();
 //   glScalef(2, 2, 1);

//    glEnable(GL_COLOR_MATERIAL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(uiTranslations.x, uiTranslations.y, uiTranslations.z);
    glRotatef(uiRotations.x, 0, 1, 0); // rotation around y axis. Left Right
    glRotatef(uiRotations.y, 1, 0, 0); // rotation around x axis. Up down
    //cout << uiRotations.x << "\n";

    //glMatrixMode( GL_PROJECTION );
    //glLoadIdentity();
    //gluPerspective(30.0f, (float)640/(float)480, 0.1f, 100000.f);
    //glScalef(uiScale.x,uiScale.y,uiScale.z);

    glutInitWindowSize(640, 480);

    GLfloat redDiffuseMaterial[] = {1.0, 0.0, 0.0, 1};
    GLfloat whiteSpecularMaterial[] = {1.0, 1.0, 1.0, 1};
    GLfloat redBaseMaterial[] = {0.5, 0.0, 0.0, 1};
    GLfloat shininess = 5.0;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redBaseMaterial);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redDiffuseMaterial);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteSpecularMaterial);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);

    glEnable(GL_FLAT);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat amb_color[] = {0.5, 0.5, 0.5, 1};
    GLfloat diff_color[] = {1, 1, 1, 1};
    GLfloat spec_color[] = {1, 1, 1, 1};
    GLfloat light_position[] = {1, -5, 5, 1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb_color);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff_color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec_color);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    DrawWithShader();
    glutSwapBuffers();
}

void ReshapeCallback(int w, int h){
    glViewport(0, 0, w, h);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(30.0f, (float)w/(float)h, 0.1f, 100000.f);
    glScalef(.25,.25,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void screenshot(){
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float out[3 * w * h];
    RGBColor BG(0,0,0);
    SimpleImage shot(w, h, BG);
    glReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, &out[0]);
    for(int y = 0; y < h; ++y){
	for(int x = 0; x < w; ++x){
	    int index = (3*w*y) + 3*x;
	    float red = out[index];
	    float green = out[index + 1];
	    float blue = out[index + 2];
	    shot.set(x,h-y,RGBColor(red, green, blue));
	}
    }
    shot.save("screenshot.png");
}

void mouseClicked(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
	xstart = x;
	ystart = y;
    }
}

void mouseMoved(int x, int y) {
//    cout << x << " and  " << xstart << "\n";
    if (x != xstart) {
	uiRotations.x += (x - xstart);
    }

    if ( y != ystart) {
	uiRotations.y += (y - ystart);
    }

    xstart = x;
    ystart = y;
}

void KeyCallback(unsigned char key, int x, int y){
    switch(key) {
    case 'a':
	uiTranslations.x += 1;
	break;
    case 'd':
	uiTranslations.x -= 1;
	break;
    case 'e':
	uiTranslations.y += 1;
	break;
    case 'r':
	uiTranslations.y -= 1;
	break;
    case 'w':
	uiTranslations.z += 1;
	break;
    case 's':
	uiTranslations.z -= 1;
	break;
    case 'q':
        exit(0);
	break;
    case 'c':
	// performs uniform change
	shader->Bind();
	if (curShader == REG_SHADER) {
	    shader->SetUniform("shaderOption", 5.f);
	    curShader = CONT_SHADER;
	} else {
	    curShader = REG_SHADER;
	    shader->SetUniform("shaderOption", 0.f);
	}
	shader->UnBind();
	break;
    case ' ':
	screenshot();
	break;
    default:
        break;
    }

}

void Setup(){
    shader = new SimpleShaderProgram();
    shader->LoadVertexShader(vertexShader);
    shader->LoadFragmentShader(fragmentShader);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    shader->SetUniform("shaderOption", (float)curShader);
//    viewPt = Point3f(0,-5,5);
//    viewCtr = Point3f(0.5,0.5,0.5);
//    viewUp = Point3f(0,1,0);
}

// performs computations for normals in case there is none.
Point3f computeNormal(Point3f v1, Point3f v2, Point3f v3){
    Point3f V = v2 - v1;
    Point3f W = v3 - v1;

    Point3f result;
    result.x = (V.y * W.z) - (V.z * W.y);
    result.y = (V.z * W.x) - (V.x * W.z);
    result.z = (V.x * W.y) - (V.y * W.x);

    //cout << "normals" << result.x << " " << result.y << " " << result.z << "\n";
    float scalar = sqrt(pow(result.x, 2) + pow(result.y, 2) + pow(result.z, 2));
    if (scalar != 1) {
	result.x /= scalar;
	result.y /= scalar;
	result.z /= scalar;
    }
    return result;
}

// reads in 2D textures
void readTexture(string filename) {
    SimpleImage texture(filename);
    int w = texture.width();
    int h = texture.height();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, texture.data());
}

// reads a file into several globals
void readFile(string filename) {
    ifstream input(filename);
    string linebuffer;

    while (input && getline(input, linebuffer)) {
	//cout << linebuffer.substr(0,2) << "\n";
	if(linebuffer.length() == 0) continue;
	if(linebuffer[0] == '#') continue;
	if(linebuffer.substr(0,2) == "v ") {
	    Point3f v;
	    //cout << "actual line: " << linebuffer;
	    sscanf(linebuffer.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
	    //cout << " " << v.x << " " << v.y << " " << v.z << "\n";
	    vertices.push_back(v);
	} else if (linebuffer.substr(0, 2) == "vn") {
	    Point3f v;
	    sscanf(linebuffer.c_str(), "vn %f %f %f", &v.x, &v.y, &v.z);
	    normals.push_back(v);
	} else if (linebuffer.substr(0, 2) == "vt") {
	    Point2f v;
	    sscanf(linebuffer.c_str(), "vt %f %f", &v.x, &v.y);
	    textures.push_back(v);
	} else if (linebuffer.substr(0, 2) == "vp") {
	    //no instruction yet
	} else if (linebuffer.substr(0, 2) == "f ") {
	    Triangle3f tri;

	    char s1[20], s2[20], s3[20];
//	    cout << "actual line: " << linebuffer;
	    // cout << v1<< v2<< v3<< t1<< t2<< t3<< n1 << n2 << n3 <<" \n";
	    sscanf(linebuffer.c_str(), "f %*d%s %s %s", s1, s2, s3);
	    // cout << s1 << s2 << s3 << "\n";
	    string test = (string) s1;

	    // do test to determine the format of the faces.
	    if (s1[0] == '/') {
		if (s1[1] == '/') {
//		    cout << "has normalz\n";
		    inputCase = NORMAL;

		    int a, b, c, n1, n2, n3;
		    sscanf(linebuffer.c_str(), "f %d//%d %d//%d %d//%d",
			    &a, &n1, &b, &n2, &c, &n3);
		    tri = Triangle3f(vertices[a - 1], vertices[b - 1],
			    vertices[c-1]);
		    tri.normal_verts(normals[n1 -1], normals[n2 -1],
			    normals[n3 -1]);
		} else if (test.find('/', 1) != -1){
//		    cout << "all three here \n";
		    inputCase = ALL;
		    int a, b, c, t1, t2, t3, n1, n2, n3;
		    sscanf(linebuffer.c_str(),
			    "f %d/%d/%d %d/%d/%d %d/%d/%d",
			    &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);

		    tri = Triangle3f(vertices[a-1], vertices[b-1],
			    vertices[c-1]);
		    tri.normal_verts(normals[n1-1], normals[n2-1],
			    normals[n3-1]);
		    tri.texture_verts(textures[t1-1], textures[t2-1],
			    textures[t3-1]);
		} else {
//		    cout << "no normal \n";
		    inputCase = TEXTURE;

		    int a, b, c, t1, t2, t3;
		    sscanf(linebuffer.c_str(),
			    "f %d/%d %d/%d %d/%d",
			    &a, &t1, &b, &t2, &c, &t3);

		    tri = Triangle3f(vertices[a-1], vertices[b-1],
			    vertices[c-1]);
		    tri.texture_verts(textures[t1-1], textures[t2-1],
			    textures[t3-1]);
		    // computes a normal.
		    Point3f normal = computeNormal(vertices[a-1],
			    vertices[b-1], vertices[c-1]);
		    tri.normal_verts(normal, normal, normal);
		}
	    } else {
//		cout << "only vertex data\n";
		inputCase = VERT_ONLY;

		int a, b, c;
		sscanf(linebuffer.c_str(), "f %d %d %d", &a, &b, &c);
//		cout << a << b << c <<"\n";
		tri = Triangle3f(vertices[a-1], vertices[b-1],
			vertices[c-1]);

		// computes a normal.
		Point3f normal = computeNormal(vertices[a-1],
			vertices[b-1], vertices[c-1]);
		tri.normal_verts(normal, normal, normal);
	    }

	    // ready for render!
	    faces.push_back(tri);
	}
    }
}

int main(int argc, char** argv){
    if(argc < 3 || argc > 5){
        printf("usage: ./hw5 <vertex shader> <fragment shader> [optional] <obj-file> <texture> \n");
        return 0;
    }

    vertexShader   = std::string(argv[1]);
    fragmentShader = std::string(argv[2]);
    if (argc > 3) {
	readFile(string(argv[3]));
	if (argc > 4) {
	    readTexture(string(argv[4]));
	}
    }


    // Initialize GLUT.
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(20, 20);
    glutInitWindowSize(640, 480);
    glutCreateWindow("CS148 Assignment 5");

    //
    // Initialize GLEW.
    //
#if !defined(__APPLE__) && !defined(__linux__)
    glewInit();
    if(!GLEW_VERSION_2_0) {
        printf("Your graphics card or graphics driver does\n"
               "\tnot support OpenGL 2.0, trying ARB extensions\n");

        if(!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader) {
            printf("ARB extensions don't work either.\n");
            printf("\tYou can try updating your graphics drivers.\n"
                   "\tIf that does not work, you will have to find\n");
            printf("\ta machine with a newer graphics card.\n");
            exit(1);
        }
    }
#endif

    Setup();
    glutDisplayFunc(DisplayCallback);
    glutReshapeFunc(ReshapeCallback);
    glutMouseFunc(mouseClicked);
    glutMotionFunc(mouseMoved);
    glutKeyboardFunc(KeyCallback);
    glutIdleFunc(DisplayCallback);
    glutMainLoop();
    return 0;
}


