
// Adapted from the original work
// of Nicola Baresi, CSML, University of Colorado Boulder (2014)


#ifndef ASTEROIDHEADERDEF
#define ASTEROIDHEADERDEF

#include <fstream>
#include <iostream>

#include "Matrix.hpp"
#include "Vect.hpp"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <cassert>
#include <cmath>
#include <set>
#include <map>
#include <fstream>
#include <armadillo>

/**
Declaration of the Asteroid Class. Stores
  - the shape model of the asteroid in the form of a vtkSmartPointer<vtkPolydata>
  - the PGM (raw accelerations evaluated at the center of each facet)
  - the principal axes/inertias (TBD)
  - the other physical properties of the asteroid
    - density
    - spin rate
    - spin axis direction
*/

class Asteroid {
private:

  double mGs;   // G*dens product

  unsigned int mNOE; //No. of edges

  // Vertex coordinates
  double* mX;
  double* mY;
  double* mZ;

  // Triangle Vertex List
  int** mListTri;

  // Face Normal Unit Vectors
  double** mListN;

  // Surface Gravity Acceleration Vectors
  double** surface_grav;

  // Face Dyads
  double** mF;

  // Edge Vertex List
  int** mListE;

  // Edge Dyads
  double** mE;


public:

  Asteroid(vtkSmartPointer<vtkPolyData> polydata, double Gs);

  ~Asteroid();

  /**
  Returns the G * density product
  @return G * density product
  */
  double GetGs() const;

  /**
  Returns the number of vertices
  @return Number of vertices
  */
  unsigned int GetNOV() const;
  /**
  Returns the number of facets
  @return Number of facets
  */
  unsigned int GetNOF() const;
  /**
  Returns the number of edges
  @return Number of edges
  */
  unsigned int GetNOE() const;


  /**
  Returns pointer to X coordinates
  @return Pointer to array of X coordinates
  */
  double * get_X()  ;
  /**
  Returns pointer to Y coordinates
  @return Pointer to array of Y coordinates
  */
  double * get_Y()  ;
  /**
  Returns pointer to Z coordinates
  @return Pointer to array of Z coordinates
  */
  double * get_Z()  ;

  Vect GetListTri() const;
  Vect GetListN() const;
  Vect GetF() const;

  double ** get_ListN() ;
  int ** get_ListTri() ;

  Vect GetListE() const;
  Vect GetE() const;

  double ** get_surface_grav();

  /**
  Returns the current spin rate of the asteroid
  @return Spin rate (rad/s)
  */
  double get_spin_rate() const;

  /**
  Returns the coordinates of the spin axis expressed in the 
  asteroid's bodyframe.
  @return Spin axis unit direction
  */
  arma::vec get_spin_axis() const;


  /** 
  Sets the asteroid's spin rate
  @param spin_rate Asteroid's spin rate (rad/s)
  */
  void set_spin_rate(double spin_rate);

  /**
  Sets the asteroid's spin axis coordinates. The provided
  vector is automatically normalized
  @param spin_axis Spin axis direction. Must have a non-zero norm
  */
  void set_spin_axis(arma::vec & spin_axis);

  /**
  Sets the asteroid's constant density
  @param density Density of asteroid (kg/m^3)
  */
  void set_density(double density);

  /**
  Returns the asteroid's density
  @return Asteroid's density (kg/m^3)
  */
  double get_density();

  /**
  Legacy method. Sets the G * density method by effectively changing the 
  density of the asteroid
  @param mGs G * density product of the asteroid
  */
  void setmGs(double mGs);


  /**
  Computes the acceleration of gravity created by the asteroid 
  at the provided location
  @param Xsc Coordinates at which the pgm acceleration must be evaluated (m)
  */
  Vect PolyGrav(Vect& Xsc);

  /**
  Evaluates the PGM gravity acceleration at the center of each facet on the 
  asteroid
  */
  void compute_global_pgm();

  /**
  Returns a pointer to the polydata
  @return Pointer to polydata
  */
  vtkSmartPointer<vtkPolyData> get_polydata();

  /**
  Saves asteroid connectivity (vertices + facets) to an obj file
  @param filename Path to file
  */
  void write_to_obj(std::string filename);

  /**
  Saves the current surface acceleration (from the polyhedron gravity model)
  to file
  @param filename Path to file
  */
  void write_surface_acceleration(std::string filename);

  /**
  Loads a previously computed surface acceleration (from the polyhedron gravity model)
  @param filename Path to file
  @return - 1 if PGM gravity file was consistent with asteroid shape model
          - 0 otherwise (no surface acceleration will be loaded)
  */
  int load_surface_acceleration(std::string filename);


protected:
  arma::vec spin_axis;
  double spin_rate;
  vtkSmartPointer<vtkPolyData> polydata;


};


#endif
