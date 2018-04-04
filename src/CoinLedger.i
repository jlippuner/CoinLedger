%module CoinLedger

%include <std_string.i>
%include <std_vector.i>

%ignore uuid_t::hash;
%ignore operator<<;

%{
#include "Account.hpp"
#include "Amount.hpp"
#include "Coin.hpp"
#include "Datetime.hpp"
#include "PriceSource.hpp"
#include "Split.hpp"
#include "Transaction.hpp"
#include "UUID.hpp"
#include "File.hpp"
%}

%include "Account.hpp"
%include "Amount.hpp"
%include "Coin.hpp"
%include "Datetime.hpp"
%include "PriceSource.hpp"
%include "Split.hpp"
%include "Transaction.hpp"
%include "UUID.hpp"
%include "File.hpp"
