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

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>

#include "camcal.h"

using namespace std;

void display_help()
{
   cout<<"usage:\n";
   cout<<"  -i/ --imagelist imroot imext number_of_im\n";
   cout<<"  -c/--display-corners\n";
   cout<<"  -r/--apply-ordering  // can be used only with -t 6 8\n";
   cout<<"  -d/--compansate-distortion\n";
   cout<<"  -t/--grid-type ncols nrows\n";
   cout<<"  -s/--grid-size gw gh\n";
   cout<<"  -o/--outfile filename\n";
   cout<<endl;
}

int main(int argc, char **argv)
{
   if( argc == 1 )
   {
      display_help();
      exit(0);
   }

   camcal calibration;

   int counter=1;
   while( counter < argc ) {
      if( !strcmp("-i", argv[counter] ) || !strcmp("--imagelist", argv[counter] ) )  {
         counter++;
         string im_file_name = argv[counter];  counter++;
         string im_file_ext  = argv[counter];  counter++;
         int im_number = atoi(argv[counter]); counter++;

         string filename;
         filename = im_file_name + "%d." + im_file_ext;

         cout<<"imname: "<<filename<<endl;
         calibration.load( filename.c_str(), im_number );
         continue;
      }
      if( !strcmp("-c", argv[counter] ) || !strcmp("--display-corners", argv[counter] ) )  {
         counter++;
         calibration.display_corners();
         continue;
      }
      if( !strcmp("-r", argv[counter] ) || !strcmp("--apply-ordering", argv[counter] ) )  {
         counter++;
         calibration.apply_ordering();
         continue;
      }
      if( !strcmp("-d", argv[counter] ) || !strcmp("--compansate-distortion", argv[counter] ) )  {
         counter++;
         calibration.compansate_distortion();
         continue;
      }
      if( !strcmp("-t", argv[counter] ) || !strcmp("--grid-type", argv[counter] ) )  {
         counter++;
         int ncols = atoi( argv[counter] ); counter++;
         int nrows = atoi( argv[counter] ); counter++;
         calibration.set_grid_structure( ncols, nrows );
         continue;
      }
      if( !strcmp("-s", argv[counter] ) || !strcmp("--grid-size", argv[counter] ) )  {
         counter++;
         float grid_w = atof( argv[counter] ); counter++;
         float grid_h = atof( argv[counter] ); counter++;
         calibration.set_grid_size( grid_h, grid_w );
         continue;
      }
      if( !strcmp("-o", argv[counter] ) || !strcmp("--outfile", argv[counter] ) )  {
         counter++;
         string out = argv[counter];
         calibration.outfile(out);
         counter++;
         continue;
      }
      cout<<"unknown option:"<<argv[counter]<<endl;
      exit(1);
   }

   calibration.calibrate();

}
