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

