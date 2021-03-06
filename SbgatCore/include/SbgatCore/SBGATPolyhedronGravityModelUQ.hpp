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
  and instantiates some containers whose size is depending upon
  the number of vertices in the pgm's input shape
  @param[in] pgm pointer to valid polyhedron gravity model
  */
  void SetPGM(vtkSmartPointer<SBGATPolyhedronGravityModel> pgm);

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
  @return PGM potential variance evaluated at the queried point (m ^ 4/ s ^4)
  */
  double GetVariancePotential(double const * point) const;

  /**
  Evaluates the Polyhedron Gravity Model potential variance at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame as
  the polydata
  @return PGM potential variance evaluated at the queried point (m ^ 4 / s ^4)
  */
  double GetVariancePotential(const arma::vec::fixed<3> & point) const;

  /**
  Evaluates the Polyhedron Gravity Model potential variance and acceleration covariance at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame as
  the polydata used to construct the PGM
  @param[out] potential_var PGM potential variance evaluated at the queried point (m ^ 4 / s ^4)
  @param[out] acc_cov PGM acceleration covariance evaluated at the queried point (m^2 / s ^4)
  */
  void GetVariancePotentialAccelerationCovariance(double const * point,double & potential_var, 
    arma::mat::fixed<3,3> & acc_cov) const;

  /**
  Evaluates the Polyhedron Gravity Model potential variance and acceleration covariance at the specified point assuming 
  a constant density
  @param point coordinates of queried point, expressed in the same frame as
  the polydata used to construct the PGM
  @param[out] potential_var PGM potential variance evaluated at the queried point (m ^ 4 / s ^4)
  @param[out] acc_cov PGM acceleration covariance evaluated at the queried point (m^2 / s ^4)
  */
  void GetVariancePotentialAccelerationCovariance(const arma::vec::fixed<3> & point,double & potential_var, 
    arma::mat::fixed<3,3> & acc_cov) const;

  /**
  Returns a square root of the covariance matrix using a cholesky decomposition.
  The covariance square root is expressed in the original shape's unit squared (that is, 
  meters or kilometers)
  @return covariance square root
  */
  arma::mat GetCovarianceSquareRoot() const;

  /**
  Runs a finite-differencing based test of the implemented PGM partials
  @param input path to obj file used to test the partials
  @param tol relative tolerance
  */
  static void TestPartials(std::string input , double tol = 1e-2);

  /**
  Obtain the partial derivative of the potential at the prescribed location
  due to a infinitesimal variation in the shape's control points
  @param pos position where to evaluate the partial derivative
  @return partial derivative of the potential with respect to the variation in the shape's control points
  */

  arma::rowvec GetPartialUPartialC(const arma::vec::fixed<3> & pos) const;

  /**
  Obtain the partial derivative of the acceleration at the prescribed location
  due to a infinitesimal variation in the shape's control points
  @param pos position where to evaluate the partial derivative
  @return partial derivative of the acceleration with respect to the variation in the shape's control points
  */
  arma::mat GetPartialAPartialC(const arma::vec::fixed<3> & pos) const;


  /**
  Sets the block P_Cv0_Cv1 in the total shape covariance to the prescribed value P. 
  When v0 != v1, this function must be called twice to set the two symmetric blocks
  on both sides of the diagonal. The covariance is expressed in the original shape's unit squared (that is, 
  meters squared or kilometers squared)
  @param P covariance/correlation of Cv0 and Cv1
  @param v0 index of first vertex
  @param v1 index of second vertex
  */
  void SetCovarianceComponent(const arma::mat::fixed<3,3> & P,const int & v0, const int & v1);

  /**
  Applies prescribed deviation to all the N_C control points and updates pgm
  @param delta_C deviation (3 * N_C x 1)
  */  
  void ApplyDeviation(const arma::vec & delta_C);


  /**
  Get covariance in acceleration arising from the uncertain shape
  @param point coordinates where to evaluate the covariance
  @return covariance of acceleration
  */
  arma::mat::fixed<3,3> GetCovarianceAcceleration(double const * point) const;

   /**
  Get covariance in acceleration arising from the uncertain shape
  @param point coordinates where to evaluate the covariance
  @return covariance of acceleration
  */
  arma::mat::fixed<3,3> GetCovarianceAcceleration(const arma::vec::fixed<3> & point) const;

  vtkSmartPointer<SBGATPolyhedronGravityModel> GetPGMModel() const {return this -> pgm_model;}

protected:

  arma::vec GetBe() const;


  /**
  Adds to the properly initialized vector the partial derivative of the sum of all Ue
  @param[in] pos position where to evaluate the partials
  @param[out] partial partial derivative being evaluated
  */
  void AddPartialSumUePartialC(const arma::vec::fixed<3> & pos,arma::rowvec & partial) const;

   /**
  Add to the properly initialized vector the partial derivative of the sum of all Uf
  @param[in] pos position where to evaluate the partials
  @param[out] partial partial derivative being evaluated
  */
  void AddPartialSumUfPartialC(const arma::vec::fixed<3> & pos,arma::rowvec & partial) const;


  /**
  Adds to the properly initialized vector the partial derivative of the sum of all Acce
  @param[in] pos position where to evaluate the partials
  @param[out] partial partial derivative being evaluated
  */
  void AddPartialSumAccePartialC(const arma::vec::fixed<3> & pos,arma::mat & partial) const;

   /**
  Add to the properly initialized vector the partial derivative of the sum of all Accf
  @param[in] pos position where to evaluate the partials
  @param[out] partial partial derivative being evaluated
  */
  void AddPartialSumAccfPartialC(const arma::vec::fixed<3> & pos,arma::mat & partial) const;



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
  Returns the partial derivative of an individual edge contribution to the acceleration (Acce) 
  with respect to the Xe^E vector holding the e-th edge dyadic factors
  @param pos position where to evaluate the partial
  @param e edge index
  @return PartialAccePartialXe (3x10)
  */
  arma::mat::fixed<3,10> PartialAccePartialXe(const arma::vec::fixed<3> & pos,const int & e) const;

  
  /**
  Returns the partial derivative of an individual facet contribution to the acceleration (Accf) 
  with respect to the Xf^F vector holding the f-th facet dyadic factors
  @param pos position where to evaluate the partial
  @param f facet index
  @return PartialAccfPartialXf (3x10)
  */
  arma::mat::fixed<3,10> PartialAccfPartialXf(const arma::vec::fixed<3> & pos,const int & f) const;



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
  @param nf facet normal
  @return PartialFfPartialnf (6x3)

  */
  static arma::mat::fixed<6,3> PartialFfPartialnf(const arma::vec::fixed<3> & nf);



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
  Returns the connectivity table associated with vector Be
  @param e edge index
  @return connectivity table
  */
  arma::sp_mat  PartialBePartialC(const int & e) const;


  /**
  Returns the connectivity table associated with vector Tf
  @param f facet index
  @return connectivity table
  */
  arma::sp_mat  PartialTfPartialC(const int & f) const;

  /**
  Given a prescribed global deviation of all of the shape's N control points,
  applies it and returns the deviation in each of the Be's vector (one per edge in the shape)
  @param delta deviation in all of the shape's N control points (3 x N_vertices)
  @return deviation in all of the shape's Be vectors (24 x N_edges)
  */
  arma::vec ApplyAndGetBeDeviation(const arma::vec & delta);


  static void TestPartialUePartialXe(std::string input,double tol) ;
  static void TestPartialUfPartialXf(std::string input,double tol) ;
  static void TestPartialXfPartialTf(std::string input,double tol) ;
  static void TestPartialOmegafPartialTf(std::string input,double tol) ;
  static void TestPartialZfPartialUnitRf(std::string input,double tol);
  static void TestPartialFfPartialTf(std::string input,double tol) ;
  static void TestPartialNormalizedVPartialNonNormalizedV(std::string input,double tol) ;
  static void TestPartialAtan2PartialZf(std::string input,double tol);
  static void TestPartialNfPartialTf(std::string input,double tol) ;
  static void TestPartialFfPartialnf(std::string input,double tol) ;
  static void TestPartialFfPartialNonNormalizedNf(std::string input,double tol);
  static void TestPartialLePartialAe(std::string input,double tol) ;
  static void TestPartialEePartialAe(std::string input,double tol) ;
  static void TestPartialEePartialTf(std::string input,double tol) ;
  static void TestPartialXePartialBe(std::string input,double tol) ;
  static void TestPartialEdgeLengthPartialAe(std::string input,double tol) ;
  static void TestPartialEPartialBe(std::string input,double tol) ;
  static void TestPartialUfPartialTf(std::string input,double tol);
  static void TestPartialUePartialBe(std::string input,double tol);
  static void TestPartialUPartialC(std::string input,double tol);
  static void TestPartialAPartialC(std::string input,double tol);
  static void TestPartialUfPartialC(std::string input,double tol);
  static void TestPartialUePartialC(std::string input,double tol);
  static void TestAddPartialSumUePartialC(std::string input,double tol);
  static void TestAddPartialSumUfPartialC(std::string input,double tol);
  static void TestAddPartialSumAccfPartialC(std::string input,double tol);
  static void TestAddPartialSumAccePartialC(std::string input,double tol);
  static void TestPartialBePartialC(std::string input,double tol);







  vtkSmartPointer<SBGATPolyhedronGravityModel> pgm_model;

  arma::mat P_CC;
  arma::sp_mat P_CC_sparse;



};

#endif


