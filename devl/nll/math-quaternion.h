#ifndef NLL_MATH_QUATERNION_H_
# define NLL_MATH_QUATERNION_H_

namespace nll
{
namespace core
{
   /**
    @brief Define a quaternion class. Internally stored as (w, i, j, k)
    @note this class does not handle quaternion with scaling components i.e. it will
          only be used to compute rotations
    */
   class Quaternion : public StaticVector<double, 4>
   {
   public:
      /**
       @brief create a quaternion from a 3x3 <b>rotation</b> matrix only (not scaling/shearing)
       */
      template <class Mapper, class T2>
      Quaternion( const Matrix<T2, Mapper>& a )
      {
         ensure( a.sizex() == 3 && a.sizey() == 3, "error only 3x3 rotation matrix handled" );
         
         double trace = a( 0 ,0 ) + a( 1, 1 ) + a( 2, 2 ) + 1;
         if( trace > 1e-6 )
         {
            double s = 0.5 / sqrt( trace );
            _buffer[ 0 ] = 0.25 / s;
            _buffer[ 1 ] = ( a( 2, 1 ) - a( 1, 2 ) ) * s;
            _buffer[ 2 ] = ( a( 0, 2 ) - a( 2, 0 ) ) * s;
            _buffer[ 3 ] = ( a( 1, 0 ) - a( 0, 1 ) ) * s;
         } else {
            if ( a( 0, 0 ) > a( 1, 1 ) && a( 0, 0 ) > a( 2, 2 ) )
            {
               double s = 2 * sqrt( 1 + a( 0, 0 ) - a( 1, 1 ) - a( 2, 2 ) );
               _buffer[ 0 ] = ( a( 2, 1 ) - a( 1, 2 ) ) / s;
               _buffer[ 1 ] = 0.25 * s;
               _buffer[ 2 ] = ( a(0, 1 ) + a( 1, 0 ) ) / s;
               _buffer[ 3 ] = ( a(0, 2 ) + a( 2, 0 ) ) / s;
            } else if ( a( 1, 1 ) > a( 2, 2 ) )
            {
               double s = 2 * sqrt( 1 + a( 1, 1 ) - a( 0, 0 ) - a( 2, 2 ) );
               _buffer[ 0 ] = ( a( 0, 2 ) - a( 2, 0 ) ) / s;
               _buffer[ 1 ] = ( a( 0, 1 ) + a( 1, 0 ) ) / s;
               _buffer[ 2 ] = 0.25 * s;
               _buffer[ 3 ] = ( a( 1, 2 ) + a( 2, 1 ) ) / s;
            } else {
               double s = 2 * sqrt( 1 + a( 2, 2 ) - a( 0, 0 ) - a( 1, 1 ) );
               _buffer[ 0 ] = ( a( 1, 0 ) - a( 0, 1 ) ) / s;
               _buffer[ 1 ] = ( a( 0, 2 ) + a( 2, 0 ) ) / s;
               _buffer[ 2 ] = ( a( 1, 2 ) + a( 2, 1 ) ) / s;
               _buffer[ 3 ] = 0.25 * s;
            }
         }
      }

      /**
       @brief create a quaternion to an axis-angle representation: the axis defines the rotation axis.
       @param angle angle in radian
       */
      template <class T>
      Quaternion( const StaticVector<T, 3>& axis, double angle )
      {
         _fromAngleAxis( angle, axis[ 0 ], axis[ 1 ], axis[ 2 ] );
      }

      /**
       @brief create a quaternion to an axis-angle representation.
       @param angle angle in radian
       */
      template <class T, class Mapper>
      Quaternion( const Buffer1D<T, Mapper>& axis, double angle )
      {
         ensure( axis.size() == 3, "the axis must be a 3D coordinate" );
         _fromAngleAxis( angle, axis[ 0 ], axis[ 1 ], axis[ 2 ] );
      }

      /**
       @brief init the quaternion with default values
       */
      Quaternion( double w, double i, double j, double k )
      {
         _buffer[ 0 ] = w;
         _buffer[ 1 ] = i;
         _buffer[ 2 ] = j;
         _buffer[ 3 ] = k;
      }

      /**
       @biref export the quaternion as a rotation matrix.
       */
      template <class T, class Mapper>
      void toMatrix( Matrix<T, Mapper>& out ) const
      {
         out = Matrix<T, Mapper>( 3, 3 );
         double xx = _buffer[ 1 ] * _buffer[ 1 ];
         double xy = _buffer[ 1 ] * _buffer[ 2 ];
         double xz = _buffer[ 1 ] * _buffer[ 3 ];
         double xw = _buffer[ 1 ] * _buffer[ 0 ];

         double yy = _buffer[ 2 ] * _buffer[ 2 ];
         double yz = _buffer[ 2 ] * _buffer[ 3 ];
         double yw = _buffer[ 2 ] * _buffer[ 0 ];

         double zz = _buffer[ 3 ] * _buffer[ 3 ];
         double zw = _buffer[ 3 ] * _buffer[ 0 ];

         out( 0, 0 ) = 1 - 2 * ( yy + zz );
         out( 0, 1 ) =     2 * ( xy - zw );
         out( 0, 2 ) =     2 * ( xz + yw );

         out( 1, 0 ) =     2 * ( xy + zw );
         out( 1, 1 ) = 1 - 2 * ( xx + zz );
         out( 1, 2 ) =     2 * ( yz - xw );

         out( 2, 0 ) =     2 * ( xz - yw );
         out( 2, 1 ) =     2 * ( yz + xw );
         out( 2, 2 ) = 1 - 2 * ( xx + yy );
      }

      /** 
       @brief to axis-angle representation. The quaternion must be in a normalized form.
       */
      template <class T>
      void toAxisAngle( StaticVector<T, 3>& axis, double& angle )
      {
         angle = 2 * acos( _buffer[ 0 ] );
         if ( fabs( _buffer[ 0 ] - 1 ) <= std::numeric_limits<double>::epsilon() )
         {
            // division by zero, any axis will do it!
            axis[ 0 ] = 0;
            axis[ 1 ] = 0;
            axis[ 2 ] = 1;
         } else {
            double w2 = _buffer[ 0 ] * _buffer[ 0 ];
            axis[ 0 ] = _buffer[ 1 ] / sqrt( 1 - w2 );
            axis[ 1 ] = _buffer[ 2 ] / sqrt( 1 - w2 );
            axis[ 2 ] = _buffer[ 3 ] / sqrt( 1 - w2 );
         }
      }

      void mul( const Quaternion& b )
      {
         Quaternion& a = *this;
         Quaternion res( a[ 0 ] * b[ 0 ] - a[ 1 ] * b[ 1 ] - a[ 2 ] * b[ 2 ] - a[ 3 ] * b[ 3 ],
                         a[ 1 ] * b[ 0 ] + a[ 0 ] * b[ 1 ] + a[ 2 ] * b[ 3 ] - a[ 3 ] * b[ 2 ],
                         a[ 0 ] * b[ 2 ] - a[ 1 ] * b[ 3 ] + a[ 2 ] * b[ 0 ] + a[ 3 ] * b[ 1 ],
                         a[ 0 ] * b[ 3 ] + a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ] + a[ 3 ] * b[ 0 ] );
         *this = res;
      }

      private:
         /**
          @brief construct the quaternion from angle-axis
          */
         void _fromAngleAxis( double angle, double i, double j, double k )
         {
            double sinangle = sin( angle / 2 );
            double cosangle = cos( angle / 2 );

            // we normalize the angle in case it is not
            double size = sqrt( i * i + j * j + k * k );
            _buffer[ 0 ] = cosangle;
            _buffer[ 1 ] = i * sinangle / size;
            _buffer[ 2 ] = j * sinangle / size;
            _buffer[ 3 ] = k * sinangle / size;
         }
   };

   /**
    @biref do the interpolation from the quaternion qa to qb according to the ratio t (t in [0..1])
    */
   inline Quaternion slerp( const Quaternion& qa, const Quaternion& qb, double t )
   {
	   // quaternion to return
	   Quaternion qm( 0, 0, 0, 0 );

	   // Calculate angle between them.
	   double cosHalfTheta = qa[ 0 ] * qb[ 0 ] + qa[ 1 ] * qb[ 1 ] + qa[ 2 ] * qb[ 2 ] + qa[ 3 ] * qb[ 3 ];

	   // if qa=qb or qa=-qb then theta = 0 and we can return qa
	   if ( fabs( cosHalfTheta ) >= 1 )
      {
		   qm[ 0 ] = qa[ 0 ];
         qm[ 1 ] = qa[ 1 ];
         qm[ 2 ] = qa[ 2 ];
         qm[ 3 ] = qa[ 3 ];
		   return qm;
	   }

	   // Calculate temporary values.
	   double halfTheta = acos( cosHalfTheta );
	   double sinHalfTheta = sqrt( 1 - cosHalfTheta*cosHalfTheta );

	   // if theta = 180 degrees then result is not fully defined
	   // we could rotate around any axis normal to qa or qb
	   if ( fabs( sinHalfTheta ) < 0.001 )
      {
		   qm[ 0 ] = ( qa[ 0 ] * 0.5 + qb[ 0 ] * 0.5 );
		   qm[ 1 ] = ( qa[ 1 ] * 0.5 + qb[ 1 ] * 0.5 );
		   qm[ 2 ] = ( qa[ 2 ] * 0.5 + qb[ 2 ] * 0.5 );
		   qm[ 3 ] = ( qa[ 3 ] * 0.5 + qb[ 3 ] * 0.5 );
		   return qm;
	   }
	   double ratioA = sin( ( 1 - t ) * halfTheta ) / sinHalfTheta;
	   double ratioB = sin( t * halfTheta ) / sinHalfTheta; 

	   //calculate Quaternion.
	   qm[ 0 ] = ( qa[ 0 ] * ratioA + qb[ 0 ] * ratioB );
	   qm[ 1 ] = ( qa[ 1 ] * ratioA + qb[ 1 ] * ratioB );
	   qm[ 2 ] = ( qa[ 2 ] * ratioA + qb[ 2 ] * ratioB );
	   qm[ 3 ] = ( qa[ 3 ] * ratioA + qb[ 3 ] * ratioB );
	   return qm;
   }
}
}

#endif
