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
 * @class  SBGATPolyhedronGravityModel
 * @author Benjamin Bercovici
 * @brief  computes the acceleration caused by a constant-density polyhedron
 *
 * SBGATPolyhedronGravityModel.hpp evaluates the potential, acceleration caused by a polyhedron
 of constant density by evaluating the so called Polyhedron Gravity Model as derived by Werner and Scheeres 
 * See Werner, R. A., & Scheeres, D. J. (1997). Exterior gravitation of a polyhedron derived and compared with harmonic and mascon gravitation representations of asteroid 4769 Castalia. Celestial Mechanics and Dynamical Astronomy, 65(3), 313–344. https://doi.org/10.1007/BF00053511
 * for more details
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
  Evaluates the Polyhedron Gravity Model at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame as
  the polydata
  @param density constant density in kg/m^3
  @return PGM potential evaluated at the queried point
  */
  double ComputePgmPotential(double * point ,const double density);


  /**
  Determines whether the provided point lies inside or outside the shape
  @param point coordinates of queried point, expressed in the same frame as
  the polydata
  @param tolerance
  @return true if the polydata contains the point, false otherwise
  */
  bool Contains(double * point, double tol = 1e-7);



protected:
  SBGATPolyhedronGravityModel();
  ~SBGATPolyhedronGravityModel() override;

  int RequestData(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  static void ComputeEdgeDyad(double * edge_dyad,
    double * nA, double * nB,double * p1, double * p2);

  double ** facet_dyads;
  double ** edge_dyads;
  double ** facet_normals;
  int ** edges;

  vtkSmartPointer<SBGATMassProperties> mass_properties;





private:
  SBGATPolyhedronGravityModel(const SBGATPolyhedronGravityModel&) = delete;
  void operator=(const SBGATPolyhedronGravityModel&) = delete;
};

#endif


