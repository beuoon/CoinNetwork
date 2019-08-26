#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace Bithumb {
	enum class TransactionType {
		BID, ASK
	};

	class TransactionHistory {
	public:
		TransactionHistory(map<string, string> data);
		
		string date;
		TransactionType type;
		double unit;
		int price;
		double total;
	};

	class API {
	public:
		vector<TransactionHistory> getTransactionHistory(int count, string currency);
	};
}