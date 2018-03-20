/// \file Transaction.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_TRANSACTION_HPP_
#define SRC_TRANSACTION_HPP_

#include <string>

#include <boost/uuid/uuid.hpp>

#include "Datetime.hpp"
#include "Split.hpp"

class Transaction {
public:
  // no constructs that don't create a new id
  Transaction() = delete;
  Transaction(const Transaction&) = delete;
  Transaction& operator=(const Transaction&) = delete;

private:
  // unique global identifier of this transaction
  const boost::uuids::uuid id_;

  // the date of this transaction
  Datetime date_;

  // description of this transaction
  std::string description_;

  // the splits that make up this transaction
  std::vector<Split> splits_;
};

#endif // SRC_TRANSACTION_HPP_
