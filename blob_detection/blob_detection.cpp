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
CvBlob BlobDetector::proc_img()
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
	cvFilterByArea( blobs, 100, 100000 );
	CvBlobs::iterator it = blobs.begin();
	CvBlob * blob = it->second;

#if VISUALIZE
	// Copy the original image for the blob rendering
	cvConvertScale( &original, frame, 1, 0 );
#endif
	return *blob;
}

// Get the blob contour
vector< vector< unsigned int > > BlobDetector::get_contour( CvBlob * blob )
{
	// Vector for the contour
	vector< vector< unsigned int > > blob_contour;

	// Vector for a point
	vector< unsigned int > blob_point;
	blob_point.resize( 2 );

	// First contour point
	unsigned int x = blob->contour.startingPoint.x;
	unsigned int y = blob->contour.startingPoint.y;

	// First contour point
	blob_point[ 0 ] = x;
	blob_point[ 1 ] = y;

	// Put the first contour point into the contour vector
	blob_contour.push_back( blob_point );

	for ( CvChainCodes::const_iterator it = blob->contour.chainCode.begin(); it != blob->contour.chainCode.end(); it++ )
	{
		// Move along the contour
		x = x + cvChainCodeMoves[ *it ][ 0 ];
		y = y + cvChainCodeMoves[ *it ][ 1 ];

		// Contour point
		blob_point[ 0 ] = x;
		blob_point[ 1 ] = y;

		// Put the contour point into the contour vector
		blob_contour.push_back( blob_point );
	}
	return blob_contour;
}
}
