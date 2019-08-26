#include "Bithumb.h"

#include "xcoin_api.h"
#include <stack>

using namespace std;

namespace Bithumb {
	string parseStr(string str) {
		int si = str.find('"') + 1;
		int ei = str.rfind('"');
		return str.substr(si, ei-si);
	}
	int parseInt(string str) {
		int si = str.find('"') + 1;
		int ei = str.rfind('"');
		return atoi(str.substr(si, ei-si).c_str());
	}
	double parseFloat(string str) {
		int si = str.find('"') + 1;
		int ei = str.rfind('"');
		return atof(str.substr(si, ei-si).c_str());
	}
	
	vector<string> parseList(string str) {
		vector<string> elements;
		
		stack<char> bracketStack;
		int i = 0, len = str.length()-1;
		int si = 1;
		while (++i < len) {
			if (str[i] == ',' && bracketStack.empty()) {
				elements.push_back(str.substr(si, i-si));
				si = i+1;
			}
			
			if (str[i] == '[' || str[i] == '{')
				bracketStack.push(str[i]);
			if (str[i] == ']' && bracketStack.top() == '[' ||
				str[i] == '}' && bracketStack.top() == '{' )
				bracketStack.pop();
		}
		elements.push_back(str.substr(si, i-si));
		
		return elements;
	}
	std::map<string, string> parseStruct(string str) {
		vector<string> elements = parseList(str);
		
		std::map<string, string> structMap;
		for (int i = 0; i < elements.size(); i++) {
			string element = elements[i];
			int index = element.find(':');
			string key = parseStr(element.substr(0, index));
			string value = element.substr(index+1);
			
			structMap[key] = value;
		}
		
		return structMap;
	}

	TransactionHistory::TransactionHistory(map<string, string> data) {
		date = parseStr(data["transaction_date"]);
		type = (parseStr(data["type"]) == "bid") ? TransactionType::BID : TransactionType::ASK;
		unit = parseFloat(data["units_traded"]);
		price = parseInt(data["price"]);
		total = parseFloat(data["total"]);
	}
	
	vector<TransactionHistory> API::getTransactionHistory(int count, string currency) {
		string url("/public/transaction_history?count=");
		string post("currency=");
		url += to_string(count);
		post += currency;
		string result = api_request(url.c_str(), post.c_str());
		
		std::map<string, string> resultMap = parseStruct(result);
		
		vector<TransactionHistory> histories;
		int status = parseInt(resultMap["status"]);
		if (status != 0) return histories;
		
		vector<string> data = parseList(resultMap["data"]);
		for (int i = 0; i < data.size(); i++)
			histories.push_back(TransactionHistory(parseStruct(data[i])));
		
		return histories;
	}
}