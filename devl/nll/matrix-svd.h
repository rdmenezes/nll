#ifndef NLL_MATRIX_SVD_H_
# define NLL_MATRIX_SVD_H_

namespace nll
{
namespace core
{
   /**
    @ingroup core
    @brief compute the euclidian distance AB without out of bound value.
    */
   inline static double pythag(double a, double b)
   {
     double absa,absb;
     absa=absolute(a);
     absb=absolute(b);
     if (absa > absb) return absa*sqrt(1.0+sqr(absb/absa));
     else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+sqr(absa/absb)));
   }

   #define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

   /**
    @ingroup core
    @brief Solve Ax = B using SVD decomposition
    @note code from numerical recipes
    */
   template <class type, class mapper>
   Buffer1D<type> solve_svd(const Matrix<type, mapper>& a, const Buffer1D<type>& b)
   {
	   Matrix<type, mapper> aa;
      aa.clone(a);
	   Buffer1D<type> w, b_io(b), x;
	   Matrix<type, mapper> v;

	   bool res = svdcmp(aa, w, v);
	   if (!res)
		   return x;
	   svbksb(aa, w, v, b_io, x);
	   return x;
   }

   // x doesn't need to be allocated
   template <class type, class mapper>
   bool svbksb(const Matrix<type, mapper>& a, const Buffer1D<type>& w, const Matrix<type, mapper>& v, const Buffer1D<type>& b, Buffer1D<type>& x)
   {
	   int jj,j,i;
	   type s;
	   ui32 singular_cnt = 0;
	   x = Buffer1D<type>(a.sizey());

	   int m=a.sizex();
	   int n=a.sizey();
	   Buffer1D<type> tmp(n);
	   for (j=0;j<n;j++) {
		   s=0.0;
		   if (!equal(w[j], 0.0)) {
			   for (i=0;i<m;i++) s += a(i, j)*b[i];
			   s /= w[j];
		   } else ++singular_cnt;

		   tmp[j]=s;
	   }
	   for (j=0;j<n;j++) {
		   s=0.0;
		   for (jj=0;jj<n;jj++) s += v(j, jj)*tmp[jj];
		   x[j]=s;
	   }

	   if (singular_cnt) {
		   std::cout << "# of zero singular values: " << singular_cnt << std::endl;
		   return false;
	   }
	   return true;
   }

   /**
    @ingroup core
    @brief SVD decomposition.

    in the case of eigen value, eigen vector decomposition:
    svdcmp( covarianceMatrix, eigen_values, eigen_vectors )
    eigen vectors are column vectors


    compute A=UWV^t, with A=[0..m][0..n] the input matrix. W is a diagonal matrix of singular value and outputed
            as a vector.
            V.V^t = 1
            U.U^t = 1

            properties: A^-1=V.(diag (1/wj)).U^t
    @param a the input matrix. It is directly replaced in the matrix (no copy). When it is done,
           a is replaced by U, an orthogonal matrix
    @param w the diagonal matrix (but stored as a vector to save space) of singular values [0..n]
    @param v (and NOT v^t) an orthogonal matrix [0..n][0..n]
    @note code from numerical recipes
    */
   template <class type, class mapper>
   bool svdcmp(Matrix<type, mapper> a, Buffer1D<type>& w, Matrix<type, mapper>& v)
   {
	   bool flag;
	   int i,its,j,jj,k,l = 0,nm = 0;
	   type anorm,c,f,g,h,s,scale,x,y,z;

	   int m=a.sizey();
	   int n=a.sizex();
	   Buffer1D<type> rv1(n);
	   g=scale=anorm=0.0;
	   w = Buffer1D<type>(n);
	   v = Matrix<type, mapper>(n, n);
	   for (i=0;i<n;i++) {
		   l=i+2;
		   rv1[i]=scale*g;
		   g=s=scale=0.0;
		   if (i < m) {
			   for (k=i;k<m;k++) scale += fabs(a(k, i));
			   if (fabs(scale) >= std::numeric_limits<type>::epsilon()) {
				   for (k=i;k<m;k++) {
					   a(k, i) /= scale;
					   s += a(k, i)*a(k, i);
				   }
				   f=a(i, i);
				   g = -SIGN(sqrt(s),f);
				   h=f*g-s;
				   a(i, i)=f-g;
				   for (j=l-1;j<n;j++) {
					   for (s=0.0,k=i;k<m;k++) s += a(k, i)*a(k, j);
					   f=s/h;
					   for (k=i;k<m;k++) a(k, j) += f*a(k, i);
				   }
				   for (k=i;k<m;k++) a(k, i) *= scale;
			   }
		   }
		   w[i]=scale *g;
		   g=s=scale=0.0;
		   if (i+1 <= m && i+1 != n) {
			   for (k=l-1;k<n;k++) scale += fabs(a(i, k));
			   if (scale != 0.0) {
				   for (k=l-1;k<n;k++) {
					   a(i, k) /= scale;
					   s += a(i, k)*a(i, k);
				   }
				   f=a(i, l-1);
				   g = -SIGN(sqrt(s),f);
				   h=f*g-s;
				   a(i, l-1)=f-g;
				   for (k=l-1;k<n;k++) rv1[k]=a(i, k)/h;
				   for (j=l-1;j<m;j++) {
					   for (s=0.0,k=l-1;k<n;k++) s += a(j, k)*a(i, k);
					   for (k=l-1;k<n;k++) a(j, k) += s*rv1[k];
				   }
				   for (k=l-1;k<n;k++) a(i, k) *= scale;
			   }
		   }
         anorm=std::max((f64)anorm,(fabs(w[i])+fabs(rv1[i])));
	   }
	   for (i=n-1;i>=0;i--) {
		   if (i < n-1) {
			   if (g != 0.0) {
				   for (j=l;j<n;j++)
					   v(j, i)=(a(i, j)/a(i, l))/g;
				   for (j=l;j<n;j++) {
					   for (s=0.0,k=l;k<n;k++) s += a(i, k)*v(k, j);
					   for (k=l;k<n;k++) v(k, j) += s*v(k, i);
				   }
			   }
			   for (j=l;j<n;j++) v(i, j)=v(j, i)=0.0;
		   }
		   v(i, i)=1.0;
		   g=rv1[i];
		   l=i;
	   }
      for (i=std::min(m,n)-1;i>=0;i--) {
		   l=i+1;
		   g=w[i];
		   for (j=l;j<n;j++) a(i, j)=0.0;
		   if (g != 0.0) {
			   g=1.0/g;
			   for (j=l;j<n;j++) {
				   for (s=0.0,k=l;k<m;k++) s += a(k, i)*a(k, j);
				   f=(s/a(i, i))*g;
				   for (k=i;k<m;k++) a(k, j) += f*a(k, i);
			   }
			   for (j=i;j<m;j++) a(j, i) *= g;
		   } else for (j=i;j<m;j++) a(j, i)=0.0;
		   ++a(i, i);
	   }
	   for (k=n-1;k>=0;k--) {
		   for (its=0;its<30;its++) {
			   flag=true;
			   for (l=k;l>=0;l--) {
				   nm=l-1;
				   if (fabs(rv1[l])+anorm == anorm) {
					   flag=false;
					   break;
				   }
				   if (fabs(w[nm])+anorm == anorm) break;
			   }
			   if (flag) {
				   c=0.0;
				   s=1.0;
				   for (i=l;i<k+1;i++) {
					   f=s*rv1[i];
					   rv1[i]=c*rv1[i];
					   if (fabs(f)+anorm == anorm) break;
					   g=w[i];
					   h=pythag(f,g);
					   w[i]=h;
					   h=1.0/h;
					   c=g*h;
					   s = -f*h;
					   for (j=0;j<m;j++) {
						   y=a(j, nm);
						   z=a(j, i);
						   a(j, nm)=y*c+z*s;
						   a(j, i)=z*c-y*s;
					   }
				   }
			   }
			   z=w[k];
			   if (l == k) {
				   if (z < 0.0) {
					   w[k] = -z;
					   for (j=0;j<n;j++) v(j, k) = -v(j, k);
				   }
				   break;
			   }
			   if (its == 29)
				   return false;	// no convergence in 30 svdcmp iterations
			   x=w[l];
			   nm=k-1;
			   y=w[nm];
			   g=rv1[nm];
			   h=rv1[k];
			   f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			   g=pythag(f,1.0);
			   f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			   c=s=1.0;
			   for (j=l;j<=nm;j++) {
				   i=j+1;
				   g=rv1[i];
				   y=w[i];
				   h=s*g;
				   g=c*g;
				   z=pythag(f,h);
				   rv1[j]=z;
				   c=f/z;
				   s=h/z;
				   f=x*c+g*s;
				   g=g*c-x*s;
				   h=y*s;
				   y *= c;
				   for (jj=0;jj<n;jj++) {
					   x=v(jj ,j);
					   z=v(jj, i);
					   v(jj, j)=x*c+z*s;
					   v(jj, i)=z*c-x*s;
				   }
				   z=pythag(f,h);
				   w[j]=z;
				   if (z) {
					   z=1.0/z;
					   c=f*z;
					   s=h*z;
				   }
				   f=c*g+s*y;
				   x=c*y-s*g;
				   for (jj=0;jj<m;jj++) {
					   y=a(jj, j);
					   z=a(jj, i);
					   a(jj, j)=y*c+z*s;
					   a(jj, i)=z*c-y*s;
				   }
			   }
			   rv1[l]=0.0;
			   rv1[k]=f;
			   w[k]=x;
		   }
	   }
	   return true;
   }

}
}

#endif
