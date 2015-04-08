/*
  core/engine/mpi/value.hpp - insilico engine Parallel value API header and source

  Copyright (C) 2015 Pranav Kulkarni, Collins Assisi Lab, IISER, Pune <pranavcode@gmail.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INCLUDED_INSILICO_CORE_ENGINE_MPI_VALUE_HPP
#define INCLUDED_INSILICO_CORE_ENGINE_MPI_VALUE_HPP

#include "core/configuration/mpi/error.hpp"
#include "core/engine/data.hpp"
#include "core/type.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace insilico { namespace engine {

auto neuron_value(unsigned _id, std::string _variable) -> double {
  char compute_key[128];
  std::string key;
  double observed_value = 0;
  FILE* fptr;
  sprintf(compute_key, "n%d%s", _id, _variable.c_str());
  key = ".ids/.";
  key += compute_key;
  fptr = fopen(key.c_str(), "rb");
  if(fptr == NULL) {
    std::cerr << "[insilico::engine::neuron_value] Failed to find "<<_variable
              << " value for neuron " << _id << ".\n";
    configuration::mpi::severe_error();
  }
  fread(&observed_value, sizeof(double), 1, fptr);
  fclose(fptr);
  return observed_value;
}

auto synapse_value(unsigned _id, std::string _variable) -> double {
  char compute_key[128];
  std::string key;
  double observed_value = 0;
  FILE* fptr;
  sprintf(compute_key, "s%d%s", _id, _variable.c_str());
  key = ".ids/.";
  key += compute_key;
  fptr = fopen(key.c_str(), "rb");
  if(fptr == NULL) {
    std::cerr << "[insilico::engine::synapse_index] Failed to find " << _variable
              << " value for synapse " << _id << ".\n";
    configuration::mpi::severe_error();
  }
  fread(&observed_value, sizeof(double), 1, fptr);
  fclose(fptr);
  return observed_value;
}

auto neuron_value(unsigned _id, std::string _variable, bool& error) -> double {
  char compute_key[128];
  std::string key;
  double observed_value = 0;
  FILE* fptr;
  sprintf(compute_key, "n%d%s", _id, _variable.c_str());
  key = ".ids/.";
  key += compute_key;
  fptr = fopen(key.c_str(), "rb");
  if(fptr == NULL) {
    error = true;
    return observed_value;
  }
  fread(&observed_value, sizeof(double), 1, fptr);
  fclose(fptr);
  return observed_value;
}

auto neuron_value(unsigned _id, std::string _variable, double value) -> void {
  char compute_key[128];
  std::string key;
  FILE* fptr;
  sprintf(compute_key, "n%d%s", _id, _variable.c_str());
  key = ".ids/.";
  key += compute_key;
  fptr = fopen(key.c_str(), "wb");
  fwrite(&value, sizeof(double), 1, fptr);
  fclose(fptr);
}

auto synapse_value(unsigned _id, std::string _variable, bool& error) -> double {
  char compute_key[128];
  std::string key;
  double observed_value = 0;
  FILE* fptr;
  sprintf(compute_key, "s%d%s", _id, _variable.c_str());
  key = ".ids/.";
  key += compute_key;
  fptr = fopen(key.c_str(), "rb");
  if(fptr == NULL) {
    error = true;
    return observed_value;
  }
  fread(&observed_value, sizeof(double), 1, fptr);
  fclose(fptr);
  return observed_value;
}

auto synapse_value(unsigned _id, std::string _variable, double value) -> void {
  char compute_key[128];
  std::string key;
  FILE* fptr;
  sprintf(compute_key, "s%d%s", _id, _variable.c_str());
  key = ".ids/.";
  key += compute_key;
  fptr = fopen(key.c_str(), "wb");
  fwrite(&value, sizeof(double), 1, fptr);
  fclose(fptr);
}

auto get_neuron_values(std::string _variable) -> std::vector< double > {
  bool error = false;
  std::vector< double > values;
  unsigned total_neurons = neuron_count();
  for(unsigned index = 0; index < total_neurons; ++index) {
    double value = neuron_value(index, _variable, error);
    if(!error) { values.push_back(value); }
    error = false;
  }
  return values;
}

auto get_synapse_values(std::string _variable) -> std::vector< double > {
  bool error = false;
  std::vector< double > values;
  unsigned total_synapses = synapse_count();
  for(unsigned index = 0; index < total_synapses; ++index) {
    double value = synapse_value(index, _variable, error);
    if(!error) { values.push_back(value); }
    error = false;
  }
  return values;
}

auto get_values(std::string _variable) -> std::vector< double > {
  std::vector< double > values;
  std::vector< double > neuron_values = get_neuron_values(_variable);
  std::vector< double > synapse_values = get_synapse_values(_variable);
  values.insert(values.end(), neuron_values.begin(), neuron_values.end());
  values.insert(values.end(), synapse_values.begin(), synapse_values.end());
  return values;
}

auto get_pre_neuron_values(unsigned _id, std::string _variable) -> std::vector<double> {
  std::vector<double> values;
  if(!pre_synaptic_lists.empty()) {
    for(std::vector<double>::size_type index = 0; index < pre_synaptic_lists[_id].size(); ++index) {
      values.push_back(synapse_value(pre_synaptic_lists[_id][index], _variable));
    }
  }
  return values;
}

auto neuron_value_key(unsigned _id, std::string _variable) -> decltype(std::string()) {
  char key[128];
  sprintf(key, "n%d%s", _id, _variable.c_str());
  if(value_map.find(key) != value_map.end()) { return key; }
  else { return ""; }
}

auto synapse_value_key(unsigned _id, std::string _variable) -> decltype(std::string()) {
  char key[128];
  sprintf(key, "s%d%s", _id, _variable.c_str());
  if(value_map.find(key) != value_map.end()) { return key; }
  else { return ""; }
}

auto get_neuron_value_keys(std::string _variable) -> std::vector< std::string > {
  std::vector< std::string > value_keys;
  std::string key;
  unsigned total_neurons = neuron_count();
  for(unsigned id = 0; id < total_neurons; ++id) {
    key = neuron_value_key(id, _variable);
    if(key.compare("") != 0) { value_keys.push_back(key); }
  }
  return value_keys;
}

auto get_synapse_value_keys(std::string _variable) -> std::vector< std::string > {
  std::vector< std::string > value_keys;
  std::string key;
  unsigned total_synapses = synapse_count();
  for(unsigned id = 0; id < total_synapses; ++id) {
    key = synapse_value_key(id, _variable);
    if(key.compare("") != 0) { value_keys.push_back(key); }
  }
  return value_keys;
}

auto get_value_keys(std::string _variable) -> std::vector< std::string > {
  std::vector< std::string > value_keys;
  std::vector< std::string > neuron_value_keys = get_neuron_value_keys(_variable);
  std::vector< std::string > synapse_value_keys = get_synapse_value_keys(_variable);
  value_keys.insert(value_keys.end(), neuron_value_keys.begin(), neuron_value_keys.end());
  value_keys.insert(value_keys.end(), synapse_value_keys.begin(), synapse_value_keys.end());
  return value_keys;
}

} } // namespace insilico::engine

#endif