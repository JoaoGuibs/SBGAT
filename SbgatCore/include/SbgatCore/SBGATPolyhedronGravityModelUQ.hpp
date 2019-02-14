/**
@file SBGATPolyhedronGravityModelUQ.hpp
@class  SBGATPolyhedronGravityModelUQ
@author Benjamin Bercovici
@date January 2019

@brief  Evaluation of the formal uncertainty in the potential (variance), acceleration (covariance) caused by a constant-density polyhedron
 @details Computes the potential variance, acceleration covariance associated to the gravity deriving from the polyhedron
 of constant density assuming that the underlying shape vertices are outcomes of a Gaussian distribution 
 of known mean and covariance
The input must be a topologically-closed polyhedron.
See Werner, R. A., & Scheeres, D. J. (1997). Exterior gravitation of a polyhedron derived and compared with harmonic and mascon gravitation representations of asteroid 4769 Castalia. Celestial Mechanics and Dynamical Astronomy, 65(3), 313–344. https://doi.org/10.1007/BF00053511
for further details. Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
@copyright MIT License, Benjamin Bercovici and Jay McMahon
*/

#ifndef SBGATPolyhedronGravityModelUQ_hpp
#define SBGATPolyhedronGravityModelUQ_hpp

#include <armadillo>
#include "SBGATMassProperties.hpp"
#include "SBGATPolyhedronGravityModel.hpp"


class SBGATPolyhedronGravityModelUQ {
public:


  /**
  Sets the PGM model associated to this uncertainty quantification run
  @param[in] pgm pointer to valid polyhedron gravity model
  */
  void SetPGM(vtkSmartPointer<SBGATPolyhedronGravityModel> pgm){this -> pgm_model = pgm;}

  /**
  Get the PGM model associated to this uncertainty quantification run
  @return pgm pointer to valid polyhedron gravity model
  */
  vtkSmartPointer<SBGATPolyhedronGravityModel> GetPGM(){return this -> pgm_model ;}

  
  /**
  Evaluates the Polyhedron Gravity Model potential variance at the specified point assuming 
  a constant density
  @param point pointer to coordinates of queried point, expressed in the same frame as
  the polydata
  @return PGM potential variance evaluated at the queried point (L ^ 4/ s ^4)
  */
  double GetPotentialVariance(double const * point) const;

  /**
  Evaluates the Polyhedron Gravity Model potential variance at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/same unit L as
  the polydata
  @return PGM potential variance evaluated at the queried point (L ^ 4 / s ^4)
  */
  double GetPotentialVariance(const arma::vec::fixed<3> & point) const;

  /**
  Evaluates the Polyhedron Gravity Model potential variance and acceleration covariance at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/unit L as
  the polydata used to construct the PGM
  @param[out] potential_var PGM potential variance evaluated at the queried point (L ^ 4 / s ^4)
  @param[out] acc_cov PGM acceleration covariance evaluated at the queried point (L^2 / s ^4)
  */
  void GetPotentialVarianceAccelerationCovariance(double const * point,double & potential_var, 
    arma::mat::fixed<3,3> & acc_cov) const;

  /**
  Evaluates the Polyhedron Gravity Model potential variance and acceleration covariance at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame/unit L as
  the polydata used to construct the PGM
  @param[out] potential_var PGM potential variance evaluated at the queried point (L ^ 4 / s ^4)
  @param[out] acc_cov PGM acceleration covariance evaluated at the queried point (L^2 / s ^4)
  */
  void GetPotentialVarianceAccelerationCovariance(const arma::vec::fixed<3> & point,double & potential_var, 
    arma::mat::fixed<3,3> & acc_cov) const;



  /**
  Returns the partial derivative of an individual edge contribution to the potential (Ue) 
  with respect to the Xe^E vector holding the e-th edge dyadic factors
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialUePartialXe (1x10)

  */
  arma::rowvec::fixed<10> PartialUePartialXe(const arma::vec::fixed<3> & pos,const int & e) const;

  

  

  /**
  Returns the partial derivative of an individual facet contribution to the potential (Uf) 
  with respect to the Xf^F vector holding the f-th facet dyadic factors
  @param pos position where to evaluate the partial
  @param f facet index
  @return PartialUfPartialXf (1x10)

  */
  arma::rowvec::fixed<10> PartialUfPartialXf(const arma::vec::fixed<3> & pos,
    const int & f) const;



  /**
  Returns the partial derivative of Xf^F, the vector holding the f-th facet dyadic factors, 
  with respect to the vertices coordiantes constitutive of the f-th triangle (Tf) 
  @param pos position where to evaluate the partial
  @param f facet index
  @return PartialXfPartialTf (10x9)
  */
  arma::mat::fixed<10,9> PartialXfPartialTf(const arma::vec::fixed<3> & pos, const int & f) const;


  /**
  Returns the partial derivative of the performance factor omega_f
  with respect to the vertices coordiantes constitutive of the f-th triangle (Tf) 
  @param pos position where to evaluate the partial
  @param f facet index
  @return PartialOmegafPartialTf (1x9)
  */
  arma::rowvec::fixed<9> PartialOmegafPartialTf(const arma::vec::fixed<3> & pos,const int & f) const;



  /**
  Returns the partial derivative of the facet dyad parametrization (Ff)
  with respect to the vertices coordinates constitutive of the f-th triangle (Tf) 
  @param pos position where to evaluate the partial
  @param f facet index
  @return PartialFfPartialTf (6x9)
  */
  arma::mat::fixed<6,9> PartialFfPartialTf(const arma::vec::fixed<3> & pos,const int & f) const;



  /**
  Returns the partial derivative of a normalized vector n relative to the non-normalized
  vector N such that n = N / || N ||
  @param non_normalized_V non-normalized vector used to produce the normalized vector
  @return PartialNormalizedVPartialNonNormalizedV (3x3)
  */
  static arma::mat::fixed<3,3> PartialNormalizedVPartialNonNormalizedV(const arma::vec::fixed<3> & non_normalized_V);


  /**
  Returns the partial derivative of the non-normalized normal at the f-th facet 
  with respect to the vertices coordinates constitutive of the f-th triangle (Tf) 
  @param f facet index
  @return PartialNfPartialTf (3x3)

  */
  arma::mat::fixed<3,3> PartialNfPartialTf(const arma::vec::fixed<3> & pos,const int & f) const;



  /**
  Returns the partial derivative of the f-th facet dyad parametrization with respect to the 
  normalized normal of the f-th facet
  @param f facet index
  @return PartialFfPartialnf (3x3)

  */
  arma::mat::fixed<3,3> PartialFfPartialnf(const int & f) const;



  /**
  Returns the partial derivative of the wire potential Le 
  with respect to the coordinates of the two vertices forming the edge (stacked in Ae)
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialLePartialAe (1x6)
  */
  arma::rowvec::fixed<6> PartialLePartialAe(const arma::vec::fixed<3> & pos,const int & e) const;


  /**
  Returns the partial derivative of field-point to edge-point vector
  with respect to the coordinates of the two vertices forming the edge (stacked in Ae)
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialRadiusEePartialAe (3x6)
  */
  arma::mat::fixed<3,6> PartialRadiusEePartialAe(const arma::vec::fixed<3> & pos,const int & e) const;
  


  /**
  Returns the partial derivative of the parametrization of the Ee dyad 
  with respect to the coordinates of the two vertices forming the edge (stacked in Ae)
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialEePartialAe (6x6)
  */
  arma::mat::fixed<6,6> PartialEePartialAe(const arma::vec::fixed<3> & pos,const int & e) const;



  /**
  Returns the partial derivative of the parametrization of the Ee dyad 
  with respect to the coordinates of one of the facets adjacent to it
  @param pos position where to evaluate the partial
  @param f facet index
  @return PartialEePartialAe (6x9)
  */
  arma::mat::fixed<6,9> PartialEePartialTf(const arma::vec::fixed<3> & pos,const int & f) const;
  




  /**
  Returns the partial derivative of the parametrization of the Xe dyadic vector
  with respect to the coordinates of the edges points and adjacent facets points
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialEePartialAe (10x24)
  */
  arma::mat::fixed<10,24> PartialXePartialBe(const arma::vec::fixed<3> & pos,const int & e) const;
  
  /**
  Returns the partial derivative of the edge length le
  with respect to the coordinates of the edges points
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialEePartialAe (10x24)
  */
  arma::rowvec::fixed<6> PartialEdgeLengthPartialAe(const arma::vec::fixed<3> & pos,const int & e) const;
  
  /**
  Returns the partial derivative of the (q,r) component of the Ee dyad with respect to the 
  with respect to the coordinates of the edges points and adjacent facets points
  @param pos position where to evaluate the partial
  @param e edge index
  @param q row index
  @param r col index
  @return PartialEqrPartialBe (1x24)
  */
  arma::rowvec::fixed<24> PartialEqrPartialBe(const arma::vec::fixed<3> & pos,const int & e,const int & q,const int & r) const;

  /**
  Returns the partial derivative of the Ee dyad parametrization with respect 
  to the coordinates of the edges points and adjacent facets points
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialEqrPartialBe (6x24)
  */
  arma::mat::fixed<6,24> PartialEPartialBe(const arma::vec::fixed<3> & pos,const int & e,const int & q,const int & r) const;




  /**
  Runs a finite-differencing based test of the implemented PGM partials
  @param tol relative tolerance
  */
  void TestPartials(double tol = 1e-2) const;


protected:




  void TestPartialUePartialXe(double tol) const;
  void TestPartialUfPartialXf(double tol) const;
  void TestPartialXfPartialTf(double tol) const;
  void TestPartialOmegafPartialTf(double tol) const;
  void TestPartialFfPartialTf(double tol) const;
  void TestPartialNormalizedVPartialNonNormalizedV(double tol) const;
  void TestPartialNfPartialTf(double tol) const;
  void TestPartialFfPartialnf(double tol) const;
  void TestPartialLePartialAe(double tol) const;
  void TestPartialRadiusEePartialAe(double tol) const;
  void TestPartialEePartialAe(double tol) const;
  void TestPartialEePartialTf(double tol) const;
  void TestPartialXePartialBe(double tol) const;
  void TestPartialEdgeLengthPartialAe(double tol) const;
  void TestPartialEqrPartialBe(double tol) const;
  void TestPartialEPartialBe(double tol) const;



  vtkSmartPointer<SBGATPolyhedronGravityModel> pgm_model;


};

#endif


