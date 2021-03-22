//
// Created by Nikita Kruk on 01.03.20.
//

#ifndef SELFREPLICATINGCOLLOIDALCLUSTERSMOVIE_MODEL_HPP
#define SELFREPLICATINGCOLLOIDALCLUSTERSMOVIE_MODEL_HPP

#include "Definitions.hpp"

#include <vector>
#include <fstream>
#include <unordered_map>
#include <string>
#include <map>

const int kNumberOfPeriodicColloidSubtypes = 2;
enum class ParticleSubtype
{
  // the same as in dissipative particle dynamics (DPD) simulations
  kB = 0, kC,
  kA, kBStar, kCStar,
  kAPrime, kBPrime, kCPrime, kBStarPrime, kCStarPrime,
  k0
};

const std::map<ParticleSubtype, ParticleSubtype> kComplementarySubtype =
    {
        {ParticleSubtype::kA, ParticleSubtype::kAPrime},
        {ParticleSubtype::kB, ParticleSubtype::kBPrime},
        {ParticleSubtype::kC, ParticleSubtype::kCPrime},
        {ParticleSubtype::kBStar, ParticleSubtype::kBStarPrime},
        {ParticleSubtype::kCStar, ParticleSubtype::kCStarPrime},
        {ParticleSubtype::kAPrime, ParticleSubtype::kA},
        {ParticleSubtype::kBPrime, ParticleSubtype::kB},
        {ParticleSubtype::kCPrime, ParticleSubtype::kC},
        {ParticleSubtype::kBStarPrime, ParticleSubtype::kBStar},
        {ParticleSubtype::kCStarPrime, ParticleSubtype::kCStar}
    };

class Model
{
 public:

  Model();
  ~Model();

  void ReadNewState(Real &t);
  void SkipTimeUnits(int t, Real delta_t);

  std::vector<Real> &GetColloidalParticles();
  const std::vector<Real> &GetSolventParticles();
  std::vector<int> &GetColloidalParticleSubtypes();
  void SetColloidalParticleSubtype(int index, ParticleSubtype subtype);
  int GetNumberOfColloidalParticles() const;
  int GetNumberOfSolventParticles() const;
  int GetNumberOfAllParticles() const;
  int GetNumberOfStateVariables() const;
  Real GetColloidDiameter() const;
  Real GetSolventDiameter() const;
  Real GetXSize() const;
  void SetXSize(Real x_size);
  Real GetYSize() const;
  void SetYSize(Real y_size);
  Real GetZSize() const;
  void SetZSize(Real z_size);
  void ApplyPeriodicBoundaryConditions(Real box_size);

  void AddNewColloidalParticle(Real x, Real y, Real z, int subtype);
  ParticleSubtype GetNewPeriodicSubtype();
  int new_periodic_subtype_;

 private:

  std::unordered_map<std::string, Real> parameters_dictionary_;
  int number_of_state_variables_;
  std::vector<Real> colloidal_particles_;
  std::vector<Real> solvent_particles_;
  std::vector<int> colloidal_particle_subtypes_;
  std::ifstream data_file_for_colloidal_particles_;
  std::ifstream data_file_for_solvent_particles_;
  std::ifstream file_with_particle_subtypes_;

};

#endif //SELFREPLICATINGCOLLOIDALCLUSTERSMOVIE_MODEL_HPP
