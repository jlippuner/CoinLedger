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
#include "importers/MiningPoolHub.hpp"
#include "importers/NiceHash.hpp"
#include "importers/XRPAccount.hpp"

#include "prices/DailyData.hpp"
#include "prices/PriceSource.hpp"

#include "taxes/Inventory.hpp"
#include "taxes/Taxes.hpp"
%}

%template(vec_str) std::vector<std::string>;
%template(vec_vec_str) std::vector<std::vector<std::string>>;
%template(map_str_str) std::map<std::string, std::string>;

%ignore std::vector<ProtoSplit>::vector(size_type);
%ignore std::vector<ProtoSplit>::resize(size_type);
%template(vec_ProtoSplit) std::vector<ProtoSplit>;

%include "Account.hpp"
%include "Amount.hpp"
%include "Balance.hpp"
%include "Coin.hpp"
%ignore operator<;
%include "Datetime.hpp"
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
%include "importers/MiningPoolHub.hpp"
%include "importers/NiceHash.hpp"
%include "importers/XRPAccount.hpp"

%include "prices/DailyData.hpp"
%include "prices/PriceSource.hpp"

%include "taxes/Inventory.hpp"
%include "taxes/Taxes.hpp"

%template(Amount) FixedPoint10<20>;
