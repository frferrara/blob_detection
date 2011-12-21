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

	// Create the needed images
	IplImage filtered_roi__ipl = ( IplImage )filtered_roi;
	IplImage * label_roi__ipl = cvCreateImage( cvSize( original_roi.cols, original_roi.rows ), IPL_DEPTH_LABEL, 1 );

	// Blob detection
	CvBlobs blobs;
	cvLabel( &filtered_roi__ipl, label_roi__ipl, blobs );

	// Release the label image
	cvReleaseImage( &label_roi__ipl );

	return blobs;
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

// Set the roi
void BlobDetector::set_roi()
{
	if ( ROI.x < 0 )
	{
		ROI.x = 0;
	}

	if ( ROI.x + ROI.width > img_size.width )
	{
		ROI.width = img_size.width - ROI.x;
	}

	if ( ROI.y < 0 )
	{
		ROI.y = 0;
	}

	if ( ROI.y + ROI.height > img_size.height )
	{
		ROI.height = img_size.height - ROI.y;
	}
}

// Detect with ROI
bool BlobDetector::detect_roi( CvBlobs & blobs, \
							   size_t & x_c, \
							   size_t & y_c, \
							   size_t & r )
{
	// Set the ROI
	Mat original_roi = original( ROI );

	// Process the ROI
	blobs = proc_roi( original_roi );

	if ( !blobs.empty() )
	{
		// Get the blob contour
		vector< vector< unsigned int > > blob_contour = get_contour( blobs.begin()->second );

		// Detect the circle in the image
		circdet( blob_contour, x_c, y_c, r );

		// Transform the ball center from ROI coordinates to image coordinates
		x_c = x_c + ROI.x;
		y_c = y_c + ROI.y;

		// Get the new image ROI
		int side = 2 * r + 20;
		ROI = Rect( ( int )( x_c - r ) - 10, \
					( int )( y_c - r ) - 10, \
					side, \
					side );

		// Set the roi
		set_roi();

		// Set the ROI flag
		flag_ROI = true;
	}
	else
	{
		// Set the ROI flag
		flag_ROI = false;
	}
	return flag_ROI;
}

// Detect without ROI
bool BlobDetector::detect( CvBlobs & blobs, \
						   size_t & x_c, \
			 	 	 	   size_t & y_c, \
			 	 	 	   size_t & r )
{
	// Process the image
	blobs = proc_img();

	if ( !blobs.empty() )
	{
		// Get the blob contour
		vector< vector< unsigned int > > blob_contour = get_contour( blobs.begin()->second );

		// Detect the circle in the image
		circdet( blob_contour, x_c, y_c, r );

		// Get the image ROI
		int side = 2 * r + 20;
		ROI = Rect( ( int )( x_c - r ) - 10, \
					( int )( y_c - r ) - 10, \
					side, \
					side );

		// Set the roi
		set_roi();

		flag_ROI = true;
	}
	else
	{
		flag_ROI = false;
	}
	return flag_ROI;
}

// Transform the blobs from ROI to image coordinates
void BlobDetector::transform_blobs( CvBlobs & blobs )
{
	// Iterate through the blobs
	for ( CvBlobs::iterator b_it = blobs.begin(); b_it != blobs.end(); b_it++ )
	{
		// Move the centroid
		b_it->second->centroid.x = b_it->second->centroid.x + ( double )ROI.x;
		b_it->second->centroid.y = b_it->second->centroid.x + ( double )ROI.y;

		// Move the bounding box
		b_it->second->minx = b_it->second->minx + ( unsigned int )ROI.x;
		b_it->second->miny = b_it->second->miny + ( unsigned int )ROI.y;
		b_it->second->maxx = b_it->second->maxx + ( unsigned int )ROI.x;
		b_it->second->maxy = b_it->second->maxy + ( unsigned int )ROI.y;
	}
}

// Draw the blob and circle in the ROI
void BlobDetector::draw_roi( CvBlobs blobs, \
			   	   	   	     const Point & x_c, \
			   	   	   	     const size_t & r, \
			   	   	   	     const Scalar & color )
{
	// Transform the blobs from ROI to image coordinates
	transform_blobs( blobs );

	// Do some image copies
	IplImage original_ipl = ( IplImage )original;
	IplImage label_ipl = ( IplImage )label;
	IplImage * frame_ipl = cvCloneImage( &original_ipl );

	// Render the blobs
	cvRenderBlobs( &label_ipl, blobs, frame_ipl, frame_ipl, CV_BLOB_RENDER_BOUNDING_BOX );

	// Copy the blob image
	frame = ( Mat )cvCloneImage( frame_ipl );

	// Release the blob image that is not needed anymore
	cvReleaseImage( &frame_ipl );

	// Draw the circle on the image
	circle( frame, x_c, 1, color, -1, 8, 0 );
	circle( frame, x_c, ( int )r, color, 1, 8, 0 );

	// Draw the ROI around the circle
	rectangle( frame, ROI, Scalar( 0.0, 255.0, 0.0 ), 1, 8, 0 );
}

// Draw the circle
void BlobDetector::draw( CvBlobs blobs, \
						 const Point & x_c, \
						 const size_t & r, \
						 const Scalar & color )
{
	// Do some image copies
	IplImage original_ipl = ( IplImage )original;
	IplImage label_ipl = ( IplImage )label;
	IplImage * frame_ipl = cvCloneImage( &original_ipl );

	// Render the blobs
	cvRenderBlobs( &label_ipl, blobs, frame_ipl, frame_ipl, CV_BLOB_RENDER_BOUNDING_BOX );

	// Copy the blob image
	frame = ( Mat )cvCloneImage( frame_ipl );

	// Release the blob image that is not needed anymore
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

	imshow( "Blobs", frame );

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

#if FLAG_ROI
	if ( flag_ROI == true )
	{
		// Detect with ROI
		CvBlobs blobs;
		flag_ROI = detect_roi( blobs, x_c, y_c, r );

#if VISUALIZE || VISUALIZE_DET
		if ( flag_ROI == true )
		{
			// Draw the blob and circle in the ROI
			draw_roi( blobs, Point( x_c, y_c ), r, Scalar( 255.0, 0.0, 0.0 ) );
		}
#endif
	}
	else
	{
		// Detect without ROI
		CvBlobs blobs;
		flag_ROI = detect( blobs, x_c, y_c, r );

#if VISUALIZE || VISUALIZE_DET
		if ( flag_ROI == true )
		{
			// Draw the blob and circle
			draw( blobs, Point( x_c, y_c ), r, Scalar( 255.0, 0.0, 0.0 ) );
		}
#endif
	}
#else
	// Detect without ROI
	CvBlobs blobs;
	if ( detect( blobs, x_c, y_c, r ) )
	{
#if VISUALIZE || VISUALIZE_DET
		// Draw the blob and circle
		draw( blobs, Point( x_c, y_c ), r, Scalar( 255.0, 0.0, 0.0 ) );
#endif
	}
#endif

#if VISUALIZE || VISUALIZE_DET
	// Show the images
	show_img();
#endif
}
}
