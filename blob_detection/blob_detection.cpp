/*
 * blob_detection.cpp
 *
 *  Created on: Dec 18, 2011
 *      Author: ferraraf
 */


#include "blob_detection.hpp"


namespace blob_detection
{
// Constructor
BlobDetector::BlobDetector( CvSize img_size, \
							CvScalar hsv_min, \
						  	CvScalar hsv_max, \
						  	unsigned int num_points, \
						  	unsigned int n )
{
	// Set the properties
	this->img_size = img_size;
	this->hsv_min = hsv_min;
	this->hsv_max = hsv_max;
	this->num_points = num_points;
	this->n = n;

	// Preallocate the images
	hsv = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );
	thresholded = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );
	filtered = cvCreateImage( img_size, IPL_DEPTH_8U, 1 );
	label = cvCreateImage( img_size, IPL_DEPTH_LABEL, 1 );

#if VISUALIZE
	frame = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );
#endif

	// Set up the filter mask
	morph_kernel = cvCreateStructuringElementEx( 5, 5, 1, 1, CV_SHAPE_RECT, NULL );

#if VISUALIZE
	// Create windows
	cvNamedWindow( "Original", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "HSV", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Thresholded", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Filtered", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Blobs", CV_WINDOW_AUTOSIZE );
#endif
}

// Read the image
void BlobDetector::read_img( Mat original )
{
	this->original = original;
}

// Process the image
CvBlobs BlobDetector::proc_img()
{
	// Convert original to HSV
	cvCvtColor( &original, hsv, CV_BGR2HSV );

	// Filter by color
	cvInRangeS( hsv, hsv_min, hsv_max, thresholded );

	// Filter the noise
	cvMorphologyEx( thresholded, filtered, NULL, morph_kernel, CV_MOP_OPEN, 1 );

	// Blob detection
	CvBlobs blobs;
	cvLabel( filtered, label, blobs );

#if VISUALIZE
	// Copy the original image for the blob rendering
	cvConvertScale( &original, frame, 1, 0 );
#endif
	return blobs;
}
}
