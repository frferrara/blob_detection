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


#define VISUALIZE 1


namespace blob_detection
{
class BlobDetector
{
	// Image size
	CvSize img_size;

	// HSV Thresholds
	CvScalar hsv_min, hsv_max;

	// Needed images
	IplImage original;
	IplImage * hsv;
	IplImage * thresholded;
	IplImage * filtered;
	IplImage * frame;
	IplImage * label;

	// Noise suppressing mask
	IplConvKernel * morph_kernel;

	// Tracks for visualization
	CvTracks tracks;

	// Parameters for the circle detection
	unsigned int num_points, n;

	// Histograms for the circle detection
	gsl_histogram * hist_r;
	gsl_histogram2d * hist__x_c;

	// Read the image
	void read_img( Mat original );

	// Process the image
	CvBlob proc_img();

	// Get the blob contour
	vector< vector< unsigned int > > get_contour( CvBlob * blob );

public:
	// Constructor
	BlobDetector( CvSize img_size, \
				  CvScalar hsv_min, \
				  CvScalar hsv_max, \
				  unsigned int num_points, \
				  unsigned int n );

	// Destructor
	~BlobDetector();
};
}

#endif /* BLOB_DETECTION_HPP_ */
