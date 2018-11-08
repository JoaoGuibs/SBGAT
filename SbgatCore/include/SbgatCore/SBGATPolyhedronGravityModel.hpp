/*=========================================================================

  Program:   Small Body Geophysical Analysis
  Module:    SBGATPolyhedronGravityModel.hpp

  Derived class from VTK's vtkPolyDataAlgorithm by Benjamin Bercovici  

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
@file SBGATPolyhedronGravityModel.hpp
@class  SBGATPolyhedronGravityModel
@author Benjamin Bercovici
@date October 2018

@brief  Evaluation of potential, acceleration caused by a constant-density polyhedron
 @details Computes the potential, acceleration caused by a polyhedron
 of constant density by evaluating the so called Polyhedron Gravity Model as derived by Werner and Scheeres.
The input must be a topologically-closed polyhedron.
See Werner, R. A., & Scheeres, D. J. (1997). Exterior gravitation of a polyhedron derived and compared with harmonic and mascon gravitation representations of asteroid 4769 Castalia. Celestial Mechanics and Dynamical Astronomy, 65(3), 313–344. https://doi.org/10.1007/BF00053511
for further details. Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
@copyright MIT License, Benjamin Bercovici and Jay McMahon


*/

#ifndef SBGATPolyhedronGravityModel_hpp
#define SBGATPolyhedronGravityModel_hpp

#include <vtkFiltersCoreModule.h> // For export macro
#include <vtkPolyDataAlgorithm.h>
#include <armadillo>
#include "SBGATMassProperties.hpp"









class VTKFILTERSCORE_EXPORT SBGATPolyhedronGravityModel : public vtkPolyDataAlgorithm{
public:

  /**
   * Constructs with initial values of zero.
   */

  static SBGATPolyhedronGravityModel *New();

  vtkTypeMacro(SBGATPolyhedronGravityModel,vtkPolyDataAlgorithm);
  void PrintSelf(std::ostream& os, vtkIndent indent) override;
  void PrintHeader(std::ostream& os, vtkIndent indent) override;
  void PrintTrailer(std::ostream& os, vtkIndent indent) override;

  
  /**
  Evaluates the Polyhedron Gravity Model potential at the specified point assuming 
  a constant density
  @param point pointer to coordinates of queried point, expressed in the same frame as
  the polydata
  @return PGM potential evaluated at the queried point (L ^ 2/ s ^2)
  */
  double GetPotential(double const * point);

  /**
  Evaluates the Polyhedron Gravity Model potential at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/same unit L as
  the polydata
  @return PGM potential evaluated at the queried point (L ^ 2 / s ^2)
  */
  double GetPotential(const arma::vec & point);

  /**
  Evaluates the Polyhedron Gravity Model potential and acceleration at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/unit L as
  the polydata used to construct the PGM
  @param PGM potential evaluated at the queried point (L ^ 2 / s ^2)
  @param PGM acceleration evaluated at the queried point (L / s ^2)
  */
  void GetPotentialAcceleration(double const * point,double & potential, 
    arma::vec & acc);


  /**
  Evaluates the Polyhedron Gravity Model potential and acceleration at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/unit L as
  the polydata used to construct the PGM
  @param PGM potential evaluated at the queried point (L ^ 2 / s ^2)
  @param PGM acceleration evaluated at the queried point (L / s ^2)
  */
  void GetPotentialAcceleration(const arma::vec & point,double & potential, 
    arma::vec & acc);


  /**
  Evaluates the Polyhedron Gravity Model acceleration at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/unit as
  the polydata used to construct the PGM
  @return PGM acceleration evaluated at the queried point (L / s ^2)
  */
  arma::vec GetAcceleration(const arma::vec & point);

  /**
  Evaluates the Polyhedron Gravity Model acceleration at the specified point assuming 
  a constant density
  @param point pointer to coordinates of queried point, expressed in the same frame/same unit L as
  the input polydata
  @return PGM acceleration evaluated at the queried point (L / s ^2)
  */
  arma::vec GetAcceleration(double const * point);

  /** 
  Determines whether the provided point lies inside or outside the shape
  @param point coordinates of queried point, expressed in the same frame as
  the polydata
  @param tolerance
  @return true if the polydata contains the point, false otherwise
  */
  bool Contains(double const * point, double tol = 1e-8);

  /**
  Sets the scale factor to 1, indicative that the polydata has its coordinates expressed in meters
  */
  void SetScaleMeters() { this -> scaleFactor = 1; this -> scaleFactorSet = true;}

  /**
  Sets the scale factor to 1000, indicative that the polydata has its coordinates expressed in kilometers
  */
  void SetScaleKiloMeters() { this -> scaleFactor = 1000; this -> scaleFactorSet = true;}

  /**
  Sets polyhedron density
  @param density bulk density of polyhedron (kg/m^3)
  */
  void SetDensity(const double density){
    this -> density = density;
    this -> densitySet = true;
  }

  /**
  Evaluates the Polyhedron Gravity Model at the surface of the specified surface elements in the provided shape
  @param[in] selected_shape shape for which the surface polyhedron gravity model must be computed
  @param[in] queried_elements shape indices of elements where the polyhedron gravity model should be evaluated
  @param[in] is_in_meters true if the shape coordinates were expressed in meters, false if they were expressed in kilometers
  @param[in] density shape bulk density in kg/m^3
  @param[in] omega fixed angular velocity of shape expressed in rad/s
  @param[out] slopes vector storing the gravitational slopes (degrees) evaluated at the center of each queried element 
  @param[out] potentials vector storing the inertial gravitational potentials (m^2/s^2) evaluated at the center of each queried element 
  @param[out] acc_magnitudes vector storing the inertial gravitational acceleration magnitudes  (m/s^2) evaluated at the center of each queried element 
  @param[out] acc_body_fixed_magnitudes vector storing the body-fixed gravitational acceleration magnitudes (m/s^2) evaluated at the center of each queried element 
  */
  static void ComputeSurfacePGM(
  vtkSmartPointer<vtkPolyData> selected_shape,
  const std::vector<unsigned int> & queried_elements,
  bool is_in_meters,
  double density,
  const arma::vec::fixed<3> & omega,
  std::vector<double> & slopes,
  std::vector<double> & potentials,
  std::vector<double> & acc_magnitudes,
  std::vector<double> & acc_body_fixed_magnitudes);




protected:
  SBGATPolyhedronGravityModel();
  ~SBGATPolyhedronGravityModel() override;


  void Clear();


  int RequestData(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  double ** facet_dyads;
  double ** edge_dyads;
  double ** facet_normals;
  double ** vertices;

  double scaleFactor;
  double density;

  int ** edges;
  int ** facets;

  bool scaleFactorSet;
  bool densitySet;

  int N_facets;
  int N_edges;


  vtkSmartPointer<SBGATMassProperties> mass_properties;


private:
  SBGATPolyhedronGravityModel(const SBGATPolyhedronGravityModel&) = delete;
  void operator=(const SBGATPolyhedronGravityModel&) = delete;
};

#endif


