/*
 * mt_circdet.hpp
 *
 *  Created on: Dec 18, 2011
 *      Author: ferraraf
 */

#ifndef MT_CIRCDET_HPP_
#define MT_CIRCDET_HPP_


// Histograms
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_histogram2d.h>

// Intel threading libs
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

// Own libs
#include <rand_gen.hpp>
#include <circdet.hpp>


using namespace tbb;


namespace mt_circdet
{
class CircleDetector
{
	// Number of points
	unsigned int num_points;

	// Blob contour
	vector< vector< unsigned int > > blob_contour;

	// Histograms for the circle detection
	gsl_histogram * hist_r;
	gsl_histogram2d * hist__x_c;

public:
	// Constructor
	CircleDetector( unsigned int num_points, \
					const vector< vector< unsigned int > > & blob_contour, \
					gsl_histogram * hist_r, \
					gsl_histogram2d * hist__x_c );

	// Multi-threading callback function
	void operator()( const blocked_range< size_t > & r ) const;
};

// Get the center and radius of the circle
void get_circle( gsl_histogram * hist_r, \
				 gsl_histogram2d * hist__x_c, \
				 size_t & x_c, \
				 size_t & y_c, \
				 size_t & r );
}


#endif /* MT_CIRCDET_HPP_ */
