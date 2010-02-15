/**
 * Defines all the basic datastructures, they will be imported from the language definition file "std.dll"
 */

/**
 @brief Defines a simple 3x3 Matrix
 */
class       Matrix3
{
   void Matrix3( float a00, float a01, float a02,
                 float a10, float a11, float a12,
                 float a20, float a21, float a22 );
   
   float operator()( int j, int i );
}

/**
 @brief Defines a simple 4x4 Matrix
 */
class       Matrix4
{
   void Matrix4( float a00, float a01, float a02, float a03,
                 float a10, float a11, float a12, float a13,
                 float a20, float a21, float a22, float a23,
                 float a30, float a31, float a32, float a33 );
   
   float operator()( int j, int i );
}

/**
 @brief Defines a simple 3-values vector
 */
class       Vector3i
{
   void Vector3i( int i, int j, int k );
   int operator[]( int index );
}

/**
 @brief Defines a simple 3-values vector
 */
class       Vector3f
{
   void Vector3i( int i, int j, int k );
   float operator[]( int index );
}

/**
 @brief Defines a volume datatype
 */
class       Volume
{
   float voxel( Vector3i position );
   void setSpacing( Vector3f spacing );
   void setRotation( Matrix3 rotation );
   void setTranslation( Vector3f translation );
   
   Vector3f getSpacing( Vector3f spacing );
   Vector3f getTranslation( Vector3f translation );
   Matrix3  getRotation( Matrix3 rotation );
   
   Matrix4  pst;
   Vector3i size;
}

void print( string val );
int toInt( float val );
int toInt( string val );
float toFloat( int val );
float toFloat( string val );
string toString( int val );
string toString( float val );