#include "H5Helper.hpp"

std::string GetStringAttribute(
    const H5::H5Location& location, const std::string& name) {
  H5::Attribute attr = location.openAttribute(name);

  hsize_t dim[1];
  H5::DataSpace attrSpace = attr.getSpace();
  if (attrSpace.getSimpleExtentNdims() != 1)
    throw std::runtime_error("String attribute space does not have rank 1");

  attrSpace.getSimpleExtentDims(dim);
  if (dim[0] != 1)
    throw std::runtime_error("String attribute space does not have size 1");

  H5::DataType dataType = attr.getDataType();
  H5::StrType strType(H5::PredType::C_S1, H5T_VARIABLE);
  if (!(dataType == strType))
    throw std::runtime_error(
        "String attribute does not have variable string "
        "data type");

  // the space is allocated by HDF5, but we need to free it
  char* charPtr;
  attr.read(dataType, &charPtr);
  attr.close();

  std::string value(charPtr);
  free(charPtr);

  return value;
}
