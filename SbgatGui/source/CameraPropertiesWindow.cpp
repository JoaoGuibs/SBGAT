#include "CameraPropertiesWindow.hpp"


using namespace SBGAT_GUI;

CameraPropertiesWindow::CameraPropertiesWindow(Mainwindow * parent) : QDialog(parent,Qt::WindowStaysOnTopHint) {

	this -> parent = parent;
	this -> setWindowTitle("Camera properties");

	QVBoxLayout * settings_layout = new QVBoxLayout(this);
	QGroupBox * focus_prop_group = new QGroupBox(tr("Camera focus"));
	this -> prop_combo_box = new QComboBox (this);

	QLabel * focus_prop_label = new QLabel("Select prop to focus on",this);

	QGridLayout * focus_prop_layout = new QGridLayout(focus_prop_group);
	focus_prop_layout -> addWidget(focus_prop_label, 0, 0, 1, 1);
	focus_prop_layout -> addWidget(prop_combo_box, 0, 1, 1, 1);

	settings_layout -> addWidget(focus_prop_group);

	// Creating the button box
	QDialogButtonBox * button_box = new QDialogButtonBox(QDialogButtonBox::Ok
		| QDialogButtonBox::Cancel);

	connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box, SIGNAL(rejected()), this, SLOT(close()));

	connect(this -> prop_combo_box,SIGNAL(currentIndexChanged(int)),this,SLOT(change_focus()));
	
	this -> init();


	
}

void CameraPropertiesWindow::init(){
	
	auto wrapped_spacecraft_data = this -> parent -> get_wrapped_spacecraft_data();
	auto wrapped_shape_data = this -> parent -> get_wrapped_shape_data();
	
	for (auto it = wrapped_spacecraft_data.begin(); it != wrapped_spacecraft_data.end(); ++it){
		this -> prop_combo_box -> insertItem(this -> prop_combo_box -> count(),QString::fromStdString(it -> first));
		
	}

	for (auto it = wrapped_shape_data.begin(); it != wrapped_shape_data.end(); ++it){
		this -> prop_combo_box -> insertItem(this -> prop_combo_box -> count(),QString::fromStdString(it -> first));
		
	}

}


void CameraPropertiesWindow::prop_removed_slot(){

	auto wrapped_spacecraft_data = this -> parent -> get_wrapped_spacecraft_data();
	auto wrapped_shape_data = this -> parent -> get_wrapped_shape_data();

	for (unsigned int i = 0; i < this -> prop_combo_box -> count(); ++i){
		QString prop_name = this -> prop_combo_box -> itemText(i);
		if (wrapped_spacecraft_data.find(prop_name.toStdString()) == wrapped_spacecraft_data.end()
			&& wrapped_shape_data.find(prop_name.toStdString()) == wrapped_shape_data.end()){
			this -> prop_combo_box -> removeItem(i);
		break;
			// This break is mandatory as the count of prop_combo_box has changed
	}

}


	// If there are no trajectories or spacecraft to use, the slider widget and
	// its spinbox are disabled
if (this -> prop_combo_box -> count() == 0 ){
		// this -> position_group -> setEnabled(false);
}



}







void CameraPropertiesWindow::prop_added_slot(){
	auto wrapped_spacecraft_data = this -> parent -> get_wrapped_spacecraft_data();
	auto wrapped_shape_data = this -> parent -> get_wrapped_shape_data();

	for (auto it = wrapped_spacecraft_data.begin(); it != wrapped_spacecraft_data.end(); ++it){
		if (this -> prop_combo_box  -> findText(QString::fromStdString(it -> first)) == -1){
			this -> prop_combo_box -> insertItem(this -> prop_combo_box -> count(),
				QString::fromStdString(it -> first));
			break;
		}
	}

	for (auto it = wrapped_shape_data.begin(); it != wrapped_shape_data.end(); ++it){
		if (this -> prop_combo_box  -> findText(QString::fromStdString(it -> first)) == -1){
			this -> prop_combo_box -> insertItem(this -> prop_combo_box -> count(),
				QString::fromStdString(it -> first));
			break;
		}
	}


	// If there are trajectories and spacecraft to use, the slider widget and
	// its spinbox are enabled
	if (this -> prop_combo_box -> count() > 0 ){
		// this -> position_group -> setEnabled(true);
	}

}

void CameraPropertiesWindow::change_focus(){


	if (this -> prop_combo_box -> count() > 0 ){

		auto wrapped_spacecraft_data = this -> parent -> get_wrapped_spacecraft_data();
		auto wrapped_shape_data = this -> parent -> get_wrapped_shape_data();

		std::string current_prop_name = this -> prop_combo_box -> currentText().toStdString();

		vtkSmartPointer<vtkActor> prop_to_focus_on;

		if (wrapped_spacecraft_data.find(current_prop_name) != wrapped_spacecraft_data.end()){
			prop_to_focus_on = wrapped_spacecraft_data[current_prop_name] -> get_actor();
		}
		else{
			prop_to_focus_on = wrapped_shape_data[current_prop_name] -> get_actor();
		}



		vtkSmartPointer<vtkCamera> camera = this -> parent -> get_renderer() -> GetActiveCamera();
		camera -> SetFocalPoint(prop_to_focus_on -> GetPosition());

		this -> parent -> get_renderer() -> Modified();
		this -> parent -> qvtkWidget -> GetRenderWindow() -> Render();
	}


}
