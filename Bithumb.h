#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

namespace Bithumb {
	enum class TransactionType {
		BID, ASK
	};

	class Order {
	public:
		Order(map<string, string> data);
		
		double unit;
		int price;
	};
	
	class Transaction {
	public:
		Transaction(map<string, string> data);
		
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
	
	ostream& operator<<(ostream& os, const Transaction &tra);
	
	bool getOrderBook(int count, string currency, map<string, vector<Order>>& book);
	bool getTransactionHistory(int count, string currency, vector<Transaction>& history);
}