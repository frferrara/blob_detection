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

// Histograms
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_histogram2d.h>

// Intel threading libs
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

// Own libs
#include <rand_gen.hpp>
#include <circdet.hpp>


using namespace std;
using namespace cv;
using namespace cvb;
using namespace tbb;


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

	// Read the image
	void read_img( Mat original );

	// Process the image
	CvBlobs proc_img();

public:
	// Constructor
	BlobDetector( CvSize img_size, \
				  CvScalar hsv_min, \
				  CvScalar hsv_max, \
				  unsigned int num_points, \
				  unsigned int n );
};
}

#endif /* BLOB_DETECTION_HPP_ */
