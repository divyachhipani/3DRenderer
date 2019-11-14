 

#include <stdlib.h>
#include <glut.h>
#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>  
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace glm;

#define OBJ_POINTS	0
#define OBJ_WIREFRAME	1
#define OBJ_SMOOTHSHADING	2
#define OBJ_FLATSHADING	3

static int obj_mode = 0;

float tip = 0, turn = 0;

float ORG[3] = {0,0,0};
 
float XP[3] = {20,0,0}, XN[3] = {-20,0,0},
      YP[3] = {0,20,0}, YN[3] = {0,-20,0},
      ZP[3] = {0,0,20}, ZN[3] = {0,0,-20};

int cx=0 , cy=0, cz=0;

struct HE_edge
{
    struct HE_vert* end; // vertex at the end of the half-edge
    struct HE_vert* start; // vertex at the start of the half edge
    struct HE_edge* pair; // oppositely oriented adjacent half-edge
    struct HE_face* face; // face the half-edge borders
    struct HE_edge* next; // next half-edge around the face
    struct HE_edge* prev; //previous half edge around the face
};

struct HE_vert
{
    float x;
    float y;
    float z;
    struct HE_edge* edge; // one of the half-edges emantating from the vertex
};

struct HE_face
{
   struct HE_edge* edge; //one of the half edges bordering the face
};


struct Vertex
{
    vec3 position;
    vec3 normal;
};

vector< Vertex > LoadM( istream& in )
{
	
	struct HE_edge *edge = (struct HE_edge*) malloc(sizeof(struct HE_edge));
	struct HE_vert *vert = (struct HE_vert*) malloc(sizeof(struct HE_vert));
	struct HE_face *face = (struct HE_face*) malloc(sizeof(struct HE_face));
	 
    vector< Vertex > verts;

    map< int, vec3 > positions;

    string lineStr;
    while( getline( in, lineStr ) )
    {
        istringstream lineSS( lineStr );
        string lineType;
        lineSS >> lineType;

        // parse vertex line
        if( lineType == "Vertex" )
        {
            int idx = 0;
           // float x = 0, y = 0, z = 0;
            lineSS >> idx >> vert->x >> vert->y >> vert->z;
            positions.insert( make_pair( idx, vec3( vert->x, vert->y, vert->z ) ) );
        }

        // parse face line
        if( lineType == "Face" )
        {
            int indexes[ 3 ] = { 0 };
            int idx = 0;
            lineSS >> idx >> indexes[0] >> indexes[1] >> indexes[2];

            // http://www.opengl.org/wiki/Calculating_a_Surface_Normal
            vec3 U( positions[ indexes[1] ] - positions[ indexes[0] ] );
            vec3 V( positions[ indexes[2] ] - positions[ indexes[0] ] );
            vec3 faceNormal = normalize( cross( U, V ) );

            for( size_t j = 0; j < 3; ++j )
            {
                Vertex vert;
                vert.position = vec3( positions[ indexes[j] ] );
                vert.normal   = faceNormal;
                verts.push_back( vert );
            }
        }
    }

    return verts;
}
// mouse state
int btn;
ivec2 startMouse;
ivec2 startRot, curRot;
ivec2 startTrans, curTrans;

void mouse(int button, int state, int x, int y )
{
    y = glutGet( GLUT_WINDOW_HEIGHT ) - y;
    if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        btn = button;
        startMouse = ivec2( x, y );
        startRot = curRot;
    }
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        btn = button;
        startMouse = ivec2( x, y );
        startTrans = curTrans;
    }
}

void motion( int x, int y )
{
    y = glutGet( GLUT_WINDOW_HEIGHT ) - y;
    ivec2 curMouse( x, y );
    if( btn == GLUT_LEFT_BUTTON )
    {
        curRot = startRot + ( curMouse - startMouse );
    }
    else if( btn == GLUT_RIGHT_BUTTON )
    {
        curTrans = startTrans + ( curMouse - startMouse );
    }
    glutPostRedisplay();
}
// Display grid
void drawGrid()
{
int i;
for(i=-120;i<40;i++)
{
 glPushMatrix();

 // for -z, -x
 if(i>=-120 && i<-100) { glTranslatef(0,0,i+100); glRotatef(-180,0,-1,0);}   
 if(i>=-100 && i<-80) { glTranslatef(i+80,0,0);  glRotatef(-90,0,-1,0);}  
 
 
 // for -z, +x
 if(i>=-80 && i<-60) { glTranslatef(0,0,i+60); glRotatef(0,0,1,0);} 
 if(i>=-60 && i<-40) { glTranslatef(i+60,0,0); glRotatef(90,0,1,0); } 

 // for +z, -x
 if(i>=-40 && i<-20) { glTranslatef(0,0,i+40); glRotatef(180,0,1,0);}  
 if(i>=-20 && i<0) { glTranslatef(i,0,0);   glRotatef(-90,0,1,0);} 
 

 // for +x ,  +z
 if(i>=0 && i<20) {  glTranslatef(0,0,i); }
 if(i>=20 && i<40) { glTranslatef(i-20,0,0);  glRotatef(-90,0,1,0);}


 glBegin(GL_LINES);
 glColor3f(1,1,1); glLineWidth(1);
 glVertex3f(0,-0.1,0); glVertex3f(19,-0.1,0);
 glEnd();
 glPopMatrix();
 }
}

// set projections

void UpdateProjection(GLboolean toggle = GL_FALSE)
{
  static GLboolean s_usePerspective = GL_TRUE;

  // toggle the control variable if appropriate
  if (toggle)
    s_usePerspective = !s_usePerspective;

  // select the projection matrix and clear it out
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // choose the appropriate projection based on the currently toggled mode
    if (s_usePerspective)
  {
    // set up an orthographic projection with the same near clip plane
    glOrtho(-1.0, 1.0, -1.0, 1.0, 5, 100);
  }
	else
  {
    // set the perspective with the appropriate aspect ratio
    glFrustum(-1.0, 1.0, -1.0, 1.0, 5, 100);
  }
	

  // select modelview matrix and clear it out
  glMatrixMode(GL_MODELVIEW);
} // end UpdateProjection



// keyboard options
void mykey(unsigned char key, int x, int y)
{
	switch(key) 
	{case 'p':
		cout << "key 'p' is pressed! draw the object in points" << endl;
		obj_mode = OBJ_POINTS;
		break;
	
	case 'w': 
		cout << "key 'w' is pressed! draw the object in wireframe" << endl;
		obj_mode = OBJ_WIREFRAME;
		break; 
	case 's':
		cout << "key 's' is pressed! draw the object in smooth shading" << endl;
		obj_mode = OBJ_SMOOTHSHADING;
		break;
	case 'f':
		cout << "key 'f' is pressed! draw the object in flat shading" << endl;
		obj_mode = OBJ_FLATSHADING;
		break;
	case 'u':
		cout << "key 'u' is pressed! " << endl;
		cz-=1;
		break;
	case 'j':
		cout << "key 'j' is pressed!" << endl;
		cz+=1;
		break;
	case 'h':
		cout << "key 'h' is pressed!" << endl;
		cx-=1;
		break;
	case 'k':
		cout << "key 'k' is pressed!" << endl;
		cx+=1;
		break;
	case 'y':
		cout << "key 'y' is pressed!" << endl;
		cy-=1;
		break;
	case 'b':
		cout << "key 'b' is pressed!" << endl;
		cy+=1;
		break;
	case 'o':
		cout << "key 'o' is pressed!" << endl;
		UpdateProjection(true);
		break;
	
	}

	// force the redraw function
	glutPostRedisplay(); 
}

vector< Vertex > model;
void display()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    double w = glutGet( GLUT_WINDOW_WIDTH );
    double h = glutGet( GLUT_WINDOW_HEIGHT );
    double ar = w / h;
    // "pan" 
    glTranslatef( curTrans.x / w * 2, curTrans.y / h * 2, 0 );
    gluPerspective( 50, ar, 0.1, 20 );
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0, 0, -10 );
	glTranslatef( cx, cy, cz );
    glPushMatrix();
    // apply mouse rotation
    glRotatef( curRot.x % 360, 0, 1, 0 );
    glRotatef( -curRot.y % 360, 1, 0, 0 );


    glColor3ub( 255, 14, 10 );
	
	// draw model
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof(Vertex), &model[0].position );
    glNormalPointer( GL_FLOAT, sizeof(Vertex), &model[0].normal );
    glDrawArrays( GL_TRIANGLES, 0, model.size() );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

	// for rendering
	if (obj_mode == OBJ_POINTS)
		{
			glPointSize(5.0f);
			glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
		}
	else if (obj_mode == OBJ_WIREFRAME)
		{	
			//glLineWidth(50.0f);
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		
	else if (obj_mode == OBJ_FLATSHADING)
		{	glShadeModel(GL_FLAT);
		}
	else
		{   glShadeModel(GL_SMOOTH);
		}
	
    // draw bounding cube
    glDisable( GL_LIGHTING );
    glColor3ub( 255, 255, 255 );
    glutWireCube( 4 );
    glEnable( GL_LIGHTING );

   
   // draw axes
	 glRotatef( curRot.x % 360, 0, 1, 0 );
    glRotatef( -curRot.y % 360, 1, 0, 0 );

	 glLineWidth (50.0);
 
       glBegin (GL_LINES);
          glColor3f (1,0,0);  glVertex3fv (ORG);  glVertex3fv (XP);    // X axis is red.
          glColor3f (0,1,0);  glVertex3fv (ORG);  glVertex3fv (YP);    // Y axis is green.
          glColor3f (0,0,1);  glVertex3fv (ORG);  glVertex3fv (ZP);    // z axis is blue.
       glEnd();
 
	   // draw grid
	  drawGrid();

    glPopMatrix();

	glutSwapBuffers();
}
// return the x/y/z min/max of some geometry
template< typename Vec >
pair< Vec, Vec > GetExtents
    ( 
    const Vec* pts, 
    size_t stride, 
    size_t count 
    )
{
    typedef typename Vec::value_type Scalar;
    Vec pmin( std::numeric_limits< Scalar >::max() );
    Vec pmax( std::min( std::numeric_limits< Scalar >::min(),
                        (Scalar)-std::numeric_limits< Scalar >::max() ) );

    // find extents
    unsigned char* base = (unsigned char*)pts;
    for( size_t i = 0; i < count; ++i )
    {
        const Vec& pt = *(Vec*)base;
        pmin = glm::min( pmin, pt );
        pmax = glm::max( pmax, pt );
        base += stride;
    }

    return make_pair( pmin, pmax );
}

// centers geometry around the origin
// and scales it to fit in a size^3 box
template< typename Vec > 

void CenterAndScale
    (
    Vec* pts, 
    size_t stride, 
    size_t count,
    const typename Vec::value_type& size
    )
{
    typedef typename Vec::value_type Scalar;

    // get min/max extents
    pair< Vec, Vec > exts = GetExtents( pts, stride, count );

    // center and scale 
    const Vec center = ( exts.first * Scalar( 0.5 ) ) + ( exts.second * Scalar( 0.5f ) );

    const Scalar factor = size / glm::compMax( exts.second - exts.first );
    unsigned char* base = (unsigned char*)pts;
    for( size_t i = 0; i < count; ++i )
    {
        Vec& pt = *(Vec*)base;
        pt = ((pt - center) * factor);
        base += stride;
    }    
}

int main( int argc, char **argv )
{
    ifstream ifile( "bunny.m" );
    model = LoadM( ifile );
    if( model.empty() )
    {
        cerr << "No model found!" << endl;
        return -1;
    }

    CenterAndScale( &model[0].position, sizeof( Vertex ), model.size(), 4 );

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE );
    glutInitWindowSize( 640, 480 );
    glutCreateWindow( "GLUT" );
    glutDisplayFunc( display );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );
	glutKeyboardFunc(mykey);
	glEnable( GL_DEPTH_TEST );

    // set up "headlamp"-like light
    glShadeModel( GL_SMOOTH );
    glEnable( GL_COLOR_MATERIAL );
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    GLfloat position[] = { 0, 0, 1, 0 };
    glLightfv( GL_LIGHT0, GL_POSITION, position );

    glPolygonMode( GL_FRONT, GL_FILL );
    glPolygonMode( GL_BACK, GL_LINE );
    glutMainLoop();
    return 0;
}
