/*
 * main.cpp
 *
 *  Created on: Dec 12, 2011
 *      Author: ferraraf
 */


#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <cvblob.h>
#include <vector>
#include <rand_gen.hpp>
#include <circdet.hpp>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_histogram2d.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <blob_detection.hpp>


using namespace std;
using namespace cv;
using namespace cvb;
using namespace tbb;


/*vector< vector< unsigned int > > get_contour( CvBlob * blob )
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
}*/

int main( int argc, char * argv[] )
{
	if ( argc == 1 )
	{
		cout << "Specify file name of the image!\n";

		return -1;
	}
    
    cout << endl << argc << ", " << argv[ 1 ] << endl;
    


    // Create images
//    Mat original, hsv, thresholded, eroded, dilated;
//
//    // Create windows
//	cvNamedWindow( "Original", CV_WINDOW_AUTOSIZE );
//	cvNamedWindow( "HSV", CV_WINDOW_AUTOSIZE );
//	cvNamedWindow( "Thresholded", CV_WINDOW_AUTOSIZE );
//    cvNamedWindow( "Blobs", CV_WINDOW_AUTOSIZE );
    
    // Read the image
    Mat original = imread( argv[ 1 ] );

    double h_min = 53.0;
    double h_max = 95.0;
    double s_min = 65.0;
    double s_max = 255.0;
    double v_min = 0.0;
    double v_max = 255.0;

    unsigned int num_points = 5;
    unsigned int n = 100;

    // Blob detector
    blob_detection::BlobDetector blob_detector( cvSize( original.cols, original.rows ), \
    											cvScalar( h_min, s_min, v_min, 0.0 ), \
    											cvScalar( h_max, s_max, v_max, 0.0 ), \
    											num_points, \
    											n );

    size_t x_c, y_c, r;
    while (1)
    {
    	// Detect the ball
    	blob_detector.blob_detection( original, x_c, y_c, r );

    	cout << "x_c: " << x_c << \
    			", y_c: " << y_c << \
    			", r: " << r << endl;
    }

    // Show the image
//    imshow( "Original", original );
//    //cvWaitKey();
//
//    // Convert to HSV
//    cvtColor( original, hsv, CV_BGR2HSV );
//
//    // Show the image
//    imshow( "HSV", hsv );
//    //cvWaitKey();
//
//    // Filter the image
//    double h_min = 53.0;
//    double h_max = 95.0;
//    double s_min = 65.0;
//    double s_max = 255.0;
//    double v_min = 0.0;
//    double v_max = 255.0;
//    Scalar hsv_min = Scalar_< double >( h_min, s_min, v_min, 0.0 );
//    Scalar hsv_max = Scalar_< double >( h_max, s_max, v_max, 0.0 );
//    inRange( hsv, hsv_min, hsv_max, thresholded );
//
//    // Show the image
//    imshow( "Thresholded", thresholded );
//    //cvWaitKey();
//
//    // Copy image
//    IplImage img_original = original;
//    IplImage img_thresholded = thresholded;
//
//    // Stuff for cv blob
//    CvTracks tracks;
//    CvSize img_size = cvGetSize( &img_original );
//    IplImage * frame = cvCreateImage( img_size, img_original.depth, img_original.nChannels );
//    IplConvKernel * morphKernel = cvCreateStructuringElementEx( 5, 5, 1, 1, CV_SHAPE_RECT, NULL );
//    cvConvertScale( &img_original, frame, 1, 0 );
//
//    cvMorphologyEx( &img_thresholded, &img_thresholded, NULL, morphKernel, CV_MOP_OPEN, 1 );
//
//    // Show the image
//    cvShowImage( "Thresholded", &img_thresholded );
//    cvWaitKey();
//
//    IplImage * labelImg = cvCreateImage( cvGetSize( frame ), IPL_DEPTH_LABEL, 1 );
//
//    CvBlobs blobs;
//    unsigned int result = cvLabel( &img_thresholded, labelImg, blobs );
//    //cvFilterByArea( blobs, 100, 1000000 );
//    //cvRenderBlobs( labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX );
//    //cvUpdateTracks( blobs, tracks, 100.0, 5 );
//    //cvRenderTracks( tracks, frame, frame, CV_TRACK_RENDER_ID | CV_TRACK_RENDER_BOUNDING_BOX );
//    CvBlobs::iterator it = blobs.begin();
//    CvBlob * blob = it->second;
//    cvRenderContourChainCode( &blob->contour, frame, cvScalar( 0.0, 0.0, 255.0 ) );
//    cvShowImage( "Blobs", frame );
//    cvWaitKey();
//
//    vector< vector< unsigned int > > blob_contour = get_contour( blob );
//
//    /*for ( int i = 0; i < ( int )blob_contour.size(); i++ )
//    {
//    	cout << "x: " << blob_contour[ i ][ 0 ] << ", " << \
//    			"y: " << blob_contour[ i ][ 1 ] << endl;
//    }*/
//
//    int num_points = 5;
//    int n = 100;
//
//    vector< double > rand_num;
//    MatrixXd x;
//    x.resize( num_points, 2 );
//    Vector2d x_c;
//    double r;
//
//    cout << endl;
//
//    // Allocate the histograms
//    gsl_histogram * hist = gsl_histogram_calloc_uniform( ( size_t )img_size.height, \
//    													 0.0, ( double )img_size.height );
//    gsl_histogram2d * hist2d = gsl_histogram2d_calloc_uniform( ( size_t )img_size.width, \
//    												  ( size_t )img_size.height, \
//    												  0.0, ( double )img_size.width, \
//    												  0.0, ( double )img_size.height );
//
//    // Generate random indices
//    rand_num = multi_uniGen( n * num_points, 0, ( double )blob_contour.size() - 1.0 );
//
//    // Index for the contour indices
//    int k = 0;
//
//    for ( int i = 0; i < n; i++ )
//    {
//     	// Put the respective values into the data matrix
//    	for ( int j = 0; j < num_points; j++ )
//    	{
//    		x( j, 0 ) = ( double )blob_contour[ ( int )round( rand_num[ k + j ] ) ][ 0 ];
//    		x( j, 1 ) = ( double )blob_contour[ ( int )round( rand_num[ k + j ] ) ][ 1 ];
//
//    		//cout << "x: " << x( j, 0 ) << ", y: " << x( j, 1 ) << endl;
//    	}
//    	// Fit a circle
//    	circdet::detect_circle( x, x_c, r );
//
//    	//cout << "x_c: " << x_c.transpose() << ", r: " << r << endl;
//    	cout << "x_c: " << round( x_c( 0 ) ) << " " << round( x_c( 1 ) ) << ", r: " << round( r ) << endl;
//
//    	// Increment histogram
//    	gsl_histogram_increment( hist, r );
//    	gsl_histogram2d_increment( hist2d, round( x_c( 0 ) ), round( x_c( 1 ) ) );
//
//    	// Increment k
//    	k = k + num_points;
//    }
//
//    cout << gsl_histogram_max_bin( hist ) << endl;
//
//    size_t l = gsl_histogram_max_bin( hist );
//    size_t i, j;
//    gsl_histogram2d_max_bin( hist2d, &i, &j );
//
//    cout << "x: " << i << ", y: " << j << ", r: " << l << endl;
//
//    cvCircle( frame, cvPoint( i, j ), l, cvScalar( 0.0, 255.0, 0.0 ), 1, 8, 0 );
//    cvCircle( frame, cvPoint( i, j ), 1, cvScalar( 0.0, 255.0, 0.0 ), 1, 8, 0 );
//    cvCircle( &img_thresholded, cvPoint( i, j ), l, cvScalar( 0.0, 0.0, 255.0 ), 1, 8, 0 );
//    cvCircle( &img_thresholded, cvPoint( i, j ), 1, cvScalar( 0.0, 0.0, 255.0 ), 1, 8, 0 );
//
//    cvShowImage( "Blobs", frame );
//    cvShowImage( "Thresholded", &img_thresholded );
//    cvWaitKey();

	return 0;
}
