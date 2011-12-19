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
BlobDetector::BlobDetector( const CvSize & img_size, \
							const CvScalar & hsv_min, \
						  	const CvScalar & hsv_max, \
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

#if VISUALIZE || VISUALIZE_DET
	frame = cvCreateImage( img_size, IPL_DEPTH_8U, 3 );
#endif

	// Set up the filter mask
	morph_kernel = cvCreateStructuringElementEx( 5, 5, 1, 1, CV_SHAPE_RECT, NULL );

	// Allocate the histograms
	hist_r = gsl_histogram_calloc_uniform( ( size_t )img_size.height, \
										   0.0, ( double )img_size.height );
	hist__x_c = gsl_histogram2d_calloc_uniform( ( size_t )img_size.width, \
											    ( size_t )img_size.height, \
											    0.0, ( double )img_size.width, \
											    0.0, ( double )img_size.height );

	// Set the ROI flag
	flag_ROI = false;

	// Create windows
#if VISUALIZE
	cvNamedWindow( "Original", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "HSV", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Thresholded", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Filtered", CV_WINDOW_AUTOSIZE );
#endif

#if VISUALIZE || VISUALIZE_DET
	cvNamedWindow( "Blobs", CV_WINDOW_AUTOSIZE );
#endif
}

// Destructor
BlobDetector::~BlobDetector()
{
	// Release the images
	cvReleaseImage( &hsv );
	cvReleaseImage( &thresholded );
	cvReleaseImage( &filtered );
	cvReleaseImage( &frame );
	cvReleaseImage( &label );

	// Release the noise suppressing mask
	cvReleaseStructuringElement( &morph_kernel );

	// Release the histograms
	gsl_histogram_free( hist_r );
	gsl_histogram2d_free( hist__x_c );

	// Destroy the windows
#if VISUALIZE
	cvDestroyWindow( "Original" );
	cvDestroyWindow( "HSV" );
	cvDestroyWindow( "Thresholded" );
	cvDestroyWindow( "Filtered" );
#endif

#if VISUALIZE || VISUALIZE_DET
	cvDestroyWindow( "Blobs" );
#endif
}

// Read the image
void BlobDetector::read_img( const Mat & original )
{
	this->original = ( IplImage )original;
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
	cvFilterByArea( blobs, 100, 100000 );

	return blobs;
}

// Process the image ROI
CvBlobs BlobDetector::proc_roi( const IplImage & original, \
								const CvSize & ROI_size )
{
	// Convert original to HSV
	IplImage * hsv_roi = cvCreateImage( ROI_size, IPL_DEPTH_8U, 3 );
	cvCvtColor( &original, hsv_roi, CV_BGR2HSV );

	// Filter by color
	IplImage * thresholded_roi = cvCreateImage( ROI_size, IPL_DEPTH_8U, 1 );
	cvInRangeS( hsv_roi, hsv_min, hsv_max, thresholded_roi );

	// Filter the noise
	IplImage * filtered_roi = cvCreateImage( ROI_size, IPL_DEPTH_8U, 1 );
	cvMorphologyEx( thresholded_roi, filtered_roi, NULL, morph_kernel, CV_MOP_OPEN, 1 );

	// Blob detection
	IplImage * label_roi = cvCreateImage( ROI_size, IPL_DEPTH_LABEL, 1 );
	CvBlobs blobs;
	cvLabel( filtered, label, blobs );
	cvFilterByArea( blobs, 100, 100000 );

	// Release the images
	cvReleaseImage( &hsv_roi );
	cvReleaseImage( &thresholded_roi );
	cvReleaseImage( &filtered_roi );
	cvReleaseImage( &label_roi );

	return blobs;
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

// Detect the circle in the image
void BlobDetector::circdet( const vector< vector< unsigned int > > & blob_contour, \
						    size_t & x_c, \
			  	  	  	    size_t & y_c, \
							size_t & r )
{
	// Multi-threaded circle detection
	parallel_for( blocked_range< size_t >( 0, n ), \
				  mt_circdet::CircleDetector( num_points, blob_contour, hist_r, hist__x_c ) );

	// Get the circle
	mt_circdet::get_circle( hist_r, hist__x_c, x_c, y_c, r );
}

// Draw the circle
void BlobDetector::draw( CvBlobs blobs, \
						 const CvPoint & x_c, \
						 const size_t & r, \
						 const CvScalar & color )
{
	// Copy the original image for the blob rendering
	cvConvertScale( &original, frame, 1, 0 );

	if ( !blobs.empty() )
	{
		// Render the blobs
		cvRenderBlobs( label, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX );

		// Draw the circle on the images
		cvCircle( frame, x_c, 1, color, -1, 8, 0 );
		cvCircle( frame, x_c, ( int )r, color, 1, 8, 0 );

#if VISUALIZE
		cvCircle( filtered, x_c, 3, color, -1, 8, 0 );
		cvCircle( filtered, x_c, ( int )r, color, 3, 8, 0 );
#endif
	}
}

// Show the images
void BlobDetector::show_img()
{
#if VISUALIZE
	// Show the images
	cvShowImage( "Original", &original );
	cvShowImage( "HSV", hsv );
	cvShowImage( "Thresholded", thresholded );
	cvShowImage( "Filtered", filtered );
#endif
	cvShowImage( "Blobs", frame );

	// Wait
	cvWaitKey( 10 );
}

// Blob detection
void BlobDetector::blob_detection( const Mat & original, \
								   size_t & x_c, \
								   size_t & y_c, \
								   size_t & r )
{
	// Read the image
	read_img( original );

	// Blobs
	CvBlobs blobs;

	if ( flag_ROI == true )
	{
		// Set the ROI
		cvSetImageROI( &this->original, ROI );

		// Initialize the new images
		CvSize ROI_size = cvSize( ROI.width, ROI.height );

		// Process the ROI
		blobs = proc_roi( this->original, ROI_size );
	}
	else
	{
		// Process the image
		blobs = proc_img();
	}

	if ( !blobs.empty() )
	{
		// Get the blob contour
		vector< vector< unsigned int > > blob_contour = get_contour( blobs.begin()->second );

		// Detect the circle in the image
		circdet( blob_contour, x_c, y_c, r );

		// Get the image ROI
		int side = 2 * r + 20;

		ROI = cvRect( ( int )( x_c - r ) - 10, \
					  ( int )( y_c - r ) - 10, \
					  side, \
					  side );

		// Set the ROI flag
		//flag_ROI = true;

#if VISUALIZE || VISUALIZE_DET
		// Draw the blob and circle
		draw( blobs, cvPoint( x_c, y_c ), r, cvScalar( 255.0, 0.0, 0.0 ) );
#endif
	}
	else
	{
		// Set the ROI flag
		flag_ROI = false;
	}

#if VISUALIZE || VISUALIZE_DET
	// Show the images
	show_img();
#endif
}
}
