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
#define VISUALIZE_DET 1


namespace blob_detection
{
class BlobDetector
{
	// Image size
	CvSize img_size;

	// HSV Thresholds
	CvScalar hsv_min, hsv_min__ROI, hsv_max, hsv_max__ROI;

	// Needed images
	IplImage original;//, hsvROI, thresholdedROI, filteredROI, frameROI, labelROI;
	IplImage * hsv;
	IplImage * thresholded;
	IplImage * filtered;
	IplImage * frame;
	IplImage * label;

	// Noise suppressing mask
	IplConvKernel * morph_kernel;

	// Parameters for the circle detection
	unsigned int num_points, n;

	// Histograms for the circle detection
	gsl_histogram * hist_r;
	gsl_histogram2d * hist__x_c;

	// Image ROI
	CvRect ROI;

	// ROI flag
	bool flag_ROI;

	// Read the image
	void read_img( const Mat & original );

	// Process the image
	CvBlobs proc_img();

	// Process the image ROI
	CvBlobs proc_roi( const IplImage & original, \
					  const CvSize & ROI_size );

	// Get the blob contour
	vector< vector< unsigned int > > get_contour( CvBlob * blob );

	// Detect the circle in the image
	void circdet( const vector< vector< unsigned int > > & blob_contour, \
				  size_t & x_c, \
				  size_t & y_c, \
				  size_t & r );

	// Draw the blob and circle
	void draw( CvBlobs blobs, \
			   const CvPoint & x_c, \
			   const size_t & r, \
			   const CvScalar & color );

	// Show the images
	void show_img();

public:
	// Constructor
	BlobDetector( const CvSize & img_size, \
				  const CvScalar & hsv_min, \
				  const CvScalar & hsv_max, \
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
