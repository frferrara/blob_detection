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


using namespace std;
using namespace cv;
using namespace cvb;
using namespace tbb;


class CircleDetector
{

};


vector< vector< unsigned int > > get_contour( CvBlob * blob )
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

int main( int argc, char * argv[] )
{
	if ( argc == 1 )
	{
		cout << "Specify file name of the image!\n";

		return -1;
	}
    
    cout << endl << argc << ", " << argv[ 1 ] << endl;
    
    // Create images
    Mat original, hsv, thresholded, eroded, dilated;

    // Create windows
	cvNamedWindow( "Original", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "HSV", CV_WINDOW_AUTOSIZE );
	cvNamedWindow( "Thresholded", CV_WINDOW_AUTOSIZE );
    cvNamedWindow( "Blobs", CV_WINDOW_AUTOSIZE );
    
    // Read the image
    original = imread( argv[ 1 ] );
    
    // Show the image
    imshow( "Original", original );
    //cvWaitKey();
    
    // Convert to HSV
    cvtColor( original, hsv, CV_BGR2HSV );
    
    // Show the image
    imshow( "HSV", hsv );
    //cvWaitKey();
    
    // Filter the image
    double h_min = 53.0;
    double h_max = 95.0;
    double s_min = 65.0;
    double s_max = 255.0;
    double v_min = 0.0;
    double v_max = 255.0;
    Scalar hsv_min = Scalar_< double >( h_min, s_min, v_min, 0.0 );
    Scalar hsv_max = Scalar_< double >( h_max, s_max, v_max, 0.0 );
    inRange( hsv, hsv_min, hsv_max, thresholded );
    
    // Show the image
    imshow( "Thresholded", thresholded );
    //cvWaitKey();
    
    // Copy image
    IplImage img_original = original;
    IplImage img_thresholded = thresholded;
    
    // Stuff for cv blob
    CvTracks tracks;
    CvSize img_size = cvGetSize( &img_original );
    IplImage * frame = cvCreateImage( img_size, img_original.depth, img_original.nChannels );
    IplConvKernel * morphKernel = cvCreateStructuringElementEx( 5, 5, 1, 1, CV_SHAPE_RECT, NULL );
    cvConvertScale( &img_original, frame, 1, 0 );
    
    cvMorphologyEx( &img_thresholded, &img_thresholded, NULL, morphKernel, CV_MOP_OPEN, 1 );
    
    // Show the image
    cvShowImage( "Thresholded", &img_thresholded );
    cvWaitKey();
    
    IplImage * labelImg = cvCreateImage( cvGetSize( frame ), IPL_DEPTH_LABEL, 1 );
    
    CvBlobs blobs;
    unsigned int result = cvLabel( &img_thresholded, labelImg, blobs );
    //cvFilterByArea( blobs, 100, 1000000 );
    //cvRenderBlobs( labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX );
    //cvUpdateTracks( blobs, tracks, 100.0, 5 );
    //cvRenderTracks( tracks, frame, frame, CV_TRACK_RENDER_ID | CV_TRACK_RENDER_BOUNDING_BOX );
    CvBlobs::iterator it = blobs.begin();
    CvBlob * blob = it->second;
    cvRenderContourChainCode( &blob->contour, frame, cvScalar( 0.0, 0.0, 255.0 ) );
    cvShowImage( "Blobs", frame );
    cvWaitKey();

    vector< vector< unsigned int > > blob_contour = get_contour( blob );

    /*for ( int i = 0; i < ( int )blob_contour.size(); i++ )
    {
    	cout << "x: " << blob_contour[ i ][ 0 ] << ", " << \
    			"y: " << blob_contour[ i ][ 1 ] << endl;
    }*/

    int num_points = 5;
    int n = 100;

    vector< double > rand_num;
    MatrixXd x;
    x.resize( num_points, 2 );
    Vector2d x_c;
    double r;

    cout << endl;

    // Allocate the histograms
    gsl_histogram * hist = gsl_histogram_calloc_uniform( ( size_t )img_size.height, \
    													 0.0, ( double )img_size.height );
    gsl_histogram2d * hist2d = gsl_histogram2d_calloc_uniform( ( size_t )img_size.width, \
    												  ( size_t )img_size.height, \
    												  0.0, ( double )img_size.width, \
    												  0.0, ( double )img_size.height );

    // Generate random indices
    rand_num = multi_uniGen( n * num_points, 0, ( double )blob_contour.size() - 1.0 );

    // Index for the contour indices
    int k = 0;

    for ( int i = 0; i < n; i++ )
    {
     	// Put the respective values into the data matrix
    	for ( int j = 0; j < num_points; j++ )
    	{
    		x( j, 0 ) = ( double )blob_contour[ ( int )round( rand_num[ k + j ] ) ][ 0 ];
    		x( j, 1 ) = ( double )blob_contour[ ( int )round( rand_num[ k + j ] ) ][ 1 ];

    		//cout << "x: " << x( j, 0 ) << ", y: " << x( j, 1 ) << endl;
    	}
    	// Fit a circle
    	circdet::detect_circle( x, x_c, r );

    	//cout << "x_c: " << x_c.transpose() << ", r: " << r << endl;
    	cout << "x_c: " << round( x_c( 0 ) ) << " " << round( x_c( 1 ) ) << ", r: " << round( r ) << endl;

    	// Increment histogram
    	gsl_histogram_increment( hist, r );
    	gsl_histogram2d_increment( hist2d, round( x_c( 0 ) ), round( x_c( 1 ) ) );

    	// Increment k
    	k = k + num_points;
    }

    cout << gsl_histogram_max_bin( hist ) << endl;

    size_t l = gsl_histogram_max_bin( hist );
    size_t i, j;
    gsl_histogram2d_max_bin( hist2d, &i, &j );

    cout << "x: " << i << ", y: " << j << ", r: " << l << endl;

    cvCircle( frame, cvPoint( i, j ), l, cvScalar( 0.0, 0.0, 255.0 ), 1, 8, 0 );
    cvCircle( frame, cvPoint( i, j ), 1, cvScalar( 0.0, 0.0, 255.0 ), 1, 8, 0 );
    cvCircle( &img_thresholded, cvPoint( i, j ), l, cvScalar( 0.0, 0.0, 255.0 ), 1, 8, 0 );
    cvCircle( &img_thresholded, cvPoint( i, j ), 1, cvScalar( 0.0, 0.0, 255.0 ), 1, 8, 0 );

    cvShowImage( "Blobs", frame );
    cvShowImage( "Thresholded", &img_thresholded );
    cvWaitKey();

//
//    int h_min = 53;
//	int h_max = 95;
//	int s_min = 65;
//	int s_max = 255;
//	int v_min = 0;
//	int v_max = 255;
//
//	IplImage * img = cvLoadImage( argv[ 1 ] );
//
//	cvShowImage( "Original", img );
//
//	CvSize imgSize = cvGetSize( img );
//
//	IplImage * frame = cvCreateImage( imgSize, img->depth, img->nChannels );
//
//	IplConvKernel * morphKernel = cvCreateStructuringElementEx( 5, 5, 1, 1, CV_SHAPE_RECT, NULL );
//
//	unsigned int blobNumber = 0;
//
//	cvConvertScale( img, frame, 1, 0 );
//
//	IplImage * segmentated = cvCreateImage( imgSize, 8, 1 );
//	IplImage * hsv = cvCreateImage( imgSize, 8, 3 );
//	IplImage * dilated = cvCreateImage( imgSize, 8, 1 );
//	IplImage * eroded = cvCreateImage( imgSize, 8, 1 );
//
//	while ( 1 )
//	{
//		int h_min = cvGetTrackbarPos( "h_min", "segmentated" );
//		int h_max = cvGetTrackbarPos( "h_max", "segmentated" );
//		int s_min = cvGetTrackbarPos( "s_min", "segmentated" );
//		int s_max = cvGetTrackbarPos( "s_max", "segmentated" );
//		int v_min = cvGetTrackbarPos( "v_min", "segmentated" );
//		int v_max = cvGetTrackbarPos( "v_max", "segmentated" );
//
//		/*cout << "h_min: " << h_min << endl;
//		cout << "h_max: " << h_max << endl;
//		cout << "s_min: " << s_min << endl;
//		cout << "s_max: " << s_max << endl;
//		cout << "v_min: " << v_min << endl;
//		cout << "v_max: " << v_max << endl;*/
//
//		CvScalar hsv_min = cvScalar( ( double )h_min, ( double )s_min, ( double )v_min, 0.0 );
//		CvScalar hsv_max = cvScalar( ( double )h_max, ( double )s_max, ( double )v_max, 0.0 );
//
//		/*cout << "hsv_min: " << hsv_min.val[ 0 ] << ", " << \
//							   hsv_min.val[ 1 ] << ", " << \
//							   hsv_min.val[ 2 ] << ", " << \
//							   hsv_min.val[ 3 ] << endl;
//		cout << "hsv_max: " << hsv_max.val[ 0 ] << ", " << \
//							   hsv_max.val[ 1 ] << ", " << \
//							   hsv_max.val[ 2 ] << ", " << \
//							   hsv_max.val[ 3 ] << endl;*/
//
//		cvCvtColor( img, hsv, CV_BGR2HSV );
//
//		cvInRangeS( hsv, hsv_min, hsv_max, segmentated );
//
//		cvShowImage( "segmentated", segmentated );
//
//		cvErode( segmentated, eroded );
//
//		/*cvErode( const CvArr* src, CvArr* dst,
//				IplConvKernel* element CV_DEFAULT(NULL),
//				int iterations CV_DEFAULT(1) );*/
//
//		cvDilate( eroded, dilated );
//
//		cvShowImage( "dilated", dilated );
//
//		/*void erode( InputArray src, OutputArray dst, InputArray kernel,
//				Point anchor=Point(-1,-1), int iterations=1,
//				int borderType=BORDER_CONSTANT,
//				const Scalar& borderValue=morphologyDefaultBorderValue() );*/
//
//		cvWaitKey( 1 );
//	}
//
//	// Detecting red pixels:
//	// (This is very slow, use direct access better...)
//	for ( unsigned int j = 0; j < imgSize.height; j++ )
//	{
//		for (unsigned int i=0; i < imgSize.width; i++ )
//		{
//			CvScalar c = cvGet2D( frame, j, i );
//
//			double b = ( ( double )c.val[ 0 ] ) / 255.0;
//			double g = ( ( double )c.val[ 1 ] ) / 255.0;
//			double r = ( ( double )c.val[ 2 ] ) / 255.0;
//			//int f = 255 * ( ( r > 0.1 + g ) && ( r > 0.2 + b ) );
//			int f = 255 * ( r > 0.1 );
//
//			cvSet2D( segmentated, j, i, CV_RGB( f, f, f ) );
//		}
//	}
//
//	cvMorphologyEx( segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1 );
//
//	cvShowImage( "segmentated", segmentated );
//
//	IplImage * labelImg = cvCreateImage( cvGetSize( frame ), IPL_DEPTH_LABEL, 1 );
//
//	CvBlobs blobs;
//	unsigned int result = cvLabel( segmentated, labelImg, blobs );
//	cvFilterByArea( blobs, 500, 1000000 );
//	cvRenderBlobs( labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX );
//	cvUpdateTracks( blobs, tracks, 200.0, 5 );
//	cvRenderTracks( tracks, frame, frame, CV_TRACK_RENDER_ID | CV_TRACK_RENDER_BOUNDING_BOX );
//
//	cvShowImage( "red_object_tracking", frame);
//
//	/*stringstream filename;
//	filename << "redobject_" << setw( 5 ) << setfill( '0' ) << frameNumber << ".png";
//	cvSaveImage( filename.str().c_str(), frame );*/
//
//	cvReleaseImage( &labelImg );
//	cvReleaseImage( &segmentated );
//
//	char k = cvWaitKey();
//	switch ( k )
//	{
//	  case 27:
//
//	  case 'q':
//
//	  case 'Q':
//		  break;
//
//	  case 's':
//
//	  case 'S':
//		  for ( CvBlobs::const_iterator it = blobs.begin(); it != blobs.end(); ++it )
//		  {
//			  stringstream filename;
//			  filename << "redobject_blob_" << setw( 5 ) << setfill( '0' ) << blobNumber << ".png";
//			  cvSaveImageBlob( filename.str().c_str(), img, it->second );
//			  blobNumber++;
//
//			  cout << filename.str() << " saved!" << endl;
//		  }
//		  break;
//	}
//
//	cvReleaseBlobs( blobs );
//
//	cvReleaseStructuringElement( &morphKernel );
//	cvReleaseImage( &frame );
//
//	cvDestroyWindow( "original_image" );
//	cvDestroyWindow( "segmentated" );
//	cvDestroyWindow( "dilated" );
//	cvDestroyWindow( "red_object_tracking" );

	return 0;
}
