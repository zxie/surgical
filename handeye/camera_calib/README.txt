Applications
============

./camcal -> Computes the intrinsic parameters of a camera
./parser -> Creates a Matlab file containing the transformations of 
            the calibration pattern.

Requirements
============

Compiler: g++-4.2

-> sudo apt-get g++-4.2

Installation steps
==================

1)Compile the "camcal" application:

-> make

2)Compile the parser application:

-> g++ parser.cpp -o parser


Test the example
================

1) Camera calibration

-> ./camcal -t 6 8 -i images/image_ ppm 19 -s 20 20 -o image 

2) Parses data to a matlab file

-> ./parser -n 19 < image.log > image.m

That's it!!
Enjoy it.



Engin Tola's documentation:
===============================================================================

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 (or higher) as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

Written and (C) by
Engin Tola

web   : http://cvlab.epfl.ch/~tola
email : engin.tola@epfl.ch

If you use this code for research purposes, please refer to the webpage above

===============================================================================

Installation:

Issue the standard 'make' and 'make install' commands. By default it will be
installed to /usr/bin/. Just change the 'installdir' variable in the makefile if
you want a separate place.

Dependency: 

Project depends on OpenCV. It is included to the project using pkg-config
utility. If you don't want to use it, include

 LDFLAGS  += -lcxcore -lcv -lhighgui -lcvaux -lml
 CXXFLAGS +=  -I/usr/include/opencv  --> opecv include directory

at the end of "MAKEFILE FLAGS" section in you makefile.

===============================================================================

You need a calibration pattern to calibrate the camera. You can find a sample
calibration pattern in the /grid folder. Print the grid.pdf. There must be some
white area around the pattern for the algorithm to work. The calibration images
have to be taken in an upward fashion i.e, #of colums < #of rows

Algorithm Information:

This program uses the OpenCV’s “cvFindChessBoardCornerGuesses“ function in order
to find the location of the corners in the image and “cvCalibrateCamera_64d”
function to find the A matrice of the camera. “cvFindExtrinsicCameraParams_64d”
function is used to find R and T matrices. There are some bugs in the OpenCV
implementation and I tried to correct or in a way avoid them. For example,
“cvFindChessBoardCornerGuesses“ function sometimes cannot find all of the
corners or returns them in a un-ordered way. To avoid wrong calculations you
