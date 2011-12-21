/*
 * mt_circdet.cpp
 *
 *  Created on: Dec 18, 2011
 *      Author: ferraraf
 */


#include "mt_circdet.hpp"


namespace mt_circdet
{
// Constructor
CircleDetector::CircleDetector( unsigned int num_points, \
								const vector< vector< unsigned int > > & blob_contour, \
								gsl_histogram * hist_r, \
								gsl_histogram2d * hist__x_c )
{
	this->num_points = num_points;
	this->blob_contour = blob_contour;
	this->hist_r = hist_r;
	this->hist__x_c = hist__x_c;
}

// Multi-threading callback function
void CircleDetector::operator()( const blocked_range< size_t > & r ) const
{
	// Fill the histograms
	for ( size_t i = r.begin(); i != r.end(); i++ )
	{
		// Matrix for the randomly selected blob points
		MatrixXd x;
		x.resize( num_points, 2 );

		// Vector for the random indeces
		vector< double > rand_num = multi_uniGen( num_points, 0, ( double )blob_contour.size() - 1.0 );

		// Get the randomly selected blob points
		for ( int j = 0; j < ( int )num_points; j++ )
		{
			x( j, 0 ) = ( double )blob_contour[ ( int )round( rand_num[ j ] ) ][ 0 ];
			x( j, 1 ) = ( double )blob_contour[ ( int )round( rand_num[ j ] ) ][ 1 ];
		}

		// Fit a circle
		Vector2d x_c;
		double r;
		circdet::detect_circle( x, x_c, r );

		// Increment histograms
		gsl_histogram_increment( hist_r, round( r ) );
		gsl_histogram2d_increment( hist__x_c, round( x_c( 0 ) ), round( x_c( 1 ) ) );
	}
}

// Get the center and radius of the circle
void get_circle( gsl_histogram * hist_r, \
				 gsl_histogram2d * hist__x_c, \
				 size_t & x_c, \
				 size_t & y_c, \
				 size_t & r )
{
	// Get the maximum histogram values
	r = gsl_histogram_max_bin( hist_r );
	gsl_histogram2d_max_bin( hist__x_c, &x_c, &y_c );

	// Reset the histograms
	gsl_histogram_reset( hist_r );
	gsl_histogram2d_reset( hist__x_c );
}
}
