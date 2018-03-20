/// \file Split.hpp
/// \author jlippuner
/// \since Mar 20, 2018
///
/// \brief
///
///

#ifndef SRC_SPLIT_HPP_
#define SRC_SPLIT_HPP_

#include <boost/uuid/uuid.hpp>

#include "Account.hpp"
#include "Amount.hpp"
#include "Coin.hpp"
#include "Transaction.hpp"

class Split {
private:
  // unique global identifier of this split
  const boost::uuids::uuid id_;

  // the transaction with which this split is associated
  const Transaction * const transaction_;

  // the account to or from which the amount is added or subtracted
  const Account * account_;

  // memo of this split
  std::string memo_;

  // the amount of this split, always positive
  Amount amount_;

  // the coin that is credited or debited to the account
  Coin coin_;

  // true if this amount is a credit to account_
  bool credit_;
};

#endif // SRC_SPLIT_HPP_
