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
  Returns the partial derivative of Z_f = (alpha_f,gamma_f)^T (as in wf = 2 * arctan2(Z_f) )
  with respect to the unit vectors from the field point to the facet vertices
  @param UnitRf 3 unit vectors stacked up
  @return PartialZfPartialUnitRf (2x9)
  */
  static arma::mat::fixed<2,9> PartialZfPartialUnitRf(const arma::vec::fixed<9> & UnitRf);


  /**
  Returns the partial derivative of arctan2(Z_f) w/r to Z_f 
  with respect to the unit vectors from the field point to the facet vertices
  @param Zf 
  @return PartialAtan2PartialZf (1x2)
  */
  static arma::rowvec::fixed<2> PartialAtan2PartialZf(const arma::vec::fixed<2> & Zf);


  /**
  Returns the partial derivative of arctan(y/x)
  @param xy input
  @return partial derivative
  */
  static arma::rowvec::fixed<2> PartialOmegafPartialXY(const arma::vec::fixed<2> & xy);


  /**
  Returns the partial derivative of the facet dyad parametrization (Ff)
  with respect to the vertices coordinates constitutive of the f-th triangle (Tf) 
  @param f facet index
  @return PartialFfPartialTf (6x9)
  */
  arma::mat::fixed<6,9> PartialFfPartialTf(const int & f) const;



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
  @return PartialNfPartialTf (3x9)
  */
  arma::mat::fixed<3,9> PartialNfPartialTf(const int & f) const;



  /**
  Returns the partial derivative of the f-th facet dyad parametrization with respect to the 
  normalized normal of the f-th facet
  @param f facet index
  @return PartialFfPartialnf (6x3)

  */
  arma::mat::fixed<6,3> PartialFfPartialnf(const int & f) const;



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
  @return PartialRadiusEePartialAe (3x6)
  */
  arma::mat::fixed<3,6> PartialRadiusEePartialAe() const;
  

  /**
  Returns the partial derivative of field-point to facet-point vector
  with respect to the coordinates of the three vertices forming the facet (stacked in Tf)
  @return PartialRadiusFfPartialTf (3x9)
  */
  arma::mat::fixed<3,9> PartialRadiusFfPartialTf() const;
  

  /**
  Returns the partial derivative of the parametrization of the Xe dyadic vector
  with respect to the coordinates of the edges points and adjacent facets points
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialXePartialBe (10x24)
  */
  arma::mat::fixed<10,24> PartialXePartialBe(const arma::vec::fixed<3> & pos,const int & e) const;
  
  /**
  Returns the partial derivative of the edge length le
  with respect to the coordinates of the edges points
  @param e edge index
  @return PartialEdgeLengthPartialAe (10x24)
  */
  arma::rowvec::fixed<6> PartialEdgeLengthPartialAe(const int & e) const;
  
  /**
  Returns the partial derivative of the (q,r) component of the Ee dyad with respect to the 
  with respect to the coordinates of the edges points and adjacent facets points
  @param e edge index
  @param q row index
  @param r col index
  @return PartialEqrPartialBe (1x24)
  */
  arma::rowvec::fixed<24> PartialEqrPartialBe(const int & e,const int & q,const int & r) const;

  /**
  Returns the partial derivative of the Ee dyad parametrization with respect 
  to the coordinates of the edges points and adjacent facets points
  @param e edge index
  @return PartialEPartialBe (6x24)
  */
  arma::mat::fixed<6,24> PartialEPartialBe(const int & e) const;





  /**
  Runs a finite-differencing based test of the implemented PGM partials
  @param tol relative tolerance
  */
  static void TestPartials(double tol = 1e-2);


  /**
  Applies deviation to the coordinates of the vertices on the prescribed edge
  and updates the pgm
  @param delta_Ae deviation
  @param e edge index
  */
  void ApplyAeDeviation(arma::vec::fixed<6> delta_Ae,const int & e);


  /**
  Applies deviation to the coordinates of the vertices in the prescribed facet
  and updates the pgm
  @param delta_Tf deviation
  @param f facet index
  */
  void ApplyTfDeviation(arma::vec::fixed<9> delta_Tf,const int & f);













  vtkSmartPointer<SBGATPolyhedronGravityModel> GetPGMModel() const {return this -> pgm_model;}
protected:




  static void TestPartialUePartialXe(double tol) ;
  static void TestPartialUfPartialXf(double tol) ;
  static void TestPartialXfPartialTf(double tol) ;
  static void TestPartialOmegafPartialTf(double tol) ;
  static void TestPartialZfPartialUnitRf(double tol);
  static void TestPartialFfPartialTf(double tol) ;
  static void TestPartialNormalizedVPartialNonNormalizedV(double tol) ;
  static void TestPartialAtan2PartialZf(double tol);
  static void TestPartialNfPartialTf(double tol) ;
  static void TestPartialFfPartialnf(double tol) ;
  static void TestPartialLePartialAe(double tol) ;
  static void TestPartialEePartialAe(double tol) ;
  static void TestPartialEePartialTf(double tol) ;
  static void TestPartialXePartialBe(double tol) ;
  static void TestPartialEdgeLengthPartialAe(double tol) ;
  static void TestPartialEqrPartialBe(double tol) ;
  static void TestPartialEPartialBe(double tol) ;



  vtkSmartPointer<SBGATPolyhedronGravityModel> pgm_model;


};

#endif


