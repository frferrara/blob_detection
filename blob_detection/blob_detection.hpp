/*
 * blob_detection.hpp
 *
 *  Created on: Dec 18, 2011
 *      Author: ferraraf
 */

#ifndef BLOB_DETECTION_HPP_
#define BLOB_DETECTION_HPP_


// STL
#include <iostream>
#include <vector>

// Computer vision
#include <opencv2/opencv.hpp>
#include <cvblob.h>

// Multi threaded circle detection
#include <mt_circdet.hpp>


using namespace std;
using namespace cv;
using namespace cvb;


#define VISUALIZE 0
#define VISUALIZE_DET 0


namespace blob_detection
{
class BlobDetector
{
	// Image size
	Size img_size;

	// HSV Thresholds
	Scalar hsv_min, hsv_min__ROI, hsv_max, hsv_max__ROI;

	// Needed images
	Mat original, hsv, thresholded, filtered, frame, label;

	// Noise suppressing mask
	Mat morph_kernel;

	// Parameters for the circle detection
	unsigned int num_points, n;

	// Histograms for the circle detection
	gsl_histogram * hist_r;
	gsl_histogram2d * hist__x_c;

	// Image ROI
	Rect ROI;

	// ROI flag
	bool flag_ROI;

	// Read the image
	void read_img( const Mat & original );

	// Process the image
	CvBlobs proc_img();

	// Process the image ROI
	CvBlobs proc_roi( const Mat & original, \
					  const Size & ROI_size );

	// Get the blob contour
	vector< vector< unsigned int > > get_contour( CvBlob * blob );

	// Detect the circle in the image
	void circdet( const vector< vector< unsigned int > > & blob_contour, \
				  size_t & x_c, \
				  size_t & y_c, \
				  size_t & r );

	// Draw the blob and circle
	void draw( CvBlobs blobs, \
			   const Point & x_c, \
			   const size_t & r, \
			   const Scalar & color );

	// Show the images
	void show_img();

public:
	// Constructor
	BlobDetector( const Size & img_size, \
				  const Scalar & hsv_min, \
				  const Scalar & hsv_max, \
				  unsigned int num_points, \
				  unsigned int n );

	// Destructor
	~BlobDetector();

	// Blob detection
	void blob_detection( const Mat & original, \
			   	   	     size_t & x_c, \
			   	   	     size_t & y_c, \
			   	   	     size_t & r );
};
}

#endif /* BLOB_DETECTION_HPP_ */
