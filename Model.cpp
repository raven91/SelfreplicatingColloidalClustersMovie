//
// Created by Nikita Kruk on 01.03.20.
//

#include "Model.hpp"

#include <string>
#include <sstream>
#include <cassert>
#include <iostream>

Model::Model() :
    parameters_dictionary_(),
    number_of_state_variables_(6),
    colloidal_particles_(),
    solvent_particles_(),
    colloidal_particle_subtypes_(),
    new_periodic_subtype_(0)
{
  std::string folder("/Users/nikita/Documents/Projects/SelfreplicatingColloidalClusters/");
  std::ifstream parameter_file(folder + "simulation_parameters.txt", std::ios::in);
  assert(parameter_file.is_open());
  std::string key("");
  Real value = 0.0;
  while (parameter_file >> key >> value)
  {
    parameters_dictionary_[key] = value;
  }
  parameter_file.close();
  colloidal_particles_.resize(
      number_of_state_variables_ * int(parameters_dictionary_["number_of_colloidal_particles"]));
  solvent_particles_.resize(number_of_state_variables_ * int(parameters_dictionary_["number_of_solvent_particles"]));
  colloidal_particle_subtypes_.resize(int(parameters_dictionary_["number_of_colloidal_particles"]));

  data_file_for_colloidal_particles_.open(folder + "colloidal_particles.bin",
                                          std::ios::in | std::ios::binary);
  assert(data_file_for_colloidal_particles_.is_open());

  data_file_for_solvent_particles_.open(folder + "solvent_particles.bin", std::ios::in | std::ios::binary);
  assert(data_file_for_solvent_particles_.is_open());

  file_with_particle_subtypes_.open(folder + "particle_subtypes.bin", std::ios::in | std::ios::binary);
  assert(file_with_particle_subtypes_.is_open());
}

Model::~Model()
{
  if (data_file_for_colloidal_particles_.is_open())
  {
    data_file_for_colloidal_particles_.close();
  }
  if (data_file_for_solvent_particles_.is_open())
  {
    data_file_for_solvent_particles_.close();
  }
  if (file_with_particle_subtypes_.is_open())
  {
    file_with_particle_subtypes_.close();
  }
  parameters_dictionary_.clear();
  colloidal_particles_.clear();
  solvent_particles_.clear();
  colloidal_particle_subtypes_.clear();
}

void Model::ReadNewState(Real &t)
{
  static std::vector<Real>
      cvec(parameters_dictionary_["number_of_colloidal_particles"] * number_of_state_variables_, 0.0f);
  data_file_for_colloidal_particles_.read((char *) &t, sizeof(Real));
  data_file_for_colloidal_particles_.read((char *) &cvec[0],
                                          parameters_dictionary_["number_of_colloidal_particles"]
                                              * number_of_state_variables_ * sizeof(Real));
  std::copy(cvec.begin(), cvec.end(), colloidal_particles_.begin());
  std::cout << "t:" << t << std::endl;
  static std::vector<Real>
      svec(parameters_dictionary_["number_of_solvent_particles"] * number_of_state_variables_, 0.0f);
  data_file_for_solvent_particles_.read((char *) &t, sizeof(Real));
  data_file_for_solvent_particles_.read((char *) &svec[0],
                                        parameters_dictionary_["number_of_solvent_particles"]
                                            * number_of_state_variables_ * sizeof(Real));
  std::copy(svec.begin(), svec.end(), solvent_particles_.begin());

  file_with_particle_subtypes_.read((char *) &t, sizeof(Real));
  file_with_particle_subtypes_.read((char *) &colloidal_particle_subtypes_[0],
                                    parameters_dictionary_["number_of_colloidal_particles"] * sizeof(int));
}

void Model::SkipTimeUnits(int t, Real delta_t)
{
  data_file_for_colloidal_particles_.seekg(
      int(t / delta_t) * (1l + parameters_dictionary_["number_of_colloidal_particles"] * number_of_state_variables_)
          * sizeof(Real), std::ios::cur);
  data_file_for_solvent_particles_.seekg(
      int(t / delta_t) * (1l + parameters_dictionary_["number_of_solvent_particles"] * number_of_state_variables_)
          * sizeof(Real), std::ios::cur);
  file_with_particle_subtypes_.seekg(
      int(t / delta_t) * (sizeof(Real) + parameters_dictionary_["number_of_colloidal_particles"] * sizeof(int)),
      std::ios::cur);
}

std::vector<Real> &Model::GetColloidalParticles()
{
  return colloidal_particles_;
}

const std::vector<Real> &Model::GetSolventParticles()
{
  return solvent_particles_;
}

std::vector<int> &Model::GetColloidalParticleSubtypes()
{
  return colloidal_particle_subtypes_;
}

void Model::SetColloidalParticleSubtype(int index, ParticleSubtype subtype)
{
  if (index < colloidal_particle_subtypes_.size())
  {
    colloidal_particle_subtypes_[index] = static_cast<int>(subtype);
  } else
  {
    std::cout << "Model::SetColloidalParticleSubtype: out of bounds index" << std::endl;
  }
}

int Model::GetNumberOfColloidalParticles() const
{
  return parameters_dictionary_.at("number_of_colloidal_particles");
}

int Model::GetNumberOfSolventParticles() const
{
  return parameters_dictionary_.at("number_of_solvent_particles");
}

int Model::GetNumberOfAllParticles() const
{
  return GetNumberOfColloidalParticles() + GetNumberOfSolventParticles();
}

int Model::GetNumberOfStateVariables() const
{
  return number_of_state_variables_;
}

Real Model::GetColloidDiameter() const
{
  return parameters_dictionary_.at("colloid_diameter");
}

Real Model::GetSolventDiameter() const
{
  return parameters_dictionary_.at("solvent_diameter");
}

Real Model::GetXSize() const
{
  return parameters_dictionary_.at("x_size");
}

void Model::SetXSize(Real x_size)
{
  parameters_dictionary_["x_size"] = x_size;
}

Real Model::GetYSize() const
{
  return parameters_dictionary_.at("y_size");
}

void Model::SetYSize(Real y_size)
{
  parameters_dictionary_["y_size"] = y_size;
}

Real Model::GetZSize() const
{
  return parameters_dictionary_.at("z_size");
}

void Model::SetZSize(Real z_size)
{
  parameters_dictionary_["z_size"] = z_size;
}

void Model::AddNewColloidalParticle(Real x, Real y, Real z, int subtype)
{
  colloidal_particles_.push_back(x);
  colloidal_particles_.push_back(y);
  colloidal_particles_.push_back(z);
  colloidal_particles_.push_back(0.0f); // v_x
  colloidal_particles_.push_back(0.0f); // v_y
  colloidal_particles_.push_back(0.0f); // v_z
  colloidal_particle_subtypes_.push_back(subtype);
  ++parameters_dictionary_["number_of_colloidal_particles"];
}

ParticleSubtype Model::GetNewPeriodicSubtype()
{
  ParticleSubtype new_subtype = static_cast<ParticleSubtype>(new_periodic_subtype_);
  new_periodic_subtype_ = (new_periodic_subtype_ + 1) % kNumberOfPeriodicColloidSubtypes;
  return new_subtype;
}

void Model::ApplyPeriodicBoundaryConditions(Real box_size)
{
  static const float x_size = box_size, y_size = box_size, z_size = box_size;
  static const float x_rsize = 1.0f / x_size, y_rsize = 1.0f / y_size, z_rsize = 1.0f / z_size;

#pragma unroll
  for (int i = 0; i < (int) parameters_dictionary_["number_of_colloidal_particles"]; ++i)
  {
    int ii = i * number_of_state_variables_;
    colloidal_particles_[ii] -= std::floorf(colloidal_particles_[ii] * x_rsize) * x_size;
    colloidal_particles_[ii + 1] -= std::floorf(colloidal_particles_[ii + 1] * y_rsize) * y_size;
    colloidal_particles_[ii + 2] -= std::floorf(colloidal_particles_[ii + 2] * z_rsize) * z_size;
  } // i
#pragma unroll
  for (int i = 0; i < (int) parameters_dictionary_["number_of_solvent_particles"]; ++i)
  {
    int ii = i * number_of_state_variables_;
    solvent_particles_[ii] -= std::floorf(solvent_particles_[ii] * x_rsize) * x_size;
    solvent_particles_[ii + 1] -= std::floorf(solvent_particles_[ii + 1] * y_rsize) * y_size;
    solvent_particles_[ii + 2] -= std::floorf(solvent_particles_[ii + 2] * z_rsize) * z_size;
  } // i
}