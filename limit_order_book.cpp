#include<bits/stdc++.h>
using namespace std;

vector<string> split_string(string s, char c) {
    // Split the string s on every position equal to char c
    string cur;
    vector<string> ans;
    for(int i = 0; i < (int)s.size(); i++) {
        if(s[i] == c) {
            if(!cur.empty()) {
                ans.push_back(cur);
            }
            cur = "";
        }
        else {
            cur += s[i];
        }
    }
    if(!cur.empty()) {
        ans.push_back(cur);
    }
    return ans;
}

class FeedHandler {
public:
    FeedHandler() {};
    // Assumption:- order_id is increasing with time, will throw an error message if not so.
    set<array<int, 3>> buy_orders; // <-price, order_id, quantity>
    set<array<int, 3>> sell_orders; // <price, order_id, quantity>
    map<int, int> volume; // volume of shares traded per price
    int max_order_id = -1;
    
    void processMessage(const std::string &line, ostream &os) {
        auto res = split_string(line, ',');
        if((int)res.size() != 5) {
            throw invalid_argument("Incorrect input given! Not processing it.");
        }
        if(res[0] != "A" and res[0] != "X" and res[0] != "M") {
            throw invalid_argument("Incorrect order type given!");
        }
        char order_type = res[0][0];
        int order_id = stoi(res[1]);
        if(order_id <= 0) {
            throw invalid_argument("Order ID should be a positive integer.");
        }
        if(order_type == 'A') {
            if(order_id <= max_order_id) {
                throw invalid_argument("Order ID is not increasing with time for new orders.");
            }
            max_order_id = max(max_order_id, order_id);
        }
        if(res[2] != "S" and res[2] != "B") {
            throw invalid_argument("Incorrect type of order given! Order type should be buy or sell.");
        }
        bool is_sell = (res[2] == "S");
        int quantity = stoi(res[3]);
        if(quantity <= 0) {
            throw invalid_argument("Quantity of buy/sell order should be a positive integer.");
        }
        int price = stoi(res[4]);
        if(price < 0) {
            throw invalid_argument("Price should be non-negative integer.");
        }

        if(order_type == 'A') {
            if(is_sell) {
                // Add sell order
                if(buy_orders.empty()) {
                    sell_orders.insert({price, order_id, quantity});
                }
                else {
                    auto best_buy = *buy_orders.begin();
                    if(price > -best_buy[0]) {
                        // Sell price is higher than the best bid
                        sell_orders.insert({price, order_id, quantity});
                    }
                    else {
                        // Sell price <= best bid
                        while(quantity > 0 and !buy_orders.empty()) {
                            best_buy = *buy_orders.begin();
                            if(price > -best_buy[0]) {
                                break;
                            }
                            if(best_buy[2] > quantity) {
                                // Plenty of buyers
                                os << "T," << quantity << "," << -best_buy[0] << '\n';
                                volume[-best_buy[0]] += quantity;
                                os << "M," << best_buy[1] << ",B," << best_buy[2] - quantity << "," << -best_buy[0] << '\n';
                                buy_orders.erase(buy_orders.begin());
                                buy_orders.insert({best_buy[0], best_buy[1], best_buy[2] - quantity});
                                quantity = 0;
                            }
                            else if(best_buy[2] == quantity) {
                                // quantity exactly matched
                                os << "T," << quantity << "," << -best_buy[0] << '\n';
                                volume[-best_buy[0]] += quantity;
                                os << "X," << best_buy[1] << ",B," << best_buy[2] << "," << -best_buy[0] << '\n';
                                buy_orders.erase(buy_orders.begin());
                                quantity = 0;
                            }
                            else {
                                os << "T," << best_buy[2] << "," << -best_buy[0] << '\n';
                                volume[-best_buy[0]] += best_buy[2];
                                os << "X," << best_buy[1] << ",B," << best_buy[2] << "," << -best_buy[0] << '\n';
                                buy_orders.erase(buy_orders.begin());
                                quantity -= best_buy[2];
                            }
                        }
                        if(quantity > 0) {
                            sell_orders.insert({price, order_id, quantity});
                            os << "A," << order_id << ",S," << quantity << "," << price << '\n';
                        }
                    }
                }
            }
            else {
                // Add buy order
                if(sell_orders.empty()) {
                    buy_orders.insert({-price, order_id, quantity});    
                }
                else {
                    auto best_sell = *sell_orders.begin();
                    if(price < best_sell[0]) {
                        buy_orders.insert({-price, order_id, quantity});        
                    }
                    else {
                        while(quantity > 0 and !sell_orders.empty()) {
                            best_sell = *sell_orders.begin();
                            if(price < best_sell[0]) {
                                break;
                            }
                            if(best_sell[2] > quantity) {
                                // modify sell_orders
                                os << "T," << quantity << "," << best_sell[0] << '\n';
                                os << "M," << best_sell[1] << ",S," << best_sell[2] - quantity << "," << best_sell[0] << '\n';
                                sell_orders.erase(sell_orders.begin());
                                sell_orders.insert({best_sell[0], best_sell[1], best_sell[2] - quantity});
                                quantity = 0;
                            }
                            else if(best_sell[2] == quantity) {
                                os << "T," << quantity << "," << best_sell[0] << '\n';
                                os << "X," << best_sell[1] << ",S," << quantity << "," << best_sell[0] << '\n';
                                sell_orders.erase(sell_orders.begin());
                                quantity = 0;
                            }
                            else {
                                os << "T," << best_sell[2] << "," << best_sell[0] << '\n';
                                os << "X," << best_sell[1] << ",S," << best_sell[2] << "," << best_sell[0] << '\n';
                                sell_orders.erase(sell_orders.begin());
                                quantity -= best_sell[2];
                            }
                        }
                        if(quantity > 0) {
                            buy_orders.insert({-price, order_id, quantity});            
                        }
                    }
                }
            }
        }
        else if(order_type == 'X') {
            if(is_sell) {
                auto it = sell_orders.find({price, order_id, quantity});
                if(it == sell_orders.end()) {
                    throw invalid_argument("Given order for removal not present.");
                }
                else {
                    sell_orders.erase(it);
                }
            }
            else {
                auto it = buy_orders.find({-price, order_id, quantity});
                if(it == buy_orders.end()) {
                    throw invalid_argument("Given order for removal not present.");
                }
                else {
                    buy_orders.erase(it);
                }
            }
        }
        else if(order_type == 'M') {
            if(is_sell) {
                auto it = sell_orders.lower_bound({price, order_id, 0});
                if(it == sell_orders.end()) {
                    throw invalid_argument("Given order for modification not present.");
                }
                else {
                    auto val = *it;
                    if(order_id != val[1]) {
                        throw invalid_argument("Given order for modification not present.");
                    }
                    else {
                        // order present, modify
                        sell_orders.erase(it);
                        sell_orders.insert({price, order_id, quantity});
                    }
                }
            }
            else {
                auto it = buy_orders.lower_bound({-price, order_id, 0});
                if(it == buy_orders.end()) {
                    throw invalid_argument("Given order for modification not present.");
                }
                else {
                    auto val = *it;
                    if(order_id != val[1]) {
                        throw invalid_argument("Given order for modification not present.");
                    }
                    else {
                        // order present, modify
                        buy_orders.erase(it);
                        buy_orders.insert({-price, order_id, quantity});
                    }
                }
            }
        }
        else {
            assert(false);
        }
        // Print midquote after every message
        printMidQuote(os);
    }

    void printCurrentOrderBook(std::ostream &os) const {
        // Not printing all buy/sell orders for same price in same line.
        // Every order is printed in new line.
        for(auto it = sell_orders.rbegin(); it != sell_orders.rend(); it++) {
            auto val = *it;
            os << val[0] << " S " << val[2] << '\n';
        }
        os << '\n';
        for(auto it = buy_orders.begin(); it != buy_orders.end(); it++) {
            auto val = *it;
            os << -val[0] << " B " << val[2] << '\n';
        }
        os << "\n";
    }

    void printMidQuote(ostream &os) const{
        if(buy_orders.empty() or sell_orders.empty()) {
            if(buy_orders.empty()) {
                os << "MidQuote N/A, No buy orders\n";
            }
            else {
                os << "MidQuote N/A, No sell orders\n";
            }
        }
        else {
            auto best_buy = *buy_orders.begin();
            auto best_sell = *sell_orders.begin();
            double ans = (double)(-best_buy[0] + best_sell[0]) / 2.0;
            os << "MidQuote: " << ans << '\n';
        }
    }
};

int main(int argc, char **argv) {
    // For fast IO in C++
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Code starts
    FeedHandler feed;
    string line;
    const string filename(argv[1]);
    ifstream infile(filename.c_str(), ios::in);
    int counter = 0;
    while (std::getline(infile, line)) {
        feed.processMessage(line, cout);
        if (++counter % 10 == 0) {
            feed.printCurrentOrderBook(cout);
        }
    }
    return 0;
}