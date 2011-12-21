/*
 * main.cpp
 *
 *  Created on: Dec 12, 2011
 *      Author: ferraraf
 */


#include <ctime>
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
    /*blob_detection::BlobDetector blob_detector( Size( original.cols, original.rows ), \
    											Scalar( h_min, s_min, v_min, 0.0 ), \
    											Scalar( h_max, s_max, v_max, 0.0 ), \
    											num_points, \
    											n );*/
    blob_detection::BlobDetector * blob_detector = NULL;

    blob_detector = new blob_detection::BlobDetector( Size( original.cols, original.rows ), \
    										   	   	  Scalar( h_min, s_min, v_min, 0.0 ), \
    										   	   	  Scalar( h_max, s_max, v_max, 0.0 ), \
    										   	   	  num_points, \
    										   	   	  n );

    // Runtime variables
    clock_t start, end;
    double T;
    vector< double > t_vec;
    t_vec.resize( 20 );
    int i = 0;
    double t_sum = 0.0;

    // Blob detection
    size_t x_c, y_c, r;
    while (1)
    {
    	// Get the ticks
    	start = clock();

    	// Detect the ball
    	//blob_detector.blob_detection( original, x_c, y_c, r );
    	bool det = blob_detector->blob_detection( original, x_c, y_c, r );

    	// Get the ticks
    	end = clock();

    	/*cout << "x_c: " << x_c << \
    			", y_c: " << y_c << \
    			", r: " << r << endl;*/

    	// Calculate the time
    	T = ( double )( end - start ) / ( double )CLOCKS_PER_SEC;

    	// Put it into the time vector
    	t_vec[ i ] = T;

    	if ( i == 19 )
    	{
    		for ( vector< double >::iterator it = t_vec.begin(); it != t_vec.end(); it++ )
    		{
    			t_sum = t_sum + *it;
    		}

    		cout << det << ", Runtime: " << t_sum / 20.0 << endl;

    		t_sum = 0.0;
    		i = 0;
    	}
    	else
    	{
    		i = i + 1;
    	}
    }
	return 0;
}
