#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <rapidjson/document.h>

using namespace rapidjson;
using namespace std;

namespace Bithumb {
	enum class TransactionType {
		BID, ASK
	};

	class Order {
	public:
		Order(const Value& data);
		
		double unit;
		int price;
	};
	
	class Transaction {
	public:
		Transaction(const Value& data);
		
		bool operator==(const Transaction& trans) {
			if (date == trans.date &&
			   type == trans.type &&
			   unit == trans.unit &&
			   price == trans.price &&
			   total == trans.total)
				return true;
			return false;
		}
		
		string date;
		TransactionType type;
		double unit;
		int price;
		double total;
	};
	
	bool getOrderBook(int count, string currency, map<string, vector<Order>>& book);
	bool getTransactionHistory(int count, string currency, vector<Transaction>& history);
}