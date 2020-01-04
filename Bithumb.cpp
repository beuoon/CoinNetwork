#include "Bithumb.h"

#include <fstream>
#include "xcoin_api.h"

namespace Bithumb {
	Order::Order(const Value& data) {
		unit = atof(data["quantity"].GetString());
		price = atof(data["price"].GetString());
	}

	Transaction::Transaction(const Value& data) {
		date = data["transaction_date"].GetString();
		type = (!strcmp(data["type"].GetString(), "ask")) ? TransactionType::ASK : TransactionType::BID;
		unit = atof(data["units_traded"].GetString());
		price = atof(data["price"].GetString());
		total = atof(data["total"].GetString());
	}
	
	Account::Account() {
		/* Wallet.secret
		{
			"Connect Key": "..."
			"Secret Key": "..."
		}
		*/
		
		ifstream in(fileName);
		char buffer[5000];
		if (!in.is_open()) return ;
		in.read(buffer, 5000);
		in.close();
		
		Document document;
		document.Parse(buffer);
		if (!document.IsObject()) return ;
		
		connectKey = document["ConnectKey"].GetString();
		secretKey = document["SecretKey"].GetString();
	}
	
	bool getOrderBook(int count, map<string, vector<Order>>& book) {
		string url("/public/orderbook/BTC");
		string post("count=");
		post += to_string(count);
		const char *result = api_request(url.c_str(), post.c_str());
		if (result == NULL) return false;
		
		Document document;
		document.Parse(result);
		if (!document.IsObject()) return false;
		
		const char *status = document["status"].GetString();
		if (strcmp(status, "0000")) return false;
		
		const Value& data = document["data"].GetObject();
		
		for (const auto& order : data["bids"].GetArray())
			book["bid"].push_back(Order(order));
		
		for (const auto& order : data["asks"].GetArray())
			book["ask"].push_back(Order(order));
		
		return true;
	}
	bool getTransactionHistory(int count, vector<Transaction>& history) {
		string url("/public/transaction_history/BTC");
		string post("count=");
		post += to_string(count);
		const char *result = api_request(url.c_str(), post.c_str());
		if (result == NULL) return false;
		
		Document document;
		document.Parse(result);
		if (!document.IsObject()) return false;
		
		const char *status = document["status"].GetString();
		if (strcmp(status, "0000")) return false;
		
		Value data = document["data"].GetArray();
		
		for (const Value& trans : data.GetArray())
			history.push_back(Transaction(trans));
		return true;
	}
	
	
	bool getBalance() {
		string connectKey = Account::getInstance()->getConnectKey();
		string secretKey = Account::getInstance()->getSecretKey();
	}
	bool buyMakretPrice() {
		string connectKey = Account::getInstance()->getConnectKey();
		string secretKey = Account::getInstance()->getSecretKey();
	}
	bool sellMarketPrice() {
		string connectKey = Account::getInstance()->getConnectKey();
		string secretKey = Account::getInstance()->getSecretKey();
	}
}