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
BlobDetector::BlobDetector( const Size & img_size, \
							const Scalar & hsv_min, \
						  	const Scalar & hsv_max, \
						  	unsigned int num_points, \
						  	unsigned int n )
{
	// Set the properties
	this->img_size = img_size;
	this->hsv_min = hsv_min;
	this->hsv_max = hsv_max;
	this->num_points = num_points;
	this->n = n;

	// Set up the filter mask
	morph_kernel = getStructuringElement( CV_SHAPE_RECT, Size( 5, 5 ), Point( 1, 1 ) );

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
	namedWindow( "Original", CV_WINDOW_AUTOSIZE );
	namedWindow( "HSV", CV_WINDOW_AUTOSIZE );
	namedWindow( "Thresholded", CV_WINDOW_AUTOSIZE );
	namedWindow( "Filtered", CV_WINDOW_AUTOSIZE );
#endif

#if VISUALIZE || VISUALIZE_DET
	namedWindow( "Blobs", CV_WINDOW_AUTOSIZE );
#endif
}

// Destructor
BlobDetector::~BlobDetector()
{
	// Release the histograms
	gsl_histogram_free( hist_r );
	gsl_histogram2d_free( hist__x_c );

	// Destroy the windows
#if VISUALIZE
	destroyWindow( "Original" );
	destroyWindow( "HSV" );
	destroyWindow( "Thresholded" );
	destroyWindow( "Filtered" );
#endif

#if VISUALIZE || VISUALIZE_DET
	destroyWindow( "Blobs" );
#endif
}

// Read the image
void BlobDetector::read_img( const Mat & original )
{
	this->original = original;
}

// Process the image
CvBlobs BlobDetector::proc_img()
{
	// Convert original to HSV
	cvtColor( original, hsv, CV_BGR2HSV );

	// Filter by color
	inRange( hsv, hsv_min, hsv_max, thresholded );

	// Filter the noise
	morphologyEx( thresholded, filtered, MORPH_OPEN, morph_kernel, Point( -1, -1 ), \
				  1, BORDER_CONSTANT, morphologyDefaultBorderValue() );

	// Blob detection
	CvBlobs blobs;
	IplImage filtered_ipl = ( IplImage )filtered;
	IplImage * label_ipl = cvCreateImage( img_size, IPL_DEPTH_LABEL, 1 );
	cvLabel( &filtered_ipl, label_ipl, blobs );
	label = ( Mat )( label_ipl );
	cvFilterByArea( blobs, 100, 100000 );

	// Release the label image
	cvReleaseImage( &label_ipl );

	return blobs;
}

// Process the image ROI
CvBlobs BlobDetector::proc_roi( const Mat & original_roi )
{
	// Convert original to HSV
	Mat hsv_roi;
	cvtColor( original_roi, hsv_roi, CV_BGR2HSV );

	// Filter by color
	Mat thresholded_roi;
	inRange( hsv_roi, hsv_min, hsv_max, thresholded_roi );

	// Filter the noise
	Mat filtered_roi;
	morphologyEx( thresholded_roi, filtered_roi, MORPH_OPEN, morph_kernel, Point( -1, -1 ), \
				  1, BORDER_CONSTANT, morphologyDefaultBorderValue() );

	// Blob detection
	CvBlobs blobs;
	IplImage filtered_roi__ipl = ( IplImage )filtered_roi;
	IplImage * label_roi__ipl = cvCreateImage( cvSize( original_roi.cols, original_roi.rows ), IPL_DEPTH_LABEL, 1 );
	cvLabel( &filtered_roi__ipl, label_roi__ipl, blobs );
//	cvFilterByArea( blobs, 100, 100000 );
//	cvLabel( filtered, label, blobs );
//	cvFilterByArea( blobs, 100, 100000 );
//
//	// Release the images
//	cvReleaseImage( &hsv_roi );
//	cvReleaseImage( &thresholded_roi );
//	cvReleaseImage( &filtered_roi );
//	cvReleaseImage( &label_roi );
	cvReleaseImage( &label_roi__ipl );

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
						 const Point & x_c, \
						 const size_t & r, \
						 const Scalar & color )
{
	// Render the blobs
	IplImage original_ipl = ( IplImage )original;
	IplImage label_ipl = ( IplImage )label;
	IplImage * frame_ipl = cvCloneImage( &original_ipl );
	cvRenderBlobs( &label_ipl, blobs, frame_ipl, frame_ipl, CV_BLOB_RENDER_BOUNDING_BOX );

	frame = ( Mat )cvCloneImage( frame_ipl );

	cvReleaseImage( &frame_ipl );

	// Draw the circle on the images
	circle( frame, x_c, 1, color, -1, 8, 0 );
	circle( frame, x_c, ( int )r, color, 1, 8, 0 );

#if VISUALIZE
	circle( filtered, x_c, 3, color, -1, 8, 0 );
	circle( filtered, x_c, ( int )r, color, 3, 8, 0 );
#endif
}

// Show the images
void BlobDetector::show_img()
{
#if VISUALIZE
	// Show the images
	imshow( "Original", original );
	imshow( "HSV", hsv );
	imshow( "Thresholded", thresholded );
	imshow( "Filtered", filtered );
#endif

#if VISUALIZE || VISUALIZE_DET
	imshow( "Blobs", frame );

	// Wait
	cvWaitKey( 10 );
#endif
}

// Detect with ROI
//bool BlobDetector::detect_roi( size_t & x_c, \
//							   size_t & y_c, \
//							   size_t & r )
//{
//	// Set the ROI
//	Mat original_roi = original( ROI );
//}


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
		Mat original_roi = this->original( ROI );

		// Process the ROI
		blobs = proc_roi( original_roi );
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

		// Transform the ball center from ROI coordinates to image coordinates
		if ( flag_ROI == true )
		{
			x_c = x_c + ROI.x;
			y_c = y_c + ROI.y;
		}

		// Get the image ROI
		int side = 2 * r + 20;

		ROI = Rect( ( int )( x_c - r ) - 10, \
					( int )( y_c - r ) - 10, \
					side, \
					side );

		// Set the ROI flag
		flag_ROI = true;

#if VISUALIZE || VISUALIZE_DET
		// Draw the blob and circle
		draw( blobs, Point( x_c, y_c ), r, Scalar( 255.0, 0.0, 0.0 ) );
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
