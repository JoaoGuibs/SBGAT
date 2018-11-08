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


	QVBoxLayout * window_layout = new QVBoxLayout(this);
	QWidget * select_shape_widget = new QWidget(this);
	QHBoxLayout * select_shape_layout = new QHBoxLayout(select_shape_widget);

	this -> compute_surface_pgm_button = new QPushButton("Compute Surface PGM",this);
	this -> primary_prop_combo_box = new QComboBox (this);
	this -> primary_shape_properties_widget = new ShapePropertiesWidget(this ,"Shape properties");

	// Creating the button box
	this -> button_box = new QDialogButtonBox(QDialogButtonBox::Ok);


	select_shape_layout -> addWidget(new QLabel("Shape model",this));
	select_shape_layout -> addWidget(this -> primary_prop_combo_box);

	window_layout -> addWidget(select_shape_widget);
	window_layout -> addWidget(this -> primary_shape_properties_widget);
	window_layout -> addWidget(this -> compute_surface_pgm_button);
	window_layout -> addWidget(this -> button_box);



	this -> init();
	connect(this -> button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(this -> compute_surface_pgm_button, SIGNAL(clicked()), this, SLOT(compute_surface_pgm()));


	window_layout -> addStretch(1);


}

void SurfacePGMWindow::init(){

	auto wrapped_shape_data = this -> parent -> get_wrapped_shape_data();	
	
	if (wrapped_shape_data.size() == 0 ){
		this -> compute_surface_pgm_button -> setEnabled(false);
	}
	else{
		this -> compute_surface_pgm_button -> setEnabled(true);

		for (auto it = wrapped_shape_data.begin(); it != wrapped_shape_data.end(); ++it){
			this -> primary_prop_combo_box -> insertItem(this -> primary_prop_combo_box -> count(),QString::fromStdString(it -> first));
		}
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


	omega *= 2 * arma::datum::pi / this -> primary_shape_properties_widget -> get_period();

	std::vector<double> slopes,potentials,acc_magnitudes,acc_body_fixed_magnitudes;

	int numCells = selected_shape -> GetNumberOfCells();
	std::vector<unsigned int> queried_elements;
	
	for (unsigned int i = 0; i < static_cast<unsigned int>(numCells); ++i){
		queried_elements.push_back(i);
	}


    SBGATPolyhedronGravityModel::ComputeSurfacePGM(selected_shape,
	queried_elements,
	true,
	this -> primary_shape_properties_widget -> get_density(),
	omega,
	slopes,
	potentials,
	acc_magnitudes,
	acc_body_fixed_magnitudes);

	std::shared_ptr<ModelDataWrapper> wrapper = this -> parent -> get_wrapped_shape_data()[selected_shape_name];

	wrapper -> set_potentials(potentials);
	wrapper -> set_acc_magnitudes(acc_magnitudes);
	wrapper -> set_slopes(slopes);
	wrapper -> set_acc_body_fixed_magnitudes(acc_body_fixed_magnitudes);
	wrapper -> get_mapper() -> ScalarVisibilityOff();

	this -> parent -> get_renderer() -> RemoveActor2D(wrapper -> get_colorbar_actor());

	this -> parent -> qvtkWidget -> GetRenderWindow() -> Render();

}


