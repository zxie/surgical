#include "camcal.h"

int Modulus(int n, int m)
{
   int ret = n%m;
   return ret<0 ? ret+m : ret;
}

double MapAnglePi2mPi(double theta)
{
   if( theta < -PI ) theta += 2*PI;
   if( theta >  PI ) theta -= 2*PI;
   return theta;
}

camcal::camcal()
{
   m_image_number = 0;

   m_x_height = 8;
   m_x_width  = 6;

   m_corner_no = m_x_height * m_x_width;

   m_grid_height = 25;
   m_grid_width  = 25;
   m_display_corners = false;
   m_apply_ordering   = true;
   m_compansate_distortion = true;

   m_distortion = NULL;
   m_camera_matrix = NULL;
   m_translation_vectors = NULL;
   m_rotation_matrices = NULL;

   uveff = NULL;
   XYZeff = NULL;
}

camcal::~camcal()
{
   delete[]m_distortion;
   delete[]m_camera_matrix;
   delete[]m_translation_vectors;
   delete[]m_rotation_matrices;
   delete[]uveff;
   delete[]XYZeff;

   if( m_input_images != NULL )
   {
      for( int i=0; i<m_image_number; i++ )
      {
         cvReleaseImage( &m_input_images[i] );
      }
      delete []m_input_images;
   }
}

void camcal::load( string filename, int im_number  )
{

   m_image_number = im_number;

   if(  m_x_height == 0 || m_x_width == 0)
      return;

   char buffer[255];
   m_input_images  = new IplImage *[im_number];

   // load images as IplImage*'s
   int im_ind = 0;
   for( int i=0; i< im_number; i++)
   {
      sprintf( buffer, filename.c_str(), i );

      cout<<"loading image: "<<buffer<<endl;

      m_input_images[im_ind]  = cvLoadImage( buffer );

      if( m_input_images[im_ind] == NULL )
      {
         cout<<"no such image " << buffer << endl;
         continue;
      }
      if( im_ind==0 )
      {
         imgsize.width  = m_input_images[0]->width;
         imgsize.height = m_input_images[0]->height;
      }
      im_ind = im_ind + 1;
   }
}

void camcal::calibrate()
{
   if( m_image_number == 0 )
      return;

   // Extract World and Image Coordinates
   FindCorners();

   // Find CameraMatrix, Distortion Params, Rotation Matrices, Translation Vectors
   CalibrateCamera();

   // Backproject Points and Calculate Errors on X and Y direction
   CalculateCalibrationErrors( true );

   // Generates Reports concerning Calibration and Error
   GenerateReport();
}

//
// Function: FindCorners
// Purpose : This function finds the corner points in the calibration images
//           using opencv functions cvFindChessBoardCornerGuesses and
//           cvFindCornerSubPix.  After image pixel coordinates are found, their
//           respective world coordinates are assigned. (z coordinate is always
//           zero). When the function cannot find the specified number of
//           coordinates in the image, the image is discarded because of the
//           added complexity in assigning their world coordinates.
// Output  : uveff : coordinates of the chessborder in image plane.
//           XYZeff: coordinates of uveff w.r.t World Coordinate System
//           m_effective_image_no : number of images processed.
//----------------------------------------------------------------------------
void camcal::FindCorners()
{
   IplImage* img  = 0;
   IplImage* img0 = 0;
   IplImage* img1 = 0;
   IplImage* greyimg = 0;

   CvFont dfont;
   cvInitFont (&dfont, CV_FONT_VECTOR0, 0.3, 0.3, 0.0f, 1);

   CvPoint onecorner;

   int numcorners = m_corner_no;

   CvPoint2D64d* uv  = new CvPoint2D64d[m_image_number * m_corner_no];
   CvPoint3D64d* XYZ = new CvPoint3D64d[m_image_number * m_corner_no];

   CvPoint2D32f* corners = new CvPoint2D32f[m_corner_no];
   CvMemStorage* storage = 0;

   m_effective_image_no=-1;

   for( int imgnum=0; imgnum<m_image_number; imgnum++ )
   {
      numcorners = m_corner_no;

      img = m_input_images[imgnum];

      imgsize.width = img->width;
      imgsize.height= img->height;

      img0 = cvCloneImage(img);
      img1 = cvCloneImage(img);
      greyimg = cvCreateImageHeader(imgsize,IPL_DEPTH_8U,1);
      cvCreateImageData(greyimg);

      cvCvtColor(img, greyimg, CV_RGB2GRAY);
      img0 = cvCloneImage(greyimg);

      cvFindChessBoardCornerGuesses(greyimg,
                                    img0,
                                    storage,
                                    cvSize(m_x_height,m_x_width),
                                    corners,
                                    &numcorners);

      if( numcorners != m_corner_no ) {
         cvReleaseImage( &img0 );
         cvReleaseImage( &img1 );
         cvReleaseImage( &greyimg );
         continue;
      }
      else
         m_effective_image_no++;

      // draw a circle at each corner found
      for( int t = 0; t < numcorners; t++ )
      {
         onecorner.x = (int)corners[t].x;
         onecorner.y = (int)corners[t].y;

         cvCircle(img1, onecorner, 8, CV_RGB(0,255,0),2);
         // image, center, radius, color, thickness
      }

      // Find sub-corners
      cvFindCornerSubPix(greyimg,
                         corners,
                         numcorners,
                         cvSize (m_x_height,m_x_width),
                         cvSize(-1, -1),
                         cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 10, 0.1));

      // correct order
      if( m_apply_ordering ){
         CvPoint3D32f init = FindRectangleCorner( corners, numcorners );
         if( init.z < m_x_height*m_x_width )
            SortPoints(corners, numcorners, &init);
         else
         {
            cout<<"Sort Error";
            cvReleaseImage( &img0 );
            cvReleaseImage( &img1 );
            cvReleaseImage( &greyimg );
            m_effective_image_no--;
            continue;
         }
      }

      //draw a circle and put the corner number at each subcorner found
      for( int t = 0; t < numcorners; t++ )
      {
         onecorner.x = (int)corners[t].x;
         onecorner.y = (int)corners[t].y;

         cvCircle(img1, onecorner, 3, CV_RGB(255,0,0),1);

         char buf[10];
         sprintf( buf, "%d", t );

//          cvPutText(img1,numbers[t], cvPoint(onecorner.x, onecorner.y + 20), &dfont, CV_RGB(255,0,0));
         cvPutText(img1, buf, cvPoint(onecorner.x, onecorner.y + 20), &dfont, CV_RGB(255,0,0));
      }

      // CAMERA CALIBRATION PART
      for( int currPoint=0; currPoint < numcorners; currPoint++ )
      {
         uv[ m_effective_image_no*numcorners + currPoint].x = corners[currPoint].x;
         uv[ m_effective_image_no*numcorners + currPoint].y = corners[currPoint].y;
      }

      int index;
      for( int i = 0; i < m_x_width; i++ )
      {
         for( int j = 0; j < m_x_height; j++ )
         {
            index = m_effective_image_no*numcorners + i*m_x_height+j;

            XYZ[ index ].x = m_grid_width *(m_x_width -i);
            XYZ[ index ].y = m_grid_height*(m_x_height-j);
            XYZ[ index ].z = 0;
         }
      }

      if( m_display_corners )
      {
         cvvNamedWindow( "image", 1 );
         cvvShowImage("image",img1);
         cvvWaitKey(0);
         cvDestroyWindow( "image" );
      }

//		cvReleaseImage( &img );
      cvReleaseImage( &img0 );
      cvReleaseImage( &img1 );
      cvReleaseImage( &greyimg );

   } //loop to next image
   free (corners);

   m_effective_image_no++;

   delete []uveff ; uveff  =NULL;
   delete []XYZeff; XYZeff = NULL;

   if( m_image_number == m_effective_image_no ){
      uveff  = uv;  uv  = NULL;
      XYZeff = XYZ; XYZ = NULL;
   }
   else
   {
      int size = m_effective_image_no * m_corner_no;
      uveff  = new CvPoint2D64d[size];
      XYZeff = new CvPoint3D64d[size];

      for(int ph=0; ph<size; ph++)
      {
         uveff [ph] = uv [ph];
         XYZeff[ph] = XYZ[ph];
      }
      delete []uv;  uv  = NULL;
      delete []XYZ; XYZ = NULL;
   }
}

CvPoint3D32f camcal::FindRectangleCorner(CvPoint2D32f *points, int n )
{
   CvPoint3D32f retVal;
   CvPoint2D32f out;

   int k;
   for( k=0; k<n; k++ )
   {
      out = CalculateAngleRespectTo( points, n,  points[k], m_x_height-2, m_x_width-2, true, true);
      if( out.x == INT_MIN || out.y == INT_MIN )
         continue;
      else
         break;
   }

   out = CalculateAngleRespectTo( points, n,  points[k], m_x_height-2, m_x_width-2, false, true);

   retVal.x = out.x;
   retVal.y = out.y;
   retVal.z = (float)k;

   if( k >= m_x_width*m_x_height )
      return retVal; // error

   int tempIndex=k;
   double degree1, degree2;
   for( int i=0; i<n; i++ )
   {
      degree1 = atan2(points[k].y-points[i].y,points[i].x-points[k].x);
      degree2 = fabs(degree1 - retVal.y);

      if( degree2 < PI/90 || degree1 == 0 )
      {
         if( points[i].x < points[tempIndex].x  )
         {
            tempIndex = i;
         }
      }
   }
   k = tempIndex;

   // find the the most repeating angle in the y direction
   out = CalculateAngleRespectTo( points, n, points[k], m_x_height-2);
   if( out.x == INT_MIN )
   {
      cout<<"error after CalculateAngleRespectTo\n";
      exit(2);
   }
   retVal.x = out.x; // orientation of y-alignment

   for( int i=0; i<n; i++ )
   {
      degree1 = atan2(points[k].y-points[i].y,points[i].x-points[k].x);
      degree2 = fabs(degree1 - retVal.x);

      if( degree2 < PI/90 || degree1 == 0 )
      {
         if( points[i].y < points[tempIndex].y  )
         {
            tempIndex = i;
         }
      }
   }
   k = tempIndex; // TOP-LEFT FOUND

   // Find the angles of alignment
   out = CalculateAngleRespectTo( points, n, points[tempIndex], m_x_height-2, m_x_width-2, false, true);
   if( out.x == INT_MIN )
   {
      cout<<"error after CalculateAngleRespectTo\n";
      exit(2);
   }
   retVal.x = out.x;
   retVal.y = out.y;
   retVal.z = (float)tempIndex;

   return retVal;
}

void camcal::SortPoints(CvPoint2D32f *&points, int n, CvPoint3D32f *init)
{
   CvPoint2D32f* output   = new CvPoint2D32f[n         ];
   CvPoint2D32f* longrow  = new CvPoint2D32f[m_x_height];
   CvPoint2D32f* shortcol = new CvPoint2D32f[m_x_width ];

   int startPointIndex = (int)init[0].z;

   int scounter = 0 ;

   double degree1, degree2;

   for( int i=0; i<n; i++)
   {
      degree1 = atan2(points[startPointIndex].y-points[i].y,points[i].x-points[startPointIndex].x);
      degree2 = fabs(degree1 - init[0].y);

      if( degree2 < PI/90 || degree1 == 0 )
      {
         shortcol[scounter] = points[i];
         scounter++;
      }
   }
   if( scounter != m_x_width )
   {
      cout<<"Error in camcal::SortPoints(CvPoint2D32f *, int, CvPoint3D32f *)\n";
      exit(3);
   }

   // sort 6-points top-to-bottom
   SortRespectTo( shortcol, points[startPointIndex], m_x_width );

   CvPoint2D32f anchor, out;
   int lcounter = 0;

   for( int i=0; i<scounter; i++ )
   {
      anchor = shortcol[i];

      out = CalculateAngleRespectTo(points, n, anchor, m_x_height-2);
      if( out.x == INT_MIN )
      {
         cout<<"error:  SortPoints\n";
         exit(4);
      }

      lcounter = 0;
      for( int j=0; j<n; j++ )
      {
         degree1 = atan2(anchor.y-points[j].y,points[j].x-anchor.x);
         degree2 = fabs(degree1 - out.x);

         if( degree2 < PI/90 || degree1 == 0 )
         {
            longrow[lcounter] = points[j];
            lcounter++;
            if( lcounter == m_x_height )
               break;
         }
      }
      if( lcounter != m_x_height )
      {
         cout<<"Error in camcal::SortPoints(CvPoint2D32f *, int, CvPoint3D32f *)\n";
         exit(1);
      }

      // sort 8-points left-to-right
      SortRespectTo( longrow, anchor, m_x_height );

      // insert sorted entires to the output array;
      for( int j=0; j<m_x_height; j++)
      {
         output[i*m_x_height+j] = longrow[j];
      }
   }

   delete []shortcol; shortcol = NULL;
   delete []longrow ; longrow  = NULL;
   delete []points  ; points   = output;
}

void camcal::SortRespectTo(CvPoint2D32f *&arr, CvPoint2D32f anchor, int n)
{
//  N*N sort scheme

   CvPoint2D32f *out  = new CvPoint2D32f[n];

   double *dist  = new double[n];
   int    *order = new int[n];

   out[0] = anchor;

   for( int i=0; i<n; i++)
   {
      dist[i] = sqrt( pow(arr[i].y-anchor.y,2) + pow(arr[i].x-anchor.x,2) );
   }

   int minIndex;
   double min;

   for( int i=0; i<n; i++ )
   {
      min = INT_MAX;
      minIndex = 0;
      for( int j=0; j<n; j++ )
      {
         if( dist[j] < min )
         {
            min = dist[j];
            minIndex = j;
         }
      }

      order[i] = minIndex;
      dist[minIndex] = INT_MAX;
   }

   for( int i=0; i<n; i++)
   {
      out[i] = arr[ order[i] ];
   }

   delete []dist;
   delete []order;
   delete []arr; arr = out;
}

CvPoint2D32f camcal::CalculateAngleRespectTo(CvPoint2D32f *arr, int n, CvPoint2D32f anchor, int thr1, int thr2, bool exact, bool vertAlso)
{
   CvPoint2D32f retVal;
   retVal.x = INT_MIN;
   retVal.y = INT_MIN;

   double angle1,angle2;

   double *angles       = new double[n];
   BYTE   *angleCounter = new BYTE  [n];

   int		max_1, max_2, maxIndex_1, maxIndex_2;
   double	degree,degree1,degree2;

   int i;
   for(  i=0; i<n; i++ )
   {
      if( anchor.x == arr[i].x && anchor.y == arr[i].y )
         angles[i] = -10000;
      else
         angles[i]	= atan2( anchor.y-arr[i].y , arr[i].x-anchor.x );
   }


   for( i=0; i<n; i++) angleCounter[i] = 0;

   for( i=0; i<n; i++ )
   {
      if( angles[i]==1000) continue;

      for(int j=i+1; j<n; j++ )
      {
         degree = MapAnglePi2mPi(angles[i]-angles[j]);
         if( fabs(degree) <= PI/90 )
         {
            angleCounter[i]++;
            angles[j] = 1000;
         }
      }
   }

   max_1	   = INT_MIN;
   maxIndex_1 = 0;
   angle1	   = 0;

   for( i=0; i<n; i++ )
   {
      if( angleCounter[i] > max_1 )
      {
         max_1 = angleCounter[i];
         maxIndex_1 = i;
         angle1 = angles[i];
      }
   }

   if( max_1 >= thr1 )
      retVal.x = (float)angle1;

   double diff = 0;

   if( vertAlso )
   {
      max_2	   = INT_MIN;
      maxIndex_2 = -1;
      angle2	   = 0;

      for( i=0; i<n; i++ )
      {
         if( fabs(angles[i]) > 500 ) continue;

         degree1 = fabs(MapAnglePi2mPi(angles[i]-angle1));
         degree2 = fabs(degree1 - PI/2);
         if( angleCounter[i] >= max_2 && (((degree2 < PI/6)&&exact) || ((!exact)&&( degree1 > diff))))
         {
            max_2 = angleCounter[i];
            maxIndex_2 = i;
            angle2 = angles[i];
            diff = degree1;
         }
      }

      if( max_2 >= thr2 && maxIndex_2 != -1 )
         retVal.y = (float)angle2;
   }

//	delete []angles;
   delete []angleCounter;

   return retVal;
}

// Function: calibrate
// Purpose : Finds the A matrix(m_camera_matrix) and the m_distortion vector
//           from the loaded images. There must be at least 4 images in order to
//           get a correct A matrix. use more "different" images to get more
//           exact results.  This function also calculates the respective
//           rotation and translation vectors for the input calibration
//           images. By using these information pixel error is calculated.
// Output : m_camera_matrix: a0 = horizontal scaling, a4= vertical scaling, a1 =
//                           skew (a2,a5) = principal point.
//          m_distortion: first two entries are radial m_distortion and last two
//                           entries are tangential distoriton parameters.
// ------------------------------------------------------------------------------
void camcal::CalibrateCamera()
{
   if( m_effective_image_no == 0 )
      return;

   int* numPoints = new int[m_effective_image_no];
   int k;
   for( k=0; k < m_effective_image_no; k++ ) numPoints[k] = m_corner_no;

   delete[]m_distortion           ; m_distortion   = new double[4]; for( k=0; k<4; k++ ) m_distortion  [k] = 0.0;
   delete[]m_camera_matrix        ; m_camera_matrix = new double[9]; for( k=0; k<9; k++ ) m_camera_matrix[k] = 0.0;
   delete[]m_translation_vectors  ; m_translation_vectors   = new double[m_effective_image_no*3];
   delete[]m_rotation_matrices    ; m_rotation_matrices     = new double[m_effective_image_no*9];

   int  useIntrinsicGuess = 0;

   CvPoint2D64d* uvinv  = new CvPoint2D64d[m_effective_image_no * m_corner_no];
   CvPoint3D64d* XYZinv = new CvPoint3D64d[m_effective_image_no * m_corner_no];

   int index;

   for(int i=0 ; i<m_effective_image_no; i++)
   {
      for( int j=0; j<m_corner_no; j++ )
      {
         index = Modulus( i+2, m_effective_image_no );
         uvinv [i*m_corner_no+j].x = uveff [index*m_corner_no+j].x;
         uvinv [i*m_corner_no+j].y = uveff [index*m_corner_no+j].y;
         XYZinv[i*m_corner_no+j].x = XYZeff[index*m_corner_no+j].x;
         XYZinv[i*m_corner_no+j].y = XYZeff[index*m_corner_no+j].y;
         XYZinv[i*m_corner_no+j].z = XYZeff[index*m_corner_no+j].z;
      }
   }

   CvVect64d INVdistortion   = new double[4];
   CvMatr64d INVcameraMatrix = new double[9];
   CvVect64d INVtransVects   = new double[m_effective_image_no*3];
   CvMatr64d INVrotMatrs     = new double[m_effective_image_no*9];

   // cvCalibrateCamera_64d SOMEHOW fails to find the first rotation matrix true.
   // However it gives correct results for the remaining images. Therefore, I invert all
   // the data sets and enter them in reverse order. By doing so, I will take the last
   // rotation matrix of the inverted data set as the rotation matrix of the normal
   // data set.
   cvCalibrateCamera_64d(m_effective_image_no,
                         numPoints,
                         imgsize,
                         uveff,
                         XYZeff,
                         m_distortion,
                         m_camera_matrix,
                         m_translation_vectors,
                         m_rotation_matrices,
                         useIntrinsicGuess
      );

   cvCalibrateCamera_64d(m_effective_image_no,
                         numPoints,
                         imgsize,
                         uvinv,
                         XYZinv,
                         INVdistortion,
                         INVcameraMatrix,
                         INVtransVects,
                         INVrotMatrs,
                         useIntrinsicGuess
      );

   CvVect64d focal = new double[2];

   focal[0] = m_camera_matrix[0];
   focal[1] = m_camera_matrix[4];

   CvPoint2D64d principal;
   principal.x = m_camera_matrix[2];
   principal.y = m_camera_matrix[5];

   cvFindExtrinsicCameraParams_64d(m_corner_no,
                                   imgsize,
                                   uveff,
                                   XYZeff,
                                   focal,
                                   principal,
                                   m_distortion,
                                   m_rotation_matrices,
                                   m_translation_vectors
      );

   cvFindExtrinsicCameraParams_64d(m_corner_no,
                                   imgsize,
                                   uvinv,
                                   XYZinv,
                                   focal,
                                   principal,
                                   INVdistortion,
                                   INVrotMatrs,
                                   INVtransVects
      );

   // Correct first rotation matrix and translation vector!
   for( int i=0; i<9; i++ )
   {
      index = Modulus(-2, m_effective_image_no );

      m_rotation_matrices[i] = INVrotMatrs[index*9+i];
      if( i<3 )
         m_translation_vectors[i] = INVtransVects[3*index+i];

      index = Modulus( m_effective_image_no-3, m_effective_image_no );

      m_rotation_matrices[9*(m_effective_image_no-1)+i] = INVrotMatrs[index*9+i];
      if( i<3 )
         m_translation_vectors[3*(m_effective_image_no-1)+i] = INVtransVects[index*3+i];
   }

   delete[]focal		   ;
   delete[]INVdistortion  ;
   delete[]INVcameraMatrix;
   delete[]INVtransVects  ;
   delete[]INVrotMatrs	   ;

   delete[]numPoints;

   delete []XYZinv;
   delete []uvinv;
}

void camcal::GenerateReport()
{
   if( m_effective_image_no == 0 ) return;

   string file;

   if( !strcmp(m_out.c_str(),"") )
      file = "calibration";
   else
      file = m_out;

   string calibration_file = file;
   calibration_file += ".intrinsic";

   string calibration_log = file;
   calibration_log += ".log";

   FILE* cps = fopen(calibration_file.c_str(), "w+");
   cout<<"Effective Image Number Processed = "<<m_effective_image_no<<endl;

   for( int m=0; m<3; m++ )
   {
      for( int n=0; n<3; n++ )
      {
         fprintf(cps, "%e\t", m_camera_matrix[m*3+n]);
      }
      fprintf(cps, "\n");
   }

   for( int k=0; k<4; k++)
      fprintf(cps,"%e\t", m_distortion[k] );
   fprintf(cps,"\n");
   fclose(cps);

   cps = fopen(calibration_log.c_str(), "w+");
   fprintf(cps,"Effective Image Number Processed = %d\n\n",m_effective_image_no);

   for( int m=0; m<3; m++ )
   {
      for( int n=0; n<3; n++ )
      {
         fprintf(cps, "%e\t", m_camera_matrix[m*3+n]);
      }
      fprintf(cps, "\n");
   }
   fprintf( cps, "\nPixel Error Powers: \n");
   fprintf( cps, "X Error : %e\tY Error : %e\n",m_dErrorPower.x,m_dErrorPower.y);

   fprintf( cps, "\nPixel Mean Errors: \n");
   fprintf( cps, "X Error : %e\tY Error : %e\n",m_dErrorMean.x,m_dErrorMean.y);

   fprintf( cps, "\nMAX X Error : %e\n",m_dErrorMax.x);
   fprintf( cps, "MAX Y Error : %e\n",m_dErrorMax.y);

   fprintf(cps,"\nDistortion\n");
   for( int k=0; k<4; k++)
      fprintf(cps,"%e\t", m_distortion[k] );
   fprintf(cps,"\n\n");

   fprintf(cps,"R & T Matrices\n\n");

   fprintf(cps, "    Rotation Matrices                Translation Vectors\n");
   fprintf(cps, "------------------------------------------------------------\n");

   for(int i=0;i<m_effective_image_no;i++)
   {
      for(int j=0; j<9; j++)
      {
         fprintf(cps, "%e\t", m_rotation_matrices[i*9+j]);
         if((j+1)%3==0) {
            fprintf(cps,"%e\n", m_translation_vectors[i*3+j/3]);
         }
      }
      fprintf(cps,"\n");
   }

   fclose(cps);
}

void camcal::CalculateCalibrationErrors(bool print)
{
   if( m_effective_image_no == 0 ) return;

   // calculate pixel errors:
   CvPoint2D64d* uvapp = new CvPoint2D64d[m_effective_image_no * m_corner_no];

   string file = m_out;
   file += ".point_errors";

   FILE* fp = NULL;

   if( print )
   {
      fp = fopen(file.c_str(), "w");
      fprintf(fp,"index\trealx\trealy\tcalcx\tcalcy\terrorx\terrory\n");
   }

   int index;

   CvPoint3D64d backProject;

   double diffx,diffy;

   m_dErrorMean.x = 0;
   m_dErrorMean.y = 0;

   m_dErrorPower.x = 0;
   m_dErrorPower.y = 0;

   m_dErrorMax.x = 0;
   m_dErrorMax.y = 0;

   double r0,r1,r2,r3,r4,r5,r6,r7,r8,t0,t1,t2;
   double r_square, r_quad;

   double xx, yy, temp;

   for( int i=0; i<m_effective_image_no; i++ )
   {
      for( int j=0; j<m_corner_no; j++)
      {
         index = i*m_corner_no+j;

         r0 = m_rotation_matrices[i*9  ];
         r1 = m_rotation_matrices[i*9+1];
         r2 = m_rotation_matrices[i*9+2];
         r3 = m_rotation_matrices[i*9+3];
         r4 = m_rotation_matrices[i*9+4];
         r5 = m_rotation_matrices[i*9+5];
         r6 = m_rotation_matrices[i*9+6];
         r7 = m_rotation_matrices[i*9+7];
         r8 = m_rotation_matrices[i*9+8];

         t0 = m_translation_vectors[3*i  ];
         t1 = m_translation_vectors[3*i+1];
         t2 = m_translation_vectors[3*i+2];

         // multiply with R and add T vector
         backProject.x = r0*XYZeff[index].x +r1*XYZeff[index].y +r2*XYZeff[index].z + t0;
         backProject.y = r3*XYZeff[index].x +r4*XYZeff[index].y +r5*XYZeff[index].z + t1;
         backProject.z = r6*XYZeff[index].x +r7*XYZeff[index].y +r8*XYZeff[index].z + t2;

         if( m_compansate_distortion )
         {
            // apply projection
            uvapp[index].x = backProject.x / backProject.z;
            uvapp[index].y = backProject.y / backProject.z;

            // calculate distorted coordinates
            r_square = pow( uvapp[index].x , 2) +  pow( uvapp[index].y , 2);
            r_quad   = pow( r_square, 2);

            xx = uvapp[index].x*(1 + m_distortion[0]*r_square + m_distortion[1]*r_quad)
               +2*m_distortion[2]*uvapp[index].x*uvapp[index].y
               +  m_distortion[3]*(r_square + 2*uvapp[index].x *uvapp[index].x);

            yy =   uvapp[index].y*(1+m_distortion[0]*r_square
                                   +m_distortion[1]*r_quad)
               +  m_distortion[2]*(r_square+2*uvapp[index].y*uvapp[index].y)
               +2*m_distortion[3]*uvapp[index].x*uvapp[index].y;

            // multiply with camera matrix.
            uvapp[index].x = m_camera_matrix[0]*xx+m_camera_matrix[2];
            uvapp[index].y = m_camera_matrix[4]*yy+m_camera_matrix[5];
         }
         else
         {
            uvapp[index].x = m_camera_matrix[0]*backProject.x+
               m_camera_matrix[1]*backProject.y+
               m_camera_matrix[2]*backProject.z;

            uvapp[index].y = m_camera_matrix[3]*backProject.x+
               m_camera_matrix[4]*backProject.y+
               m_camera_matrix[5]*backProject.z;

            temp = m_camera_matrix[6]*backProject.x+
               m_camera_matrix[7]*backProject.y+
               m_camera_matrix[8]*backProject.z;

            uvapp[index].x /= temp;
            uvapp[index].y /= temp;
         }

         diffx = uveff[index].x-uvapp[index].x;
         diffy = uveff[index].y-uvapp[index].y;

         m_dErrorMean.x += fabs(diffx);
         m_dErrorMean.y += fabs(diffy);

         m_dErrorPower.x += pow(diffx,2);
         m_dErrorPower.y += pow(diffy,2);

         if( fabs(diffx) > m_dErrorMax.x ) m_dErrorMax.x = fabs(diffx);
         if( fabs(diffy) > m_dErrorMax.y ) m_dErrorMax.y = fabs(diffy);

         if( print )
         {
            fprintf( fp, "%d\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t\n",
                     index,
                     uveff[index].x,
                     uveff[index].y,
                     uvapp[index].x,
                     uvapp[index].y,
                     diffx,
                     diffy );
         }
      }
   }

   if( print )
   {
      fclose(fp);
   }

   m_dErrorPower.x  = sqrt(m_dErrorPower.x);
   m_dErrorPower.y  = sqrt(m_dErrorPower.y);
   m_dErrorPower.x /= m_effective_image_no*m_corner_no;
   m_dErrorPower.y /= m_effective_image_no*m_corner_no;

   m_dErrorMean.x /= m_effective_image_no*m_corner_no;
   m_dErrorMean.y /= m_effective_image_no*m_corner_no;

   delete []uvapp; uvapp = NULL;
}
