%module CoinLedger

%include <stdint.i>
%include <std_map.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_shared_ptr.i>

%ignore uuid_t::hash;
%ignore operator<<;

%shared_ptr(Account)
%shared_ptr(Coin)
%shared_ptr(Split)
%shared_ptr(Transaction)

%{
#include "Account.hpp"
#include "Amount.hpp"
#include "Balance.hpp"
#include "Coin.hpp"
#include "Datetime.hpp"
#include "PriceSource.hpp"
#include "Split.hpp"
#include "Transaction.hpp"
#include "UUID.hpp"
#include "File.hpp"

#include "importers/Binance.hpp"
#include "importers/Bittrex.hpp"
#include "importers/CoreWallet.hpp"
#include "importers/CSV.hpp"
#include "importers/ETHAccount.hpp"
#include "importers/ElectrumWallet.hpp"
#include "importers/GDAX.hpp"
#include "importers/Kraken.hpp"
#include "importers/XRPAccount.hpp"
%}

%template(vec_str) std::vector<std::string>;
%template(vec_vec_str) std::vector<std::vector<std::string>>;
%template(map_str_str) std::map<std::string, std::string>;

%include "Account.hpp"
%include "Amount.hpp"
%include "Balance.hpp"
%include "Coin.hpp"
%include "Datetime.hpp"
%include "PriceSource.hpp"
%include "Split.hpp"
%include "Transaction.hpp"
%include "UUID.hpp"
%include "File.hpp"

%include "importers/Binance.hpp"
%include "importers/Bittrex.hpp"
%include "importers/CoreWallet.hpp"
%ignore CSV::operator[];
%include "importers/CSV.hpp"
%include "importers/ETHAccount.hpp"
%include "importers/ElectrumWallet.hpp"
%include "importers/GDAX.hpp"
%include "importers/Kraken.hpp"
%include "importers/XRPAccount.hpp"

%template(Amount) FixedPoint10<20>;
