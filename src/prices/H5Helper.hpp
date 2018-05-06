#ifndef SRC_PRICES_H5HELPER_HPP_
#define SRC_PRICES_H5HELPER_HPP_

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <hdf5/serial/H5Cpp.h>

template <typename T>
H5::DataType GetH5DataType() {
  if (std::is_same<T, float>::value)
    return H5::DataType(H5::PredType::NATIVE_FLOAT);
  if (std::is_same<T, double>::value)
    return H5::DataType(H5::PredType::NATIVE_DOUBLE);
  else if (std::is_same<T, int>::value)
    return H5::DataType(H5::PredType::NATIVE_INT);
  else if (std::is_same<T, int64_t>::value)
    return H5::DataType(H5::PredType::NATIVE_INT64);
  else if (std::is_same<T, char>::value)
    return H5::DataType(H5::PredType::NATIVE_CHAR);
  else if (std::is_same<T, size_t>::value)
    return H5::DataType(H5::PredType::NATIVE_UINT64);
  else if (std::is_same<T, unsigned char>::value)
    return H5::DataType(H5::PredType::NATIVE_UCHAR);
  else
    throw std::invalid_argument("Unknown type");
}

template <typename T>
std::vector<T> ReadDataFromH5File(
    const H5::H5File& H5File, const std::string& name) {
  auto dataset = H5File.openDataSet(name);
  auto dataSpace = dataset.getSpace();

  hsize_t dim[1];
  if (1 != dataSpace.getSimpleExtentDims(dim))
    throw std::runtime_error(
        "Unexpected number of dimensions in data set " + name);

  auto datatype = GetH5DataType<T>();
  if (!(dataset.getDataType() == datatype))
    throw std::runtime_error("Wrong data type in data set " + name);

  // set size of data vector
  std::vector<T> data(dim[0]);
  dataset.read(data.data(), datatype);
  dataset.close();

  return data;
}

template <typename T>
std::vector<std::vector<T>> Read2DDataFromH5File(
    const H5::H5File& H5File, const std::string& name) {
  auto dataset = H5File.openDataSet(name);
  auto dataSpace = dataset.getSpace();

  hsize_t dim[2];
  if (2 != dataSpace.getSimpleExtentDims(dim))
    throw std::runtime_error(
        "Unexpected number of dimensions in data set " + name);

  auto datatype = GetH5DataType<T>();
  if (!std::is_same<T, char>::value) {
    // char is used to read booleans, which are weird in HDF5
    if (!(dataset.getDataType() == datatype))
      throw std::runtime_error("Wrong data type in data set " + name);
  }

  // set size of data
  std::vector<std::vector<T>> data(dim[0], std::vector<T>(dim[1]));

  // we need to arrange the data contiguously
  // we must use new here, otherwise the memory will be allocated on the stack,
  // which is rather small and would lead to segfaults for large datasets
  std::size_t typeSize = sizeof(T);
  char* const rawData = new char[dim[0] * dim[1] * typeSize];

  dataset.read(rawData, datatype);

  for (unsigned int i = 0; i < dim[0]; ++i) {
    memcpy(
        data[i].data(), rawData + (i * dim[1] * typeSize), dim[1] * typeSize);
  }

  dataset.close();
  delete[] rawData;

  return data;
}

template <typename T>
std::vector<T> ReadFlat2DDataFromH5File(
    const H5::H5File& H5File, const std::string& name) {
  auto dataset = H5File.openDataSet(name);
  auto dataSpace = dataset.getSpace();

  hsize_t dim[2];
  if (2 != dataSpace.getSimpleExtentDims(dim))
    throw std::runtime_error(
        "Unexpected number of dimensions in data set " + name);

  if (dim[1] != 1) throw std::runtime_error("2D data is not flat");

  auto datatype = GetH5DataType<T>();
  if (!std::is_same<T, char>::value) {
    // char is used to read booleans, which are weird in HDF5
    if (!(dataset.getDataType() == datatype))
      throw std::runtime_error("Wrong data type in data set " + name);
  }

  // set size of data vector
  std::vector<T> data(dim[0]);
  dataset.read(data.data(), datatype);
  dataset.close();

  return data;
}

std::string GetStringAttribute(
    const H5::H5Location& location, const std::string& name);

#endif  // SRC_PRICES_H5HELPER_HPP_
