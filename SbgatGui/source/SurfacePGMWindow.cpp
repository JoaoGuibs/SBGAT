/** MIT License

Copyright (c) 2018 Benjamin Bercovici and Jay McMahon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <QScrollArea>

#include "ShapePropertiesWidget.hpp"
#include "SurfacePGMWindow.hpp"

#include <SBGATMassProperties.hpp>
#include <SBGATTrajectory.hpp>
#include <OrbitConversions.hpp>
#include <SBGATPolyhedronGravityModel.hpp>

using namespace SBGAT_GUI;

SurfacePGMWindow::SurfacePGMWindow(Mainwindow * parent) {

	this -> parent = parent;
	this -> setWindowTitle("Compute/Load Surface PGM");

	QVBoxLayout * window_layout = new QVBoxLayout(this);
	QWidget * select_shape_widget = new QWidget(this);
	QWidget * button_widget = new QWidget(this);

	QHBoxLayout * select_shape_layout = new QHBoxLayout(select_shape_widget);
	QHBoxLayout * button_widget_layout = new QHBoxLayout(button_widget);


	this -> compute_surface_pgm_button = new QPushButton("Compute Surface PGM",this);
	this -> load_surface_pgm_button = new QPushButton("Load Surface PGM",this);

	this -> primary_prop_combo_box = new QComboBox (this);
	this -> primary_shape_properties_widget = new ShapePropertiesWidget(this ,"Shape properties");


	// Creating the button box
	this -> button_box = new QDialogButtonBox(QDialogButtonBox::Ok);

	// Creating the output folder 
	this ->  open_output_file_dialog_button = new QPushButton("Select output file",this);


	select_shape_layout -> addWidget(new QLabel("Shape model",this));
	select_shape_layout -> addWidget(this -> primary_prop_combo_box);

	window_layout -> addWidget(select_shape_widget);

	window_layout -> addWidget(this -> primary_shape_properties_widget);
	
	button_widget_layout -> addWidget(this -> compute_surface_pgm_button);
	button_widget_layout -> addWidget(this -> load_surface_pgm_button);

	window_layout -> addWidget(this -> open_output_file_dialog_button);
	window_layout -> addWidget(button_widget);

	window_layout -> addWidget(this -> button_box);



	this -> compute_surface_pgm_button -> setEnabled(false);

	this -> init();
	connect(this -> button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(this -> compute_surface_pgm_button, SIGNAL(clicked()), this, SLOT(compute_surface_pgm()));
	connect(this -> load_surface_pgm_button, SIGNAL(clicked()), this, SLOT(load_surface_pgm()));
	connect(this -> open_output_file_dialog_button,SIGNAL(clicked()),this,
		SLOT(open_output_file_dialog()));


	window_layout -> addStretch(1);


}

void SurfacePGMWindow::init(){

	auto wrapped_shape_data = this -> parent -> get_wrapped_shape_data();	
	
	
	if (wrapped_shape_data.size() != 0 ){
		for (auto it = wrapped_shape_data.begin(); it != wrapped_shape_data.end(); ++it){
			this -> primary_prop_combo_box -> insertItem(this -> primary_prop_combo_box -> count(),QString::fromStdString(it -> first));
		}
	}

	if(wrapped_shape_data.size() == 0){
		this -> load_surface_pgm_button -> setEnabled(false);
	}

}

void SurfacePGMWindow::compute_surface_pgm(){



	std::string selected_shape_name = this -> primary_prop_combo_box -> currentText().toStdString();
	vtkSmartPointer<vtkPolyData> selected_shape = this -> parent -> get_wrapped_shape_data()[selected_shape_name]-> get_polydata();


	arma::vec::fixed<3> omega = this -> primary_shape_properties_widget -> get_spin();

	if (this -> primary_shape_properties_widget -> get_period() == 0){
		QMessageBox::warning(this, "Evaluate Surface PGM", "The rotation period must be strictly greater than 0!");
		return;
	}

	std::string opening_line = "### Computing surface PGM of " + selected_shape_name  + " ###";
	this -> parent ->log_console -> appendPlainText(QString::fromStdString(opening_line));

	omega *= 2 * arma::datum::pi / this -> primary_shape_properties_widget -> get_period();

	std::vector<double> slopes,
	inertial_potentials,
	body_fixed_potentials,
	inertial_acc_magnitudes,
	body_fixed_acc_magnitudes;

	int numCells = selected_shape -> GetNumberOfCells();
	std::vector<unsigned int> queried_elements;
	
	for (unsigned int i = 0; i < static_cast<unsigned int>(numCells); ++i){
		queried_elements.push_back(i);
	}

	auto start = std::chrono::system_clock::now();


	SBGATPolyhedronGravityModel::ComputeSurfacePGM(selected_shape,
		queried_elements,
		true,
		this -> primary_shape_properties_widget -> get_density(),
		omega,
		slopes,
		inertial_potentials,
		body_fixed_potentials,
		inertial_acc_magnitudes,
		body_fixed_acc_magnitudes);

	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end-start;
	


	vtkSmartPointer<SBGATMassProperties> mass_properties = vtkSmartPointer<SBGATMassProperties>::New();

	mass_properties -> SetInputData(selected_shape);
	mass_properties -> Update();
	double mass = mass_properties -> GetVolume() * this -> primary_shape_properties_widget -> get_density();

	SBGATPolyhedronGravityModel::SaveSurfacePGM(selected_shape,
		queried_elements,
		true,
		mass,
		omega,
		slopes,
		inertial_potentials,
		body_fixed_potentials,
		inertial_acc_magnitudes,
		body_fixed_acc_magnitudes,
		this -> output_path );



	std::shared_ptr<ModelDataWrapper> wrapper = this -> parent -> get_wrapped_shape_data()[selected_shape_name];

	wrapper -> set_inertial_potentials(inertial_potentials);
	wrapper -> set_body_fixed_potentials(body_fixed_potentials);

	wrapper -> set_inertial_acc_magnitudes(inertial_acc_magnitudes);
	wrapper -> set_body_fixed_acc_magnitudes(body_fixed_acc_magnitudes);
	wrapper -> set_slopes(slopes);
	wrapper -> get_mapper() -> ScalarVisibilityOff();

	this -> parent -> get_renderer() -> RemoveActor2D(wrapper -> get_colorbar_actor());

	this -> parent -> qvtkWidget -> GetRenderWindow() -> Render();

	std::string displayed_line = "- Saved surface-evaluated PGM of " + selected_shape_name + " to " + this -> output_path;

	this -> parent -> log_console -> appendPlainText(QString::fromStdString("- Done computing surface PGM in " + std::to_string(elapsed_seconds.count()) +  " seconds."));
	this -> parent -> log_console -> appendPlainText(QString::fromStdString(displayed_line));

	std::string closing_line(opening_line.length() - 1, '#');

	closing_line.append("\n");

	this -> parent -> log_console -> appendPlainText(QString::fromStdString(closing_line));








}


void SurfacePGMWindow::open_output_file_dialog(){

	std::string default_name;

	std::string name = this -> primary_prop_combo_box -> currentText().toStdString();
	auto shape_data = this -> parent -> get_wrapped_shape_data();
	
	if ( shape_data.find(name)!= shape_data.end()){
		default_name = "./" + name + "_surface_pgm.json";
	}

	QString path = QFileDialog::getSaveFileName(this, tr("Save Surface PGM To File"),
		QString::fromStdString(default_name),
		tr("JSON file (*.json)"));

	this -> output_path = path.toStdString();

	if (this -> output_path.size() > 0){
		this -> compute_surface_pgm_button -> setEnabled(true);

	}
}

void SurfacePGMWindow::load_surface_pgm(){
	std::string selected_shape_name = this -> primary_prop_combo_box -> currentText().toStdString();

	QString path = QFileDialog::getOpenFileName(this, tr("Open Surface PGM File"),
		"~",
		tr("JSON (*.json)"));
	
	if (path.size() == 0){
		return;
	}

	std::vector<double> slopes,inertial_potentials,
	body_fixed_potentials,
	inertial_acc_magnitudes,
	body_fixed_acc_magnitudes;
	double mass;
	arma::vec::fixed<3> omega;

	try{
		std::shared_ptr<ModelDataWrapper> wrapper = this -> parent -> get_wrapped_shape_data()[selected_shape_name];

		try{
			SBGATPolyhedronGravityModel::LoadSurfacePGM(mass,
				omega,
				slopes,
				inertial_potentials,
				body_fixed_potentials,
				inertial_acc_magnitudes,
				body_fixed_acc_magnitudes,
				path.toStdString());
		}

		catch(std::runtime_error & e){
			QMessageBox::warning(this, "Load Surface PGM", e.what());
			return;
		}

		if (slopes.size() != wrapper -> get_polydata() -> GetNumberOfCells()){
			QMessageBox::warning(this, 
				"Load Surface PGM", QString::fromStdString("Error: the loaded surface PGM (" + std::to_string(slopes.size())+ 
					" facets) does not match the selected shape resolution (" + std::to_string(wrapper -> get_polydata() -> GetNumberOfCells()) + " facets)"));
			return;
		}

		wrapper -> set_slopes(slopes);

		wrapper -> set_inertial_potentials(inertial_potentials);
		wrapper -> set_body_fixed_potentials(body_fixed_potentials);

		wrapper -> set_inertial_acc_magnitudes(inertial_acc_magnitudes);
		wrapper -> set_body_fixed_acc_magnitudes(body_fixed_acc_magnitudes);
		
		wrapper -> get_mapper() -> ScalarVisibilityOff();

		this -> parent -> get_renderer() -> RemoveActor2D(wrapper -> get_colorbar_actor());

		this -> parent -> qvtkWidget -> GetRenderWindow() -> Render();
		this -> parent -> log_console -> appendPlainText(QString::fromStdString("\n- Done load surface PGM from file " + path.toStdString()));

	}
	catch(std::runtime_error & e ){
		QMessageBox::warning(this, "Load Surface PGM", "An error occured loading the Surface PGM file.");
		return;
	}

}


