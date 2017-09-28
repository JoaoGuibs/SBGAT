

#include "Tests.hpp"

void TestsSBCore::run() {
	TestsSBCore::test_loading_shape();
	TestsSBCore::test_pgm_consistency_cube();
	TestsSBCore::test_pgm_consistency_ellipsoid();

}




/**
This check ensures that the loaded shape
model has the proper number of facets, vertices and edges
*/
void TestsSBCore::test_loading_shape() {

	std::cout << "- Running test_loading_shape ..." << std::endl;

	SBGAT_CORE::ShapeModel shape_model("", nullptr);
	SBGAT_CORE::ShapeModelImporter shape_io("cube.obj", 1);
	shape_io.load_shape_model(&shape_model);

	assert(shape_model.get_NFacets() == 12);
	assert(shape_model.get_NVertices() == 8);
	assert(shape_model.get_NEdges() == 18);

	std::cout << "-- test_loading_shape successful" << std::endl;

}


/**
This test checks that the Polyhedron Gravity Model
computed around a cubic shape is not depending upon
the cube resolution and consistent with the analytical expression
*/
void TestsSBCore::test_pgm_consistency_cube() {

	std::cout << "- Running test_pgm_consistency_cube ..." << std::endl;


	// The attracting shape is a cube of dimensions 1 x 1 x 1 m of density rhp = 1e6 kg/m^3
	// The analytic acceleration at (1,2,3) (m) in the shape model's barycentric frame is computed
	// Assumes that G = 6.67408e-11 m^3 / (kg * s ^2)
	arma::vec X = {1, 2, 3};
	arma::vec acc_true = {
		-1.273782722739791e-06,
		-2.548008881415967e-06,
		-3.823026510474731e-06
	};


	// Shape models of increasing resolutions but still representative
	// of a cube are loaded. They should all yield the same acceleration
	// at the specified query point

	{
		SBGAT_CORE::ShapeModel shape_model("", nullptr);
		SBGAT_CORE::ShapeModelImporter shape_io("cube.obj", 1);
		shape_io.load_shape_model(&shape_model);




		SBGAT_CORE::DynamicAnalyses dyn_an(&shape_model);
		arma::vec acc = dyn_an.pgm_acceleration(X.colptr(0), 1e6);

		assert(arma::norm(acc_true - acc) / arma::norm(acc) < 1e-12);
	}

	{
		SBGAT_CORE::ShapeModel shape_model("", nullptr);
		SBGAT_CORE::ShapeModelImporter shape_io("cube_50k.obj", 1);
		shape_io.load_shape_model(&shape_model);




		SBGAT_CORE::DynamicAnalyses dyn_an(&shape_model);
		arma::vec acc = dyn_an.pgm_acceleration(X.colptr(0), 1e6);

		assert(arma::norm(acc_true - acc) / arma::norm(acc) < 1e-12);
	}

	{
		SBGAT_CORE::ShapeModel shape_model("", nullptr);
		SBGAT_CORE::ShapeModelImporter shape_io("cube_200k.obj", 1);
		shape_io.load_shape_model(&shape_model);


		SBGAT_CORE::DynamicAnalyses dyn_an(&shape_model);
		arma::vec acc = dyn_an.pgm_acceleration(X.colptr(0), 1e6);

		assert(arma::norm(acc_true - acc) / arma::norm(acc) < 1e-12);
	}

	std::cout << "-- test_pgm_consistency_cube successful" << std::endl;


}



/**
This test checks ensures that the Polyhedron Gravity Model
computed around an ellispoid shape is consistent
with the analytical expression
*/
void TestsSBCore::test_pgm_consistency_ellipsoid() {

	std::cout << "- Running test_pgm_consistency_ellipsoid ..." << std::endl;

	// The attracting shape is an ellispoid of semi-major axes 3 x 2 x 1 m of density rho = 1e6 kg/m^3
	// The analytic acceleration at (1,2,3) (m) in the shape model's barycentric frame is computed
	// Assumes that G = 6.67408e-11 m^3 / (kg * s ^2)
	arma::vec X = {1, 2, 3};
	arma::vec acc_true = {
		-2.19160852e-05,  
		-5.18364044e-05,  
		-8.79434337e-05
	};

	SBGAT_CORE::ShapeModel shape_model("", nullptr);
	SBGAT_CORE::ShapeModelImporter shape_io("ellipsoid.obj", 1);
	shape_io.load_shape_model(&shape_model);

	SBGAT_CORE::DynamicAnalyses dyn_an(&shape_model);
	arma::vec acc = dyn_an.pgm_acceleration(X.colptr(0), 1e6);

	assert(arma::norm(acc_true - acc) / arma::norm(acc) < 5e-5);

	std::cout << "-- test_pgm_consistency_ellipsoid successful" << std::endl;





}


