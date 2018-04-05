%module CoinLedger

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
