#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace Bithumb {
	enum class TransactionType {
		BID, ASK
	};

	class Order {
	public:
		Order(TransactionType type, map<string, string> data);
		
		TransactionType type;
		double unit;
		int price;
	};
	
	class Transaction {
	public:
		Transaction(map<string, string> data);
		
		string date;
		TransactionType type;
		double unit;
		int price;
		double total;
	};
	
	map<string, vector<Order>> getOrderBook(int count, string currency);
	vector<Transaction> getTransactionHistory(int count, string currency);
}