#include <SBGATPolyhedronGravityModel.hpp>
#include <SBGATPolyhedronGravityModelUQ.hpp>
#include <SBGATObjWriter.hpp>

#include <vtkCleanPolyData.h>
#include <vtkOBJReader.h>

#include <json.hpp>
#include <boost/progress.hpp>

#include <armadillo>

int main(){

	std::ifstream i("input_file.json");
	nlohmann::json input_data;
	i >> input_data;

	std::string PATH_SHAPE = input_data["PATH_SHAPE"];
	double CORRELATION_DISTANCE =  input_data["CORRELATION_DISTANCE"];

	double ERROR_STANDARD_DEV  = input_data["ERROR_STANDARD_DEV"];
	double DENSITY  = input_data["DENSITY"];
	double PERIOD_SD  = input_data["PERIOD_SD"];
	double PERIOD  = input_data["PERIOD"];



	bool UNIT_IN_METERS  = input_data["UNIT_IN_METERS"];

	int N_MONTE_CARLO = input_data["N_MONTE_CARLO"];

	std::string OUTPUT_DIR = input_data["OUTPUT_DIR"];

	std::cout << "- Path to shape: " << PATH_SHAPE << std::endl;
	std::cout << "- Standard deviation on point coordinates (m) : " << ERROR_STANDARD_DEV << std::endl;
	std::cout << "- Correlation distance (m) : " << CORRELATION_DISTANCE << std::endl;
	std::cout << "- Standard deviation on rotation period (s) : " << PERIOD_SD << std::endl;
	std::cout << "- Density (kg/m^3) : " << DENSITY << std::endl;
	std::cout << "- Rotation period (s) : " << PERIOD << std::endl;
	std::cout << "- Monte Carlo Draws : " << N_MONTE_CARLO << std::endl;


	// Reading
	vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
	reader -> SetFileName(PATH_SHAPE.c_str());
	reader -> Update(); 

	// An instance of SBGATPolyhedronGravityModel is created to evaluate the PGM of 
	// the considered polytdata
	vtkSmartPointer<SBGATPolyhedronGravityModel> pgm_filter = vtkSmartPointer<SBGATPolyhedronGravityModel>::New();
	pgm_filter -> SetInputConnection(reader -> GetOutputPort());
	pgm_filter -> SetDensity(DENSITY);

	
	if(UNIT_IN_METERS){
		pgm_filter -> SetScaleMeters();
	} else{
		pgm_filter -> SetScaleKiloMeters();
	}

	std::cout << "Building pgm ...\n";
	pgm_filter -> SetOmega(2 * arma::datum::pi / PERIOD * arma::vec({0,0,1}));
	pgm_filter -> Update();

	// An instance of SBGATPolyhedronGravityModelUQ is created to perform
	// uncertainty quantification from the PGM associated to the shape
	SBGATPolyhedronGravityModelUQ pgm_uq;
	pgm_uq.SetModel(pgm_filter);
	pgm_uq.SetPeriodErrorStandardDeviation(PERIOD_SD);

	// Saving baseline slices
	pgm_uq.TakeAndSaveSlice(0,OUTPUT_DIR + "baseline_slice_x.txt",0);
	pgm_uq.TakeAndSaveSlice(1,OUTPUT_DIR + "baseline_slice_y.txt",0);
	pgm_uq.TakeAndSaveSlice(2,OUTPUT_DIR + "baseline_slice_z.txt",0);


	std::cout << "Populating shape covariance ...\n";

	// Populate the shape vertices covariance
	pgm_uq.AddUncertaintyRegionToCovariance(0,ERROR_STANDARD_DEV,CORRELATION_DISTANCE);
	pgm_uq.AddUncertaintyRegionToCovariance(1147,ERROR_STANDARD_DEV,CORRELATION_DISTANCE);

	arma::mat C_CC = pgm_uq.GetCovarianceSquareRoot();
	arma::mat P_CC = pgm_uq.GetVerticesCovariance();

	std::cout << "Maximum absolute error in covariance square root: " << arma::abs(P_CC - C_CC * C_CC.t()).max() << std::endl;


	std::cout << "Saving non-zero partition of shape covariance ...\n";

	// Save the covariance
	pgm_uq.SaveNonZeroVerticesCovariance(OUTPUT_DIR + "shape_covariance.json");

	
	std::vector<int > all_facets = {
		0,10,100,1000,200,300
	};


	// Analytical UQ

	std::vector<double> analytical_variances_pot(all_facets.size());

	std::cout << "Computing analytical uncertainties ... ";
	auto start = std::chrono::system_clock::now();
	#pragma omp parallel for
	for (int e = 0; e < all_facets.size(); ++e){
		analytical_variances_slopes[e] = pgm_uq.GetVarianceSlope(all_facets[e]);
	}
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end-start;
	std::cout << "Done computing analytical uncertainties in " << elapsed_seconds.count() << " s\n";

	// Running a Monte Carlo to compare against
	std::vector<arma::vec> deviations;
	std::vector<double> period_errors;

	std::vector < std::vector<double> > all_slopes;


	std::cout << "Running MC ... ";

	
	start = std::chrono::system_clock::now();
	SBGATPolyhedronGravityModelUQ::RunMCUQSlopes(PATH_SHAPE,
		DENSITY,
		Omega,
		UNIT_IN_METERS,
		C_CC,
		PERIOD_SD,
		N_MONTE_CARLO, 
		all_facets,
		OUTPUT_DIR,
		std::min(30,N_MONTE_CARLO),
		deviations,
		period_errors,
		all_slopes);


	

	end = std::chrono::system_clock::now();

	elapsed_seconds = end-start;

	std::cout << "Done running MC in " << elapsed_seconds.count() << " s\n";

	
	std::vector<double> mc_variances_slopes(all_facets.size());

	std::cout << "Computing MC dispersions...\n";
	
	#pragma omp parallel for
	for (int e = 0; e < mc_variances_slopes.size(); ++e){

		arma::vec slopes_mc(N_MONTE_CARLO);

		for (int sample = 0; sample < N_MONTE_CARLO; ++sample){
			slopes_mc(sample) = all_slopes[sample][e];
		}

		mc_variances_slopes[e] = arma::var(slopes_mc);

	}

	std::cout << "\t After " << N_MONTE_CARLO << " MC outcomes:\n";

	for (int e = 0; e < all_facets.size(); ++e){
		all_positions[e].t().print("\t At: ");
		std::cout << "\t\tMC variance in slope: " << mc_variances_slopes[e] << std::endl;
		std::cout << "\t\tAnalytical variance in slope: " << analytical_variances_slopes[e] << std::endl;

		std::cout << "\t\tError (%): " << (mc_variances_slopes[e] - analytical_variances_slopes[e])/analytical_variances_slopes[e] * 100 << std::endl;

	}


	return 0;
}