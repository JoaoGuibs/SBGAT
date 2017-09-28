/** 
@file   main.cpp
@Author Benjamin Bercovici (bebe0705@colorado.edu)
@date   September, 2017
@brief  Example demonstrating the computation of gravity spherical harmonics
assuming a constant density from an input polyhedral shape model
*/

#include <iostream>
#include <armadillo>
#include <chrono>

#include <ShapeModelImporter.hpp>
#include <ShapeModel.hpp>
#include <DynamicAnalyses.hpp>

int main( int argc, char** argv ) {


	SBGAT_CORE::FrameGraph frame_graph;
	SBGAT_CORE::ShapeModel shape_model("B", &frame_graph);
	SBGAT_CORE::ShapeModelImporter shape_io("../eros_64.obj", 1);

	shape_io.load_shape_model(&shape_model);
	SBGAT_CORE::DynamicAnalyses dynamic_analyses(&shape_model);

	// Harmonics up to degree five are computed
	int degree = 5;

	// Density of Eros (kg/km^3)
	double density = 2670000000000.0;

	// Reference radius of Eros (km)
	double ref_radius = 16;

	// Flag set to true for normalized coefficients, false for unnormalized ones
	bool normalized = true;

	arma::mat Cnm_total;
	arma::mat Snm_total;

	dynamic_analyses.compute_exterior_sh_coefs(
		Cnm_total,
		Snm_total,
		degree,
		ref_radius,
		density,
		normalized);


	// The coefficients are stored in a more convenient form 
	arma::mat coefs = arma::zeros<arma::mat>((degree + 1) * (degree + 2)/2 - 1,4);
	
	for (int n = 1 ; n < degree + 1 ; ++n){
		for (int m = 0 ; m <= n; ++m){

			int index = (n) * (n + 1 )/2 + m - 1;

			coefs.row(index)(0) = n;
			coefs.row(index)(1) = m;
			coefs.row(index)(2) = Cnm_total.row(n)(m);
			coefs.row(index)(3) = Snm_total.row(n)(m);

		}
	}
	// coefs now holds the normalized spherical harmonics coefficients
	// the matrix's 4 columns are labelled as follows:
	// degree n -- order m -- Cnm -- Snm

	std::cout << coefs << std::endl;

	// The coefficient table is saved to a file
	coefs.save("eros_spherical_coords_normalized.txt",arma::raw_ascii);

	return 0;
}