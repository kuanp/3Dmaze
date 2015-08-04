#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

using namespace std;
#include <fstream>
#include <iostream>
#include <vector>
#include "SimpleImage.h"
#include "util.h"

const int DISP_SIZE = 512;
const int VERT_ONLY = 0;
const int NORMAL = 1;
const int TEXTURE = 2;
const int ALL = 3;

int xstart;
int ystart;

Point3f viewPt, viewCtr, viewUp;
vector<Point3f> vertices;
vector<Point3f> normals;
vector<Point2f> textures;
vector<Triangle3f> faces;
int inputCase;
GLuint textureID;

void readTexture(string filename) {
    SimpleImage texture(filename);
    int w = texture.width();
    int h = texture.height();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, texture.data());
}

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
	    cout << "actual line: " << linebuffer;
	    // cout << v1<< v2<< v3<< t1<< t2<< t3<< n1 << n2 << n3 <<" \n";
	    sscanf(linebuffer.c_str(), "f %*d%s %s %s", s1, s2, s3);
	    // cout << s1 << s2 << s3 << "\n";
	    string test = (string) s1;

	    // do test to determine the format of the faces.
	    if (s1[0] == '/') {
		if (s1[1] == '/') {
		    cout << "has normalz\n";
		    inputCase = NORMAL;

		    int a, b, c, n1, n2, n3;
		    sscanf(linebuffer.c_str(), "f %d//%d %d//%d %d//%d",
			    &a, &n1, &b, &n2, &c, &n3);
		    tri = Triangle3f(vertices[a - 1], vertices[b - 1], vertices[c-1]);
		    tri.normal_verts(normals[n1 -1], normals[n2 -1], normals[n3 -1]);
		} else if (test.find('/', 1) != -1){
		    cout << "all three here \n";
		    inputCase = ALL;
		    int a, b, c, t1, t2, t3, n1, n2, n3;
		    sscanf(linebuffer.c_str(),
			    "f %d/%d/%d %d/%d/%d %d/%d/%d",
			    &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);

		    tri = Triangle3f(vertices[a-1], vertices[b-1], vertices[c-1]);
		    tri.normal_verts(normals[n1-1], normals[n2-1], normals[n3-1]);
		    tri.texture_verts(textures[t1-1], textures[t2-1], textures[t3-1]);
		} else {
		    cout << "no normal \n";
		    inputCase = TEXTURE;

		    int a, b, c, t1, t2, t3;
		    sscanf(linebuffer.c_str(),
			    "f %d/%d %d/%d %d/%d",
			    &a, &t1, &b, &t2, &c, &t3);

		    tri = Triangle3f(vertices[a-1], vertices[b-1], vertices[c-1]);
		    tri.texture_verts(textures[t1-1], textures[t2-1], textures[t3-1]);
		    // computes a normal.
		    Point3f normal = computeNormal(vertices[a-1], vertices[b-1], vertices[c-1]);
		    tri.normal_verts(normal, normal, normal);
		}
	    } else {
		cout << "only vertex data\n";
		inputCase = VERT_ONLY;

		int a, b, c;
		sscanf(linebuffer.c_str(), "f %d %d %d", &a, &b, &c);
//		cout << a << b << c <<"\n";
		tri = Triangle3f(vertices[a-1], vertices[b-1], vertices[c-1]);

		// computes a normal.
		Point3f normal = computeNormal(vertices[a-1], vertices[b-1], vertices[c-1]);
		tri.normal_verts(normal, normal, normal);
	    }

	    // ready for render!
	    faces.push_back(tri);
	}
    }
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

void initOpenGL () {
    // Polygon mode stuff
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    viewPt = Point3f(0,-5,5);
    viewCtr = Point3f(0.5,0.5,0.5);
    viewUp = Point3f(0,1,0);

    // view and other things.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1,1,-1,1,1,10);
//    viewPt = Point3f(0,0,-2);
//    viewCtr = Point3f(0,0,0);
//    viewUp = Point3f(0,1,0);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(viewPt.x, viewPt.y, viewPt.z,
	    viewCtr.x, viewCtr.y, viewCtr.z,
	    viewUp.x, viewUp.y, viewUp.z);
}

void mouseClicked(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
	xstart = x;
	ystart = y;
    }
}

void mouseMoved(int x, int y) {
    glMatrixMode(GL_MODELVIEW);

    if (x != xstart) {
	glTranslatef(viewCtr.x, viewCtr.y, viewCtr.z);
	glRotatef(xstart - x, viewUp.x, viewUp.y, viewUp.z);
	glTranslatef(-viewCtr.x, -viewCtr.y, -viewCtr.z);
    }

    if ( y != ystart) {
	glTranslatef(viewCtr.x, viewCtr.y, viewCtr.z);
	glRotatef(ystart - y, 1, 0, 0);
	glTranslatef(-viewCtr.x, -viewCtr.y, -viewCtr.z);
    }

    glutPostRedisplay();
    xstart = x;
    ystart = y;
}

void keyPressed(unsigned char key, int x, int y) {
    glMatrixMode(GL_PROJECTION);
    if (key == 'w') {
	glScalef(2, 2, 1);
    } else if (key == 's') {
	glScalef(.5, .5, 1);
    } else if (key == ' ') {
	screenshot();
    }
    glutPostRedisplay();
}

void renderVertex(Point3f vert, Point3f n, Point2f t) {
//	cout << " x: " << vert.x << " y: " << vert.y <<
//	    " z: " << vert.z << "\n";


    if (inputCase == ALL || inputCase == TEXTURE) {
//        cout << " u: " << t.x << " v: " << t.y << "\n";
        glTexCoord2f(t.x, t.y);
    }

    //cout << "normals" << n.x << " " << n.y << " " << n.z << "\n";
    glNormal3f(n.x, n.y, n.z);
    glVertex3f(vert.x, vert.y, vert.z);
}

void display() {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    //def lights
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);

    GLfloat light_ambient[] = {0.3, 0.3, 0.3, 1};
    GLfloat light_diffuse[] = {.7, .7, .7, 1};
//    GLfloat light_specular[] = {1, 1, 1, 1};
    GLfloat light_position[] = {0, -10, 4, 0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    //Textures
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_TRIANGLES);
//    glColor3f(.8,.8,.8);
//    cout << faces.size() << " \n";
    for(auto& tri: faces) {
	renderVertex(tri.a, tri.a_normal, tri.a_texture);
	renderVertex(tri.b, tri.b_normal, tri.b_texture);
	renderVertex(tri.c, tri.c_normal, tri.c_texture);
    }
    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
	cout << "Specify obj file to import\n";
	return 0;
    }
    readFile(string(argv[1]));
    glutInit(&argc,argv);
    glutInitWindowSize(DISP_SIZE, DISP_SIZE);
    glutInitDisplayMode(GLUT_RGB);
    glutCreateWindow("Hello World");
    initOpenGL();
    if (argc == 3) {
	readTexture(string(argv[2]));
    }

    glutDisplayFunc(display);
    glutMouseFunc(mouseClicked);
    glutKeyboardFunc(keyPressed);
    glutMotionFunc(mouseMoved);
    glutMainLoop();
    return 0;
}