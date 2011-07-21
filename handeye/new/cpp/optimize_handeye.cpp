#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>

using namespace std;
USING_PART_OF_NAMESPACE_EIGEN
using cv::Mat;
using cv::Point2f;
using cv::Point3f;
using cv::FileStorage;

// #TODO: Use Boost.Program_options instead of hardcoding these

// Checkerboard parameters
#define CB_ROWS 8
#define CB_COLS 6
#define CB_CELL_SIZE 7 // in mm
#define CB_SIZE CB_ROWS*CB_COLS
// Other configuration params
#define IMAGE_DIR "/home/ziang/Desktop/20110615_handeye/captures/left"
#define IMAGE_PREFIX ""
#define MAX_IMAGE_INDEX 50 // Can set much higher than actual
#define IMAGE_EXT ".tif"
#define OUTPUT_DIR string("calibration_results/")

/*
 * Returns string vector of IMAGE_EXT files in given directory.
 * Currently not used.
 */
int getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error opening " << dir << endl;
        return -1;
    }
    while ((dirp = readdir(dp)) != NULL) {
        string fname = dirp->d_name;
        if (strcmp(fname.c_str(), ".") == 0 || strcmp(fname.c_str(), "..") == 0)
            continue;
        // Can throw index-out-of-bounds depending on image naming
        if (strcmp(fname.substr(fname.size()-4).c_str(), IMAGE_EXT) != 0)
            continue;
        files.push_back(string(IMAGE_DIR)+"/"+fname);
    }
    closedir(dp);
    return 0;
}

/* 
 * Uses image files in given directory to obtain camera calibration parameters.
 * Largely copied from code in Learning OpenCV, 2008.
 * NOTE: Checkerboard orientation cannot vary too much in images, otherwise
 * corners are not detected in proper order. Manually check this in images.
 */
void calibrate_camera(string img_dir)
{
    vector<string> img_files;
    for (int i = 0; i < MAX_IMAGE_INDEX; i++)
    {
        char img_file[250];
        sprintf(img_file, "%s/%s%d%s", IMAGE_DIR, IMAGE_PREFIX, i, IMAGE_EXT);
        IplImage *temp = cvLoadImage(img_file);
        if (temp == NULL)
        {
            cout << "skipping " << img_file << endl;
            continue;
        }
        img_files.push_back(img_file);
    }
    int n_boards = img_files.size();

    // Allocate matrices for calibration
    CvSize board_sz = cvSize(CB_ROWS, CB_COLS);
    vector< vector<Point2f> > image_points;
    vector< vector<Point3f> > object_points;
    Mat intrinsic_matrix = Mat(3,3,CV_32FC1);
    Mat distortion_coeffs = Mat(5,1,CV_32FC1);

    // Go through each of the images and collect data
    CvPoint2D32f* corners = new CvPoint2D32f[CB_SIZE];
    int corner_count;
    int successes = 0;
    int step; // Used to index into checkerboard in image_points
    IplImage *image;
    IplImage *gray_image;
    for (int i = 0; i < n_boards; i++)
    {
        image = cvLoadImage(img_files.at(i).c_str());
        cout << "loading " img_files.at(i) << "..." << endl;
        gray_image = cvCreateImage(cvGetSize(image),8,1); // subpixel

        // Find checkerboard corners
        int found = cvFindChessboardCorners(
                image, board_sz, corners, &corner_count,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

        // Get Subpixel accuracy on those corners
        cvCvtColor(image, gray_image, CV_BGR2GRAY);
        cvFindCornerSubPix(gray_image, corners, corner_count,
                cvSize(11,11),cvSize(-1,-1),
                cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));

        // Draw checkerboard on image
        cvDrawChessboardCorners(image, board_sz, corners,
                corner_count, found);

        char window_name[250];
        sprintf(window_name, "Camera Calibration, Image %d of %d", i+1, n_boards);
        cvShowImage(window_name, image);

        // Wait for keypress before moving on to next image.
        // Hit 'Esc' to exit.
        int c = cv::waitKey();
        if (c == 27)
            return;

        // If all corners found, add to data
        vector<Point2f> single_image_points;
        vector<Point3f> single_object_points;
        if( corner_count == CB_SIZE ) {
            for( int j=0; j<CB_SIZE; ++j ) {
                single_image_points.push_back(Point2f((float) corners[j].x, (float) corners[j].y));
                single_object_points.push_back(Point3f((float) (j/CB_COLS)*CB_CELL_SIZE, (float) (j%CB_COLS)*CB_CELL_SIZE, 0.0f));
            }
            successes++;
        }

        image_points.push_back(single_image_points);
        object_points.push_back(single_object_points);

    }

    // Create matrices for translation and rotation vectors
    vector<Mat> tvecs;
    vector<Mat> rvecs;
    for (int i = 0; i < successes; i++)
    {
        tvecs.push_back(Mat(3,1,CV_32FC1));
        rvecs.push_back(Mat(3,3,CV_32FC1));
    }

    // At this point we have all of the chessboard corners we need.
    // Initialize the intrinsic matrix such that the two focal
    // lengths have a ratio of 1.0.
    intrinsic_matrix.at<float>(0, 0) = 1.0f;
    intrinsic_matrix.at<float>(1, 1) = 1.0f;

    // CALIBRATE CAMERA!
    cv::calibrateCamera(object_points, image_points,
            cvGetSize(image),intrinsic_matrix, distortion_coeffs,
            rvecs,tvecs,0 // CV_CALIB_FIX_ASPECT_RATIO
            );
   
    FileStorage fs(OUTPUT_DIR+"intrinsics.xml",FileStorage::WRITE);
    fs << "camera_intrinsics" << intrinsic_matrix; fs.release();
    fs = FileStorage(OUTPUT_DIR+"distortions.xml",FileStorage::WRITE);
    fs << "camera_distortions" << distortion_coeffs; fs.release();
    fs = FileStorage(OUTPUT_DIR+"rotations.xml",FileStorage::WRITE);
    for (int i = 0; i < rvecs.size(); i++)
    {
        char name[100];
        sprintf(name, "camera_rotation%d", i);
        fs << string(name) << rvecs.at(i);
    } fs.release();
    fs = FileStorage(OUTPUT_DIR+"translations.xml",FileStorage::WRITE);
    for (int i = 0; i < tvecs.size(); i++)
    {
        char name[100];
        sprintf(name, "camera_translation%d", i);
        fs << string(name) << tvecs.at(i);
    } fs.release();

    return;
}

void optimize_robot_camera_transformation()
{
    return;
}

int main(int argc, char* argv[])
{
    calibrate_camera(IMAGE_DIR);
    return 0;
}
