#include "Mainwindow.hpp"
using namespace SBGAT_GUI;

// shortcut to interactor modes
#define INTERACTOR_IS_ORIENT 0
#define INTERACTOR_IS_SELECT 1


Mainwindow::Mainwindow() {

    // The GUI elements are created
    this -> setupUi();

    // The elements interfacing with SbgatCore are created
    this -> frame_graph = std::make_shared<SBGAT_CORE::FrameGraph>();
    this -> frame_graph -> add_frame("inertial_default"); // a default inertial frame of reference is created

}

void Mainwindow::setupUi() {
    this -> resize(1024, 768);

    // The widget are created
    this -> lateral_dockwidget = new QDockWidget(this);
    this -> qvtkWidget = new QVTKOpenGLWidget(this);
    this -> status_bar = new QStatusBar(this);
    this -> log_console = new QPlainTextEdit(this);
    this -> log_console -> setReadOnly(true);
    this -> prop_table = new QTableWidget(0, 3, this);


    // The status bar is populated
    this -> setStatusBar(this -> status_bar);
    this -> statusBar() -> showMessage("Ready");

    // Headers are added to the shape table
    QStringList header_lists = {"Name", "Show", "Erase"};
    this -> prop_table -> setHorizontalHeaderLabels(header_lists);
    this -> prop_table -> horizontalHeader()->setStretchLastSection(true);

    // Selecting an item in the table highlights the entire row
    this -> prop_table -> setSelectionBehavior(QAbstractItemView::SelectRows);
    this -> prop_table -> setSelectionMode(QAbstractItemView::SingleSelection);

    // The lateral dockwidget is filled up
    this -> lateral_dockwidget -> setFeatures( QDockWidget::DockWidgetMovable );

    QWidget * lateral_dockwidget_container = new QWidget(this);
    QVBoxLayout * lateral_dockwidget_container_layout = new QVBoxLayout();

    lateral_dockwidget_container -> setLayout(lateral_dockwidget_container_layout);
    lateral_dockwidget_container_layout -> addWidget( this -> prop_table );
    lateral_dockwidget_container_layout -> addWidget( this -> log_console );


    this -> lateral_dockwidget -> setWidget(lateral_dockwidget_container);
    this -> addDockWidget(Qt::RightDockWidgetArea, this -> lateral_dockwidget);
    this -> lateral_dockwidget -> hide();

    // Central window
    this -> setCentralWidget(qvtkWidget);
    this -> setWindowTitle(QStringLiteral("SBGAT (WIP)"));

    // Actions and menus are created
    this -> createActions();
    this -> createMenus();

    // A VTK renderer is created and linked with the qvtk widget
    this -> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> render_window = vtkSmartPointer< vtkGenericOpenGLRenderWindow>::New();
    this -> qvtkWidget -> SetRenderWindow(render_window);
    this -> qvtkWidget -> GetRenderWindow() -> AddRenderer(this -> renderer);


    this -> renderer -> SetGradientBackground (true);
    this -> renderer -> SetBackground (0.5, 0.5, 1);

    vtkSmartPointer<vtkAreaPicker> areaPicker = vtkSmartPointer<vtkAreaPicker>::New();

    vtkSmartPointer<vtkInteractorStyleSwitch> style =
    vtkSmartPointer<vtkInteractorStyleSwitch>::New();

    render_window -> GetInteractor() -> SetInteractorStyle( style );
    render_window -> GetInteractor() -> SetPicker(areaPicker);


    vtkSmartPointer<vtkAxesActor> axes =
    vtkSmartPointer<vtkAxesActor>::New();

    this -> orientation_widget =
    vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    orientation_widget -> SetOrientationMarker( axes );
    orientation_widget -> SetInteractor(render_window -> GetInteractor() );
    orientation_widget -> SetViewport( 0.0, 0.0, 0.2, 0.2 );
    orientation_widget -> SetEnabled( 1 );
    orientation_widget -> InteractiveOff();


    // A slot is connected to the signal sent by the table when a new selection is made
    connect(this -> prop_table, SIGNAL(currentItemChanged(QTableWidgetItem * , QTableWidgetItem * )),
        this, SLOT(update_GUI_changed_prop()));

    this -> qvtkWidget -> update();

    this -> show();

    this -> qvtkWidget -> GetRenderWindow() -> Render();


}


void Mainwindow::set_action_status(bool enabled, QAction * action) {
    action -> setEnabled(enabled);
}

void Mainwindow::update_GUI_changed_prop() {

    // The status bar is updated depending on whether a shape model remains
    if (this -> wrapped_shape_data.size() == 0 && this -> wrapped_trajectory_data.size() == 0) {
        this -> statusBar() -> showMessage("Ready");
    }

    else {
        int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
        std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();
        
        if (this -> wrapped_shape_data.find(name) != this -> wrapped_shape_data.end()){
            auto active_shape_model  =  this -> wrapped_shape_data[name] -> get_shape_model();


            std::string message("Facets : " + std::to_string(active_shape_model -> get_NFacets()) + " Vertices: " + std::to_string(active_shape_model -> get_NVertices())
                + " Edges: " + std::to_string(active_shape_model -> get_NEdges()));
            this -> statusBar() -> showMessage(QString::fromStdString(message));
        }
        else if (this -> wrapped_trajectory_data.find(name) != this -> wrapped_trajectory_data.end()){

            auto points =  this -> wrapped_trajectory_data[name] -> get_points();
            std::string message("Trajectory points : " + std::to_string(points -> GetNumberOfPoints()) );

            this -> statusBar() -> showMessage(QString::fromStdString(message));
        }

    }

    this -> update_actions_availability();

}



void Mainwindow::open_settings_window() {

    SettingsWindow settings_window(this);
    settings_window.exec();

}

void Mainwindow::createActions() {


    this -> load_shape_model_action = new QAction(tr("Load shape model"), this);
    this -> load_shape_model_action -> setStatusTip(tr("Load obj file holding the facet/vertex description of a shape of interest"));
    connect(this -> load_shape_model_action, &QAction::triggered, this, &Mainwindow::load_shape_model);

    this -> load_trajectory_action = new QAction(tr("Load trajectory"), this);
    this -> load_trajectory_action -> setStatusTip(tr("Load a text file storing the x/y/z components a body-fixed trajectory "));
    connect(this -> load_trajectory_action, &QAction::triggered, this, &Mainwindow::load_trajectory);


    this -> open_settings_window_action = new QAction(tr("Settings"), this);
    this -> open_settings_window_action -> setStatusTip(tr("Open settings window where SbgatGUI settings can be set"));
    connect(this -> open_settings_window_action, &QAction::triggered, this, &Mainwindow::open_settings_window);

    this -> show_lateral_dockwidget_action = new QAction(tr("Show lateral widget"), this);
    this -> show_lateral_dockwidget_action -> setStatusTip(tr("Shows/hides lateral widget holding shape model information"));
    connect(this -> show_lateral_dockwidget_action, &QAction::triggered, this, &Mainwindow::show_lateral_dockwidget);


    this -> clear_console_action = new QAction(tr("Clear log console"), this);
    this -> clear_console_action -> setStatusTip(tr("Clears the log console"));
    connect(this -> clear_console_action, &QAction::triggered, this, &Mainwindow::clear_console);


    this -> save_console_action = new QAction(tr("Save log console to file"), this);
    this -> save_console_action -> setStatusTip(tr("Saves log console to a file"));
    connect(this -> save_console_action, &QAction::triggered, this, &Mainwindow::save_console);



    this -> compute_geometry_measures_action = new QAction(tr("Compute geometry measures"), this);
    this -> compute_geometry_measures_action -> setStatusTip(tr("Compute geometry measures of the selected prop to the console"));
    connect(this -> compute_geometry_measures_action, &QAction::triggered, this, &Mainwindow::compute_geometry_measures);


    this -> compute_pgm_acceleration_action = new QAction(tr("Compute PGM acceleration"), this);
    this -> compute_pgm_acceleration_action -> setStatusTip(tr("Compute PGM acceleration at a point whose coordinates are expressed in the shape's body frame"));
    connect(this -> compute_pgm_acceleration_action, &QAction::triggered, this, &Mainwindow::compute_pgm_acceleration);


    this -> compute_global_pgm_acceleration_action = new QAction(tr("Compute global PGM accelerations"), this);
    this -> compute_global_pgm_acceleration_action -> setStatusTip(tr("Compute PGM accelerations over the entire shape model"));
    connect(this -> compute_global_pgm_acceleration_action, &QAction::triggered, this, &Mainwindow::compute_global_pgm_acceleration);


    this -> compute_global_pgm_potential_action = new QAction(tr("Compute global PGM potentials"), this);
    this -> compute_global_pgm_potential_action -> setStatusTip(tr("Compute PGM potentials over the entire shape model"));
    connect(this -> compute_global_pgm_potential_action, &QAction::triggered, this, &Mainwindow::compute_global_pgm_potential);




    this -> compute_grav_slopes_action = new QAction(tr("Compute gravity slopes"), this);
    this -> compute_grav_slopes_action -> setStatusTip(tr("Compute PGM accelerations over the entire shape model"));
    connect(this -> compute_grav_slopes_action, &QAction::triggered, this, &Mainwindow::compute_gravity_slopes);

    this -> show_grav_slopes_action = new QAction(tr("Show gravity slopes"), this);
    this -> show_grav_slopes_action -> setStatusTip(tr("Display gravity slopes along with colorbar"));
    connect(this -> show_grav_slopes_action, &QAction::triggered, this, &Mainwindow::show_grav_slopes);


    this -> show_global_pgm_pot_action = new QAction(tr("Show gravity potentials"), this);
    this -> show_global_pgm_pot_action -> setStatusTip(tr("Display gravity potentials along with colorbar"));
    connect(this -> show_global_pgm_pot_action, &QAction::triggered, this, &Mainwindow::show_global_pgm_pot);


    this -> update_actions_availability();
}

void Mainwindow::update_actions_availability() {

    // If no shape model is loaded
    if (this -> wrapped_shape_data.size() == 0 && this -> wrapped_trajectory_data.size() == 0){
        this -> compute_geometry_measures_action -> setEnabled(false);
    }
    else if (this -> wrapped_shape_data.size() == 0) {

        this -> compute_geometry_measures_action -> setEnabled(true);


        this -> compute_pgm_acceleration_action -> setEnabled(false);
        this -> compute_global_pgm_potential_action -> setEnabled(false);

        this -> compute_global_pgm_acceleration_action -> setEnabled(false);
        this -> compute_grav_slopes_action -> setEnabled(false);

    }

    else {

        this -> compute_geometry_measures_action -> setEnabled(true);
        this -> compute_pgm_acceleration_action -> setEnabled(true);
        this -> compute_global_pgm_acceleration_action -> setEnabled(true);
        this -> compute_global_pgm_potential_action -> setEnabled(true);

        int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
        std::string name = this -> prop_table -> item(selected_row_index, 0)-> text() .toStdString();

        if (this -> wrapped_shape_data.find(name) != this -> wrapped_shape_data.end()){

            if (this -> wrapped_shape_data[name] -> get_global_pgm_acc() == true) {

                this -> compute_grav_slopes_action -> setEnabled(true);
            }

            else {
                this -> compute_grav_slopes_action -> setEnabled(false);
            }
        }
    }

}

void Mainwindow::remove_results_visual_props(std::string name, bool remove_all) {

    // Loop over all the shape mappers to turn off the scalar visibility
    for (auto it = this -> wrapped_shape_data.begin(); it != this -> wrapped_shape_data.end(); ++it) {

        if ((it -> first == name && it -> second -> get_mapper() -> GetScalarVisibility()) || remove_all) {
            it -> second -> get_mapper() -> ScalarVisibilityOff();
            if (this -> renderer -> GetActors2D() -> GetNumberOfItems () > 0) {
                this -> renderer -> RemoveActor2D(this -> renderer -> GetActors2D() -> GetLastActor2D());
            }

            break;
        }

    }


    this -> qvtkWidget -> GetRenderWindow() -> Render();


}

void Mainwindow::show_grav_slopes() {

    QStringList valid_shapes;

    for (auto it = this -> wrapped_shape_data.begin(); it != this -> wrapped_shape_data.end(); ++it) {
        if (it -> second -> get_grav_slopes() == true) {
            valid_shapes << QString::fromStdString(it -> first);
        }
    }

    valid_shapes << QString::fromStdString("");

    bool ok_item;
    QString selected_shape_model = QInputDialog::getItem(this, "Gravitational slopes", "Toggle visibility of gravity slopes for shape model:", valid_shapes, 0, false, &ok_item);


    // If the selected shape model is a valid one
    if (ok_item && this -> wrapped_shape_data.find(selected_shape_model.toStdString()) !=  this -> wrapped_shape_data.end()) {

        // All props are just removed before displaying the one that was just selected
        this -> remove_results_visual_props("", true);

        auto active_mapper  =  this -> wrapped_shape_data[selected_shape_model.toStdString()] -> get_mapper();
        auto active_polydata  =  this -> wrapped_shape_data[selected_shape_model.toStdString()] -> get_polydata();

        if (!active_mapper -> GetScalarVisibility()) {
            active_mapper -> ScalarVisibilityOn();
            active_mapper -> SetScalarModeToUseCellData();

            double range[2] ;
            active_polydata -> GetCellData() -> SetActiveScalars("SlopeData");

            active_polydata -> GetCellData() -> GetScalars() -> GetRange(range);
            active_mapper -> SetColorModeToMapScalars();
            active_mapper -> SetScalarRange(range[0], range[1]);
            vtkLookupTable * lut = vtkLookupTable::SafeDownCast ( active_mapper -> GetLookupTable());
            lut -> SetHueRange(.667, 0);

            vtkSmartPointer<vtkScalarBarActor> scalarBar =
            vtkSmartPointer<vtkScalarBarActor>::New();
            scalarBar -> SetUnconstrainedFontSize (true);
            scalarBar -> GetTitleTextProperty() -> SetFontSize(30);
            scalarBar -> GetLabelTextProperty() -> SetFontSize(30);
            scalarBar -> SetLookupTable(active_mapper -> GetLookupTable());
            scalarBar -> SetTitle("Gravity slopes (deg)");
            scalarBar -> SetNumberOfLabels(4);

            this -> renderer -> AddActor2D(scalarBar);
        }

    }




    this -> qvtkWidget -> GetRenderWindow() -> Render();

}


void Mainwindow::show_global_pgm_pot() {

    QStringList valid_shapes;

    for (auto it = this -> wrapped_shape_data.begin(); it != this -> wrapped_shape_data.end(); ++it) {
        if (it -> second -> get_global_pgm_pot() == true) {
            valid_shapes << QString::fromStdString(it -> first);
        }
    }

    valid_shapes << QString::fromStdString("");


    bool ok_item;
    QString selected_shape_model = QInputDialog::getItem(this, "Gravitational potentials", "Toggle visibility of gravity potentials for shape model:", valid_shapes, 0, false, &ok_item);


    // If the selected shape model is a valid one
    if (ok_item && this -> wrapped_shape_data.find(selected_shape_model.toStdString()) !=  this -> wrapped_shape_data.end()) {

        // All props are just removed before displaying the one that was just selected
        this -> remove_results_visual_props("", true);

        auto active_mapper  =  this -> wrapped_shape_data[selected_shape_model.toStdString()] -> get_mapper();
        auto active_polydata  =  this -> wrapped_shape_data[selected_shape_model.toStdString()] -> get_polydata();

        if (!active_mapper -> GetScalarVisibility()) {
            active_mapper -> ScalarVisibilityOn();
            active_mapper -> SetScalarModeToUseCellData();

            double range[2] ;
            active_polydata -> GetCellData() -> SetActiveScalars("PotentialData");
            active_polydata -> GetCellData() -> GetScalars() -> GetRange(range);
            active_mapper -> SetColorModeToMapScalars();
            active_mapper -> SetScalarRange(range[0], range[1]);
            vtkLookupTable * lut = vtkLookupTable::SafeDownCast ( active_mapper -> GetLookupTable());
            lut -> SetHueRange(.667, 0);



            vtkSmartPointer<vtkScalarBarActor> scalarBar =
            vtkSmartPointer<vtkScalarBarActor>::New();
            scalarBar -> SetUnconstrainedFontSize (true);
            scalarBar -> GetTitleTextProperty() -> SetFontSize(30);
            scalarBar -> GetLabelTextProperty() -> SetFontSize(30);
            scalarBar -> SetLookupTable(active_mapper -> GetLookupTable());
            scalarBar -> SetTitle("Gravity potentials (J)");
            scalarBar -> SetNumberOfLabels(4);

            this -> renderer -> AddActor2D(scalarBar);
        }

    }

    this -> qvtkWidget -> GetRenderWindow() -> Render();
}

void Mainwindow::clear_console() {
    this -> log_console -> clear();
}


void Mainwindow::save_console() {

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to file"), "",
     tr("Text file (*.txt)"));
    if (fileName != "") {
        QFile file(fileName);

        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << this -> log_console -> toPlainText();
            file.flush();
            file.close();
        }
        else {
            QMessageBox::critical(this, tr("Error"), tr("Cannot save the file"));
            return;
        }
    }

}


void Mainwindow::show_lateral_dockwidget() {

    if (this -> lateral_dockwidget -> isVisible()) {
        this -> lateral_dockwidget -> hide();
        this -> show_lateral_dockwidget_action -> setText(QString::fromStdString("Show lateral widget"));
    }
    else {
        this -> lateral_dockwidget -> show();
        this -> show_lateral_dockwidget_action -> setText(QString::fromStdString("Hide lateral widget"));
    }
}

void Mainwindow::load_shape_model() {

    QString fileName = QFileDialog::getOpenFileName(this,
     tr("Open Shape Model"), "~/", tr("Wavefront file (*.obj)"));

    if (fileName.isEmpty() == false) {

        bool ok;
        double scaling_factor = QInputDialog::getDouble(this,
            "Scaling factor", "Enter scaling factor :", 1, 1e-6, 1e6,
            5, &ok);
        if (ok) {
            this -> log_console -> appendPlainText(QString::fromStdString("- Loading shape model from ") + fileName);


            std::chrono::time_point<std::chrono::system_clock> start, end;

            start = std::chrono::system_clock::now();
            SBGAT_CORE::ShapeModelImporter shape_io(fileName.toStdString(),
                scaling_factor, true);


            // The name of the shape model is extracted from the path
            int dot_index = fileName.lastIndexOf(".");
            int slash_index = fileName.lastIndexOf("/");
            std::string name = (fileName.toStdString()).substr(slash_index + 1 , dot_index - slash_index - 1);


            std::shared_ptr<SBGAT_CORE::ShapeModel>  shape_model = std::make_shared<SBGAT_CORE::ShapeModel>(name, this -> frame_graph.get());
            shape_io.load_shape_model(shape_model.get());

            // A new ModelDataWrapper is created and stored under the name of the shape model
            std::shared_ptr<ModelDataWrapper> model_data = std::make_shared<ModelDataWrapper>();
            model_data -> set_shape_model(shape_model);

            // The Camera is moved to be adjusted to the new shape
            this -> renderer -> GetActiveCamera() -> SetPosition(0, 0, 1.5 * scaling_factor);

            // A VTK Polydata is created from the loaded shape model
            // and displayed on the QVTKWidget. The ModelDataWrapper
            // will stored the pointer to the associated polydata, mapper and actor
            this -> create_vtkpolydata_from_shape_model(model_data);

            // The ModelDataWrapper pointer is stored. An exception
            // is thrown if the name read from the file is already
            // associated to one other ModelDataWrapper
            this -> wrapped_shape_data[name] = model_data;

            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;


            // The status bar is updated to show the significant figures of the current shape model
            std::string message("Facets : " + std::to_string(shape_model -> get_NFacets()) + " Vertices: " + std::to_string(shape_model -> get_NVertices())
                + " Edges: " + std::to_string(shape_model -> get_NEdges()));
            this -> statusBar() -> showMessage(QString::fromStdString(message));

            // The shape table is updated to show the newly loaded shape model
            this -> add_prop_to_table_widget(name);


            // The lateral dockwidget is shown if it was not visible already
            if (this -> lateral_dockwidget -> isVisible() == false) {
                this -> show_lateral_dockwidget();

            }

            // The GUI actions are updated
            this -> update_actions_availability();

            // The log console displays the name and content of the loaded shape model
            this -> log_console -> appendPlainText(QString::fromStdString("- Loading completed in ")
             + QString::number(elapsed_seconds.count()) +  QString::fromStdString(" s"));
        }
    }
}





void Mainwindow::load_trajectory() {

    QString fileName = QFileDialog::getOpenFileName(this,
     tr("Load trajectory"), "~/", tr("(*.txt)"));

    if (fileName.isEmpty() == false) {

        bool ok;
        double scaling_factor = QInputDialog::getDouble(this,
            "Scaling factor", "Enter scaling factor :", 1, 1e-6, 1e6,
            5, &ok);
        if (ok) {

            this -> log_console -> appendPlainText(QString::fromStdString("- Loading trajectory from ") + fileName);

            std::chrono::time_point<std::chrono::system_clock> start, end;

            start = std::chrono::system_clock::now();


            // The name of the trajectory is extracted from the path
            int dot_index = fileName.lastIndexOf(".");
            int slash_index = fileName.lastIndexOf("/");
            std::string name = (fileName.toStdString()).substr(slash_index + 1 , dot_index - slash_index - 1);

            arma::mat traj;
            traj.load(fileName.toStdString());

            traj *= scaling_factor;

            if(traj.n_cols < traj.n_rows){
                arma::inplace_trans(traj);
            }

            
            // Create a vtkPoints object and store the points in it
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

            for (unsigned int i = 0; i < traj.n_cols; ++i){

                arma::vec pos = traj.col(i).rows(1,3);
                points->InsertNextPoint(pos.colptr(0));

            }

            vtkSmartPointer<vtkParametricSpline> spline = 
            vtkSmartPointer<vtkParametricSpline>::New();
            spline->SetPoints(points);

            vtkSmartPointer<vtkParametricFunctionSource> functionSource = 
            vtkSmartPointer<vtkParametricFunctionSource>::New();
            functionSource->SetParametricFunction(spline);
            functionSource -> SetUResolution (traj.n_cols);
            functionSource -> Update();

            // Set the actor and mapper
            vtkSmartPointer<vtkPolyDataMapper> mapper = 
            vtkSmartPointer<vtkPolyDataMapper>::New();
            mapper -> SetInputConnection(functionSource->GetOutputPort());
            vtkSmartPointer<vtkActor> actor = 
            vtkSmartPointer<vtkActor>::New();
            actor -> SetMapper(mapper);
            this -> renderer -> AddActor(actor);

            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;

            // A new ModelDataWrapper is created and stored under the name of the trajectory
            std::shared_ptr<ModelDataWrapper> model_data = std::make_shared<ModelDataWrapper>();
            model_data -> set_polydata(mapper -> GetInput());
            model_data -> set_points(points);
            model_data -> set_actor(actor);
            model_data -> set_mapper(mapper);

            // The ModelDataWrapper pointer is stored. An exception
            // is thrown if the name read from the file is already
            // associated to one other ModelDataWrapper
            this -> wrapped_trajectory_data[name] = model_data;

            // The status bar is updated to show the significant figures of the current shape model
            std::string message("Trajectory points : " + std::to_string(traj.n_cols) );

            this -> statusBar() -> showMessage(QString::fromStdString(message));

            // The shape table is updated to show the newly loaded shape model
            this -> add_prop_to_table_widget(name);

            // The lateral dockwidget is shown if it was not visible already
            if (this -> lateral_dockwidget -> isVisible() == false) {
                this -> show_lateral_dockwidget();

            }

            // The GUI actions are updated
            this -> update_actions_availability();

            // The log console displays the name and content of the loaded shape model
            this -> log_console -> appendPlainText(QString::fromStdString("- Loading completed in ")
             + QString::number(elapsed_seconds.count()) +  QString::fromStdString(" s"));
        }
    }
}






void Mainwindow::add_prop_to_table_widget(std::string name) {


    QTableWidgetItem * nameItem = new QTableWidgetItem(QString::fromStdString(name));
    nameItem -> setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


    this -> prop_table -> insertRow(this -> prop_table -> rowCount());
    this -> prop_table -> setItem(this -> prop_table -> rowCount() - 1, 0, nameItem);


    QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
    checkBoxItem -> setCheckState(Qt::Checked);
    
    this -> prop_table -> setItem(this -> prop_table -> rowCount() - 1, 1, checkBoxItem);

    QWidget * button_container = new QWidget(this -> prop_table -> cellWidget(this -> prop_table -> rowCount() - 1, 0));
    QHBoxLayout* layout = new QHBoxLayout(button_container);

    QPushButton * erase_shape_button = new QPushButton(button_container);
    erase_shape_button -> setText("X");



    
    erase_shape_button -> setProperty("name", QVariant(QString::fromStdString(name)));
    layout -> addWidget(erase_shape_button);
    layout -> setAlignment(Qt::AlignCenter);
    layout -> setContentsMargins(0, 0, 0, 0);
    button_container -> setLayout(layout);

    this -> prop_table -> setCellWidget(this -> prop_table -> rowCount() - 1, 2, button_container);




    // The push button is connected to the proper slot
    connect(erase_shape_button, SIGNAL(clicked(bool)), this, SLOT(remove_prop()));

    // The check box is connected to the proper slot
    connect(this -> prop_table, SIGNAL(cellChanged(int, int)), this, SLOT(toggle_prop_visibility(int, int)));



    // This prop is selected
    this -> prop_table -> selectRow(this -> prop_table -> rowCount() - 1);


}


void Mainwindow::toggle_prop_visibility(int row, int col) {


    if (col == 1) {
        std::string name = this -> prop_table -> item(row, 0) -> text() .toStdString();

        auto item = this -> prop_table -> item(row, col);


        if ( this -> wrapped_shape_data.find(name)!= this -> wrapped_shape_data.end()){
            if (item -> checkState() == Qt::Checked) {
                this -> wrapped_shape_data[this -> prop_table -> item(row, 0) -> text() . toStdString()] -> get_actor() -> VisibilityOn();
            }

            else {
                this -> wrapped_shape_data[this -> prop_table -> item(row, 0) -> text() . toStdString()] -> get_actor() -> VisibilityOff();
                this -> remove_results_visual_props(this -> prop_table -> item(row, 0) -> text() . toStdString(), false);
            }
        }
        else if ( this -> wrapped_trajectory_data.find(name) != this -> wrapped_trajectory_data.end()){
         if (item -> checkState() == Qt::Checked) {
            this -> wrapped_trajectory_data[this -> prop_table -> item(row, 0) -> text() . toStdString()] -> get_actor() -> VisibilityOn();
        }

        else {
            this -> wrapped_trajectory_data[this -> prop_table -> item(row, 0) -> text() . toStdString()] -> get_actor() -> VisibilityOff();
            this -> remove_results_visual_props(this -> prop_table -> item(row, 0) -> text() . toStdString(), false);
        }
    }



}

    // The Render window is updated
this -> qvtkWidget -> GetRenderWindow() -> Render();


}





void Mainwindow::remove_prop() {

    QPushButton * button = qobject_cast<QPushButton*>(sender());
    std::string name = button -> property("name") . toString().toStdString();

    // Removal of potentially existing props corresponding to this shape
    this -> remove_results_visual_props(name, false);


    if (this -> wrapped_shape_data.find(name) != this -> wrapped_shape_data.end() ){
    // The actor of this shape is removed
        this -> renderer -> RemoveActor(this -> wrapped_shape_data[name] -> get_actor());

    // The data wrapper is removed
        this -> wrapped_shape_data.erase(name);
    }
    else{
         // The actor of this trajectory is removed
        this -> renderer -> RemoveActor(this -> wrapped_trajectory_data[name] -> get_actor());

    // The data wrapper is removed
        this -> wrapped_trajectory_data.erase(name);
    }

    // The corresponding row in the table widget is removed
    // This will trigger the corresponding signal/slot mechanism updating the GUI
    for (int i = 0; i < this -> prop_table -> rowCount(); ++i) {
        if (this -> prop_table -> item(i, 0) -> text() == QString::fromStdString(name)) {
            this -> prop_table -> removeRow(i);
            break;
        }
    }

    this -> update_actions_availability();

    // The Render window is updated
    this -> qvtkWidget -> GetRenderWindow() -> Render();

}



void Mainwindow::compute_geometry_measures(){

 int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
 std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();
 if (this -> wrapped_shape_data.find(name) != this -> wrapped_shape_data.end()){

    std::string opening_line = "### Computing shape model geometry measures ###\n";
    this -> log_console -> appendPlainText(QString::fromStdString(opening_line));
    this -> compute_surface_area();
    this -> compute_volume();
    this -> compute_inertia();
    this -> compute_center_of_mass();
    std::string closing_line(opening_line.length() - 1, '#');
    closing_line.append("\n");
    this -> log_console -> appendPlainText(QString::fromStdString(closing_line));


}
else if (this -> wrapped_trajectory_data.find(name) != this -> wrapped_trajectory_data.end()){
    std::string opening_line = "### Computing trajectory geometry measures ###\n";
    this -> log_console -> appendPlainText(QString::fromStdString(opening_line));
    std::string closing_line(opening_line.length() - 1, '#');
    closing_line.append("\n");
    this -> log_console -> appendPlainText(QString::fromStdString(closing_line));


}



}



void Mainwindow::compute_volume() {
    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();
    auto active_shape  =  this -> wrapped_shape_data[name] -> get_shape_model();

    this -> log_console -> appendPlainText(QString::fromStdString("- Volume of " + name + " (m^3) :"));
    this -> log_console -> appendPlainText(" " + QString::number(active_shape -> get_volume()));
}



void Mainwindow::compute_surface_area() {
    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();
    auto active_shape  =  this -> wrapped_shape_data[name] -> get_shape_model();

    this -> log_console -> appendPlainText(QString::fromStdString("- Surface of " + name + " (m^2) :"));
    this -> log_console -> appendPlainText(" " + QString::number(active_shape -> get_surface_area()));
}

void Mainwindow::compute_inertia() {
    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();
    auto active_shape  =  this -> wrapped_shape_data[name] -> get_shape_model();

    this -> log_console -> appendPlainText(QString::fromStdString("- Dimensionless inertia tensor of " + name + " :"));
    std::stringstream ss;
    active_shape -> get_inertia().print(ss);
    this -> log_console -> appendPlainText(QString::fromStdString(ss.str()));
}

void Mainwindow::compute_center_of_mass() {
    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();
    auto active_shape  =  this -> wrapped_shape_data[name] -> get_shape_model();

    this -> log_console -> appendPlainText(QString::fromStdString("- Center of mass coordinates of " + name + " (m) :"));
    std::stringstream ss;
    active_shape -> get_center_of_mass().print(ss);
    this -> log_console -> appendPlainText(QString::fromStdString(ss.str()));
}










void Mainwindow::create_vtkpolydata_from_shape_model(std::shared_ptr<ModelDataWrapper> model_data) {


    // Add the polygon to a list of polygons
    vtkSmartPointer<vtkCellArray> polygons =
    vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> points =
    vtkSmartPointer<vtkPoints>::New();

    auto shape_model = model_data -> get_shape_model();

    for (unsigned int facet_index = 0; facet_index < shape_model -> get_NFacets(); ++facet_index) {

        auto vertices = shape_model -> get_facets() -> at(facet_index) -> get_vertices() ;

        double * p0 = vertices -> at(0) -> get_coordinates() -> colptr(0);
        double * p1 = vertices -> at(1) -> get_coordinates() -> colptr(0);
        double * p2 = vertices -> at(2) -> get_coordinates() -> colptr(0);

        points -> InsertNextPoint(p0);
        points -> InsertNextPoint(p1);
        points -> InsertNextPoint(p2);

        // Create the polygon
        vtkSmartPointer<vtkPolygon> polygon =
        vtkSmartPointer<vtkPolygon>::New();
        polygon -> GetPointIds() -> SetNumberOfIds(3); //make a triangle
        polygon -> GetPointIds() -> SetId(0, 3 * facet_index);
        polygon -> GetPointIds() -> SetId(1, 3 * facet_index + 1);
        polygon -> GetPointIds() -> SetId(2, 3 * facet_index + 2);

        polygons -> InsertNextCell(polygon);


    }


    // Create a PolyData
    vtkSmartPointer<vtkPolyData> polygonPolyData =
    vtkSmartPointer<vtkPolyData>::New();
    polygonPolyData -> SetPoints(points);
    polygonPolyData -> SetPolys(polygons);

    // Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();

    mapper -> SetInputData(polygonPolyData);
    mapper -> ScalarVisibilityOff();

    vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
    actor -> SetMapper(mapper);

    // Visualize
    this -> renderer -> AddActor(actor);

    // Render
    this -> qvtkWidget -> GetRenderWindow() -> Render();

    // Store
    model_data -> set_polydata(polygonPolyData);
    model_data -> set_actor(actor);
    model_data -> set_mapper(mapper);

}








void Mainwindow::compute_global_pgm_acceleration() {

    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();

    auto dyn_analyses = std::make_shared<SBGAT_CORE::DynamicAnalyses>(this -> wrapped_shape_data[name] -> get_shape_model().get());

    bool ok_density = false;


    double density = QInputDialog::getDouble(this,
       "Global Polyhedron Gravity Model Acceleration", "Density (kg/m^3) :", 2000, 0, 1e9,
       5, &ok_density);




    if (ok_density) {

        double mu = density * arma::datum::G * this -> wrapped_shape_data[name] -> get_shape_model() -> get_volume();


        // Log in
        this -> log_console -> appendPlainText(QString::fromStdString("- Computing global PGM facet accelerations of " + name + " ..."));

        // The shape selection table is frozen
        this -> prop_table -> setDisabled(true);
        this -> menuBar() -> setDisabled(true);


        QThread * thread = new QThread;
        Worker * worker = new Worker(dyn_analyses, mu, this -> wrapped_shape_data[name],
           name);
        worker ->  moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process_pgm_acc()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(logging_out(QString)), this -> log_console, SLOT(insertPlainText(QString)));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished()), this, SLOT(update_actions_availability()));
        connect(worker, SIGNAL(free_shape_table(bool)), this -> prop_table, SLOT(setEnabled(bool)));
        connect(worker, SIGNAL(free_menu_bar(bool)), this -> menuBar() , SLOT(setEnabled(bool)));

        thread -> start();


    }

}



void Mainwindow::compute_global_pgm_potential() {

    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();

    auto dyn_analyses = std::make_shared<SBGAT_CORE::DynamicAnalyses>(this -> wrapped_shape_data[name] -> get_shape_model().get());

    bool ok_density = false;


    double density = QInputDialog::getDouble(this,
       "Global Polyhedron Gravity Model Acceleration", "Density (kg/m^3) :", 2000, 0, 1e9,
       5, &ok_density);

    if (ok_density) {


        double mu = density * arma::datum::G * this -> wrapped_shape_data[name] -> get_shape_model() -> get_volume();



        // Log int
        this -> log_console -> appendPlainText(QString::fromStdString("- Computing global PGM facet potentials of " + name + " ..."));

        // The shape selection table is frozen
        this -> prop_table -> setDisabled(true);
        this -> menuBar()  -> setDisabled(true);


        QThread * thread = new QThread;
        Worker * worker = new Worker(dyn_analyses,
           mu,
           this -> wrapped_shape_data[name],
           name);
        worker ->  moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process_pgm_pot()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(logging_out(QString)), this -> log_console, SLOT(insertPlainText(QString)));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished()), this, SLOT(update_actions_availability()));
        connect(worker, SIGNAL(finished()), this, SLOT(update_vtk_potentials()));
        connect(worker, SIGNAL(free_shape_table(bool)), this -> prop_table, SLOT(setEnabled(bool)));
        connect(worker, SIGNAL(free_menu_bar(bool)), this -> menuBar() , SLOT(setEnabled(bool)));

        thread -> start();

    }

}


void Mainwindow::compute_gravity_slopes() {
    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();


    SBGAT_CORE::DynamicAnalyses dynas(this -> wrapped_shape_data[name] -> get_shape_model().get());

    bool ok_spin_axis = true;
    bool correct_format = false;
    bool ok_spin_rate = false;
    arma::vec spin_axis = {0, 0, 1};
    arma::vec angles_arma = {0, 0, 0};

    while (ok_spin_axis == true && correct_format == false) {

        QString coords = QInputDialog::getText(this,
         tr("Gravity slopes"),
         tr("(Azimuth,Elevation) of spin axis (deg) . (0,0) : along z :"),
         QLineEdit::Normal,
         QString::fromStdString("Azimuth,Elevation") ,
         &ok_spin_axis);

        QStringList angles_split = coords.split(",");

        if (angles_split.count() != 2) {
            correct_format = false;
            continue;
        }

        // Matching doubles
        QRegExp re("[-+]?[0-9]*\\.?[0-9]+");

        if (re.exactMatch(angles_split.at(0)) && re.exactMatch(angles_split.at(1))) {
            angles_arma(0) = angles_split.at(0).toDouble();
            angles_arma(1) = angles_split.at(1).toDouble();

            correct_format = true;


        }
        else {
            correct_format = false;
            continue;
        }

    }

    if (ok_spin_axis) {
        spin_axis = RBK::euler313_to_dcm(angles_arma).t() * spin_axis;

        double period = QInputDialog::getDouble(this,
            "Gravity slopes", "Rotation period (hours) :", 0, -1e9, 1e9,
            5, &ok_spin_rate);

        double spin_rate = arma::datum::pi * 2 / (period * 3600);

        if (ok_spin_rate ) {

            this -> log_console -> appendPlainText(QString::fromStdString("- Computing gravity slopes of " + name +  "..."));

            std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
            arma::vec slope_stats = dynas.compute_gravity_slopes(spin_axis, spin_rate);

            this -> update_vtk_slopes();


            // A GUI flag is updated to indicate that this shape model has consistent slopes ready to be displayed
            this -> wrapped_shape_data[name] -> set_grav_slopes(true);

            // The GUI actions are updated
            this -> update_actions_availability();

            // The previously displayed slopes (if any) are removed
            this -> remove_results_visual_props("", true);

            // Some statistics regarding the slopes are printed
            std::string message("-- Mean slope: " + std::to_string(slope_stats(1)) + " deg");
            this -> log_console -> appendPlainText(QString::fromStdString(message));
            message = ("-- Minimum slope: " + std::to_string(slope_stats(0)) + " deg");
            this -> log_console -> appendPlainText(QString::fromStdString(message));
            message = ("-- Maximum slope: " + std::to_string(slope_stats(2)) + " deg");
            this -> log_console -> appendPlainText(QString::fromStdString(message));

            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;

            this -> log_console -> appendPlainText(QString::fromStdString("- Done computing in ")
             + QString::number(elapsed_seconds.count()) +  QString::fromStdString(" s"));

        }

    }

}

void Mainwindow::update_vtk_potentials() {

    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();

    vtkSmartPointer<vtkPolyData> active_polydata =  this -> wrapped_shape_data[name] -> get_polydata();
    vtkSmartPointer<vtkPolyDataMapper> active_mapper = this -> wrapped_shape_data[name] -> get_mapper();
    std::shared_ptr<SBGAT_CORE::ShapeModel> active_shape_model = this -> wrapped_shape_data[name] -> get_shape_model();

    vtkSmartPointer<vtkDoubleArray> potential_data =
    vtkSmartPointer<vtkDoubleArray>::New();
    potential_data -> SetNumberOfValues(active_shape_model -> get_NFacets());
    potential_data -> SetName("PotentialData");

    for (unsigned int facet_index = 0; facet_index < active_shape_model -> get_NFacets(); ++facet_index) {

        SBGAT_CORE::Facet * facet =  active_shape_model -> get_facets() -> at(facet_index);

        potential_data -> SetValue(facet_index, facet -> get_facet_results() -> get_grav_potential());
    }

    // The array is added to the polydata
    active_polydata -> GetCellData() -> SetActiveScalars("PotentialData");
    active_polydata -> GetCellData() -> SetScalars(potential_data);
    active_polydata -> Modified();

}

void Mainwindow::update_vtk_slopes() {

    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0) -> text() .toStdString();

    vtkSmartPointer<vtkPolyData> active_polydata =  this -> wrapped_shape_data[name] -> get_polydata();
    vtkSmartPointer<vtkPolyDataMapper> active_mapper = this -> wrapped_shape_data[name] -> get_mapper();
    std::shared_ptr<SBGAT_CORE::ShapeModel> active_shape_model = this -> wrapped_shape_data[name] -> get_shape_model();

    vtkSmartPointer<vtkDoubleArray> slope_data =
    vtkSmartPointer<vtkDoubleArray>::New();
    slope_data -> SetNumberOfValues(active_shape_model -> get_NFacets());
    slope_data -> SetName("SlopeData");

    for (unsigned int facet_index = 0; facet_index < active_shape_model -> get_NFacets(); ++facet_index) {

        SBGAT_CORE::Facet * facet =  active_shape_model -> get_facets() -> at(facet_index);

        slope_data -> SetValue(facet_index, facet -> get_facet_results() -> get_grav_slope());
    }

    // The array is added to the polydata
    active_polydata -> GetCellData() -> SetActiveScalars("SlopeData");
    active_polydata -> GetCellData() -> SetScalars(slope_data);
    active_polydata -> Modified();

}


void Mainwindow::compute_pgm_acceleration() {

    int selected_row_index = this -> prop_table -> selectionModel() -> currentIndex().row();
    std::string name = this -> prop_table -> item(selected_row_index, 0)-> text() .toStdString();

    SBGAT_CORE::DynamicAnalyses dynas(this -> wrapped_shape_data[name] -> get_shape_model().get());

    bool ok_coords = true;
    bool correct_format = false;
    double point[3];
    bool ok_density = false;
    double density;

    while (ok_coords == true && correct_format == false) {

        QString coords = QInputDialog::getText(this,
         tr("Polyhedron Gravity Model Acceleration"),
         tr("Body-fixed frames coordinates (m) :"),
         QLineEdit::Normal,
         QString::fromStdString("x,y,z") ,
         &ok_coords);

        QStringList coords_split = coords.split(",");

        if (coords_split.count() != 3) {
            correct_format = false;
            continue;
        }

        // Matching doubles
        QRegExp re("[-+]?[0-9]*\\.?[0-9]+");

        if (re.exactMatch(coords_split.at(0)) && re.exactMatch(coords_split.at(1)) && re.exactMatch(coords_split.at(2))) {
            point[0] = coords_split.at(0).toDouble();
            point[1] = coords_split.at(1).toDouble();
            point[2] = coords_split.at(2).toDouble();

            correct_format = true;


        }
        else {
            correct_format = false;
            continue;
        }

    }



    if (ok_coords) {
        density = QInputDialog::getDouble(this,
          "Polyhedron Gravity Model Acceleration", "Density (kg/m^3) :", 2000, 0, 1e9,
          5, &ok_density);


        if (ok_density) {

            double mu = density * arma::datum::G * this -> wrapped_shape_data[name] -> get_shape_model() -> get_volume();



            // The PGM acceleration is computed at the provided point
            arma::vec coords_arma = {point[0], point[1], point[2]};
            std::stringstream ss_coords;
            ss_coords.precision(10);
            ss_coords << " " << point[0] << "\n" << " " << point[1] << "\n" << " " << point[2] << "\n";

            arma::vec acc = dynas.pgm_acceleration(point , mu);
            std::stringstream ss_acc;
            ss_acc.precision(10);
            ss_acc << " " << acc[0] << "\n" << " " << acc[1] << "\n" << " " << acc[2] << "\n";

            this -> log_console -> appendPlainText(QString::fromStdString("\n- At body-fixed coordinates (m) : "));
            this -> log_console -> appendPlainText(QString::fromStdString(ss_coords.str()));
            this -> log_console -> appendPlainText(QString::fromStdString("- PGM acceleration (m/s^2): "));
            this -> log_console -> appendPlainText(QString::fromStdString(ss_acc.str()));

        }
    }


}




void Mainwindow::createMenus() {
    this -> FileMenu = this -> menuBar() -> addMenu(tr("&File"));
    this -> FileMenu -> addAction(this -> load_shape_model_action);
    this -> FileMenu -> addSeparator();
    this -> FileMenu -> addAction(this -> load_trajectory_action);
    this -> FileMenu -> addAction(this -> open_settings_window_action);


    this -> ViewMenu = menuBar() -> addMenu(tr("&View"));
    this -> ViewMenu -> addAction(this -> show_lateral_dockwidget_action);
    this -> ViewMenu -> addSeparator();

    this -> ShapeMenu = menuBar() -> addMenu(tr("&Measures"));
    this -> ShapeMenu -> addAction(this -> compute_geometry_measures_action);


    this -> DynamicAnalysesMenu = menuBar() -> addMenu(tr("&Analyses"));
    this -> DynamicAnalysesMenu -> addAction(this -> compute_pgm_acceleration_action);
    this -> DynamicAnalysesMenu -> addSeparator();
    this -> DynamicAnalysesMenu -> addAction(this -> compute_global_pgm_potential_action);
    this -> DynamicAnalysesMenu -> addAction(this -> compute_global_pgm_acceleration_action);
    this -> DynamicAnalysesMenu -> addAction(this -> compute_grav_slopes_action);


    this -> ResultsMenu = menuBar() -> addMenu(tr("&Visualization"));
    this -> ResultsMenu -> addAction(this -> show_grav_slopes_action);
    this -> ResultsMenu -> addAction(this -> show_global_pgm_pot_action);


    this -> ConsoleMenu = menuBar() -> addMenu(tr("&Console"));
    this -> ConsoleMenu -> addAction(this -> clear_console_action);
    this -> ConsoleMenu -> addAction(this -> save_console_action);







}

std::pair<std::string,vtkSmartPointer<vtkActor> > Mainwindow::get_skybox_pair() const{
    return this ->skybox_pair;
}

void Mainwindow::set_skybox_actor(vtkSmartPointer<vtkActor> skybox_actor){
    this -> skybox_pair.second = skybox_actor;
}


void Mainwindow::set_skybox_directory(std::string skybox_dir){
    this -> skybox_pair.first = skybox_dir;
}



vtkSmartPointer<vtkRenderer> Mainwindow::get_renderer() {
    return this -> renderer;
}

