/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it              //
// and/or modify it under the terms of the GNU General Public License  //
// version 2 (or higher) as published by the Free Software Foundation. //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by                                                  //
// Engin Tola                                                          //
//                                                                     //
// web   : http://cvlab.epfl.ch/~tola                                  //
// email : engin.tola@epfl.ch                                          //
//                                                                     //
// If you use this code for research purposes, please refer to the     //
// webpage above                                                       //
/////////////////////////////////////////////////////////////////////////

#ifndef CAMCAL_H
#define CAMCAL_H

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include "cv.h"
#include "highgui.h"

using namespace std;

typedef unsigned char BYTE;

#define PI 3.1415

//for visual number effect to see the corners that were found
// char* numbers[] =
// { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
//   "21", "22", "23", "24", "25", "26", "27", "28", "29", "30","31", "32", "33", "34", "35", "36", "37", "38", "39", "40",
//   "41", "42", "43", "44", "45", "46", "47", "48", "49", "50","51", "52", "53", "54", "55", "56", "57", "58", "59", "60",
//   "61", "62", "63", "64", "65", "66", "67", "68", "69", "70","71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
//   "81", "82", "83", "84", "85", "86", "87", "88", "89", "90"
// };

class camcal
{
public:
   camcal();
   ~camcal();
   void calibrate();
   void load( string filename, int im_number );

   void set_grid_structure( int ncols, int nrows  )
      {
         m_x_height = nrows;
         m_x_width  = ncols;
      }

   void set_grid_size( float grid_height=25, float grid_width=25 )
      {
         m_grid_width = grid_width;
         m_grid_height = grid_height;
      }

   void display_corners( bool dcorners = true )
      {
         m_display_corners = dcorners;
      }

   void outfile( string out )
      {
         m_out = out;
      }

   void apply_ordering( bool order=true )
      {
         m_apply_ordering = order;
      }

   void compansate_distortion( bool distortion=true )
      {
         m_compansate_distortion = distortion;
      }

private:
   int m_effective_image_no;
   int m_image_number;

   int m_x_height;
   int m_x_width;
   int m_corner_no;

   float m_grid_height;
   float m_grid_width;

   bool m_display_corners;
   bool m_apply_ordering;
   bool m_compansate_distortion;

   IplImage** m_input_images;
   CvSize imgsize;

   string m_out;

   // results
   CvVect64d m_distortion;
   CvMatr64d m_camera_matrix;
   CvVect64d m_translation_vectors;
   CvMatr64d m_rotation_matrices;
   CvPoint2D64d* uveff ;
   CvPoint3D64d* XYZeff;
   CvPoint2D64d m_dErrorPower;
   CvPoint2D64d m_dErrorMax;
   CvPoint2D64d m_dErrorMean;

   // functions
   void CalibrateCamera();
   CvPoint2D32f CalculateAngleRespectTo(CvPoint2D32f *arr, int n, CvPoint2D32f anchor,
                                        int thr1, int thr2=0,
                                        bool exact=true, bool vertAlso=false);

   CvPoint3D32f FindRectangleCorner(CvPoint2D32f *points, int n );

   void FindCorners();
   void SortPoints(CvPoint2D32f *&points, int n, CvPoint3D32f *init);
   void SortRespectTo(CvPoint2D32f *&arr, CvPoint2D32f anchor, int n);
   void CalculateCalibrationErrors(bool print);
   void GenerateReport();
};

int Modulus(int n, int m);
double MapAnglePi2mPi(double theta);

#endif
