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
#include <vector>

#include "Datetime.hpp"
#include "Split.hpp"
#include "UUID.hpp"

class File;
class Split;

class Transaction {
public:
  static Transaction* Create(File* file, Datetime date, std::string description,
      const std::vector<ProtoSplit>& protoSplits, std::string import_id = "");

  uuid_t Id() const { return id_; }
  Datetime Date() const { return date_; }
  const std::string& Description() const { return description_; }
  const std::string& Import_id() const { return import_id_; }
  const std::vector<Split*>& Splits() const { return splits_; }

  void AddSplit(Split * split) {
    splits_.push_back(split);
  }

  // return true if the transaction has matched splits, i.e. there is a positive
  // and a negative split
  bool Matched() const;

private:
  friend class File;

  Transaction(uuid_t id, Datetime date, std::string description,
      std::string import_id) :
      id_(id),
      date_(date),
      description_(description),
      import_id_(import_id) {}

  // unique global identifier of this transaction
  const uuid_t id_;

  // the date of this transaction
  Datetime date_;

  // description of this transaction
  std::string description_;

  // if this transaction is imported from an external source, an import ID can
  // be stored here in order to avoid duplicate imports
  std::string import_id_;

  // the splits that make up this transaction
  std::vector<Split*> splits_;
};

#endif // SRC_TRANSACTION_HPP_
