/*
 * main.cpp
 *
 *  Created on: Dec 12, 2011
 *      Author: ferraraf
 */


#include <blob_detection.hpp>


int main( int argc, char * argv[] )
{
	if ( argc == 1 )
	{
		cout << "Specify file name of the image!\n";

		return -1;
	}
    
    cout << endl << argc << ", " << argv[ 1 ] << endl;
    
    // Read the image
    Mat original = imread( argv[ 1 ] );

    // Specify the color threshold
    double h_min = 53.0;
    double h_max = 95.0;
    double s_min = 65.0;
    double s_max = 255.0;
    double v_min = 0.0;
    double v_max = 255.0;

    // Circle detection parameters
    unsigned int num_points = 5;
    unsigned int n = 100;

    // Blob detector
    blob_detection::BlobDetector blob_detector( cvSize( original.cols, original.rows ), \
    											cvScalar( h_min, s_min, v_min, 0.0 ), \
    											cvScalar( h_max, s_max, v_max, 0.0 ), \
    											num_points, \
    											n );

    // Blob detection
    size_t x_c, y_c, r;
    while (1)
    {
    	// Detect the ball
    	blob_detector.blob_detection( original, x_c, y_c, r );

    	/*cout << "x_c: " << x_c << \
    			", y_c: " << y_c << \
    			", r: " << r << endl;*/
    }
	return 0;
}
