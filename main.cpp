#define FILE_EXTENSION ".txt"
#include <algorithm>
#include <cstring>
#include <experimental/filesystem>  // for g++ 7.5.0
//#include <filesystem> // for newer g++ version
//#include <ctime>  // time
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::experimental::filesystem;  // remove "experimental" for newer g++ version

// Trie
const int ALPHABET_SIZE = 26;

// trie node
struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];
    // isEndOfWord is true if the node represents
    // end of a word
    bool isEndOfWord;
};

// vector of TreeNode roots for each txt file
vector<TrieNode *> tries;
vector<TrieNode *> tries_reversed;

// Returns new trie node (initialized to NULLs)
struct TrieNode *getNode(void) {
    struct TrieNode *pNode = new TrieNode;

    pNode->isEndOfWord = false;

    for (int i = 0; i < ALPHABET_SIZE; i++)
        pNode->children[i] = NULL;

    return pNode;
}

// If not present, inserts key into trie
// If the key is prefix of trie node, just
// marks leaf node
void insert(struct TrieNode *root, string key) {
    struct TrieNode *pCrawl = root;

    for (int i = 0; i < key.length(); i++) {
        if (key[i] <= 'Z') {
            key[i] += (26 + 6);  // Uppercase letters are seen as lowercase letters
        }
        int index = key[i] - 'a';  // ASCII code to 0~25 index
        if (!pCrawl->children[index])
            pCrawl->children[index] = getNode();

        pCrawl = pCrawl->children[index];
    }

    // mark last node as leaf
    pCrawl->isEndOfWord = true;
}

// Search function for "exact" key in trie
bool search(struct TrieNode *root, string key) {
    struct TrieNode *pCrawl = root;

    for (int i = 0; i < key.length(); i++) {
        int index = key[i] - 'a';
        if (!pCrawl->children[index])
            return false;

        pCrawl = pCrawl->children[index];
    }

    return (pCrawl->isEndOfWord);
}
// Search function for prefix key in trie
bool search_prefix(struct TrieNode *root, string key) {
    struct TrieNode *pCrawl = root;

    for (int i = 0; i < key.length(); i++) {
        int index = key[i] - 'a';
        if (!pCrawl->children[index])
            return false;

        pCrawl = pCrawl->children[index];
    }

    return true;  // No need to check if it's leaf or not
}
// infixToPostfix conversion
vector<string> infixToPostfix(vector<string> key_string) {
    vector<string> key_postfix;
    stack<string> s;
    for (auto i : key_string) {
        if (i == "+" || i == "/") {
            while (!s.empty()) {
                key_postfix.push_back(s.top());
                s.pop();
            }
            s.push(i);
        } else {
            key_postfix.push_back(i);
        }
    }
    while (!s.empty()) {
        key_postfix.push_back(s.top());
        s.pop();
    }

    return key_postfix;
}
// Evaluate function for postfix query
bool eval(TrieNode *root, vector<string> key_postfix, int num) {
    stack<bool> s;
    for (auto i : key_postfix) {
        if (i == "+" || i == "/") {
            bool right = s.top();
            s.pop();
            bool left = s.top();
            s.pop();
            if (i == "+") {
                s.push(left & right);
            } else {
                s.push(left | right);
            }
        } else {
            if (i[0] == '"') {
                string sub_string = i.substr(1, i.size() - 2);
                s.push(search(root, sub_string));
            } else if (i[0] == '*') {
                string sub_string = i.substr(1, i.size() - 2);
                reverse(sub_string.begin(), sub_string.end());
                s.push(search_prefix(tries_reversed[num], sub_string));
            } else {
                s.push(search_prefix(root, i));
            }
        }
    }
    return s.top();
}

// string parser : output vector of strings (words) after parsing
vector<string> word_parse(vector<string> tmp_string) {
    vector<string> parse_string;
    for (auto &word : tmp_string) {
        string new_str;
        for (auto &ch : word) {
            if (isalpha(ch))
                new_str.push_back(ch);
        }
        parse_string.emplace_back(new_str);
    }
    return parse_string;
}

vector<string> split(const string &str, const string &delim) {
    vector<string> res;
    if ("" == str) return res;
    //先將要切割的字串從string型別轉換為char*型別
    char *strs = new char[str.length() + 1];  //不要忘了
    strcpy(strs, str.c_str());

    char *d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while (p) {
        string s = p;      //分割得到的字串轉換為string型別
        res.push_back(s);  //存入結果陣列
        p = strtok(NULL, d);
    }

    return res;
}
//
int main(int argc, char *argv[]) {
    // INPUT :
    // 1. data directory in data folder
    // 2. number of txt files
    // 3. output route

    string data_dir = argv[1] + string("/");
    string query = string(argv[2]);
    string output = string(argv[3]);

    // Read File & Parser

    fstream fi;  // input
    fstream fo;  // output
    string file, title_name, tmp;

    vector<string> tmp_string;
    vector<string> title_string;

    int count = 0;
    string previous;  // for storing previous query
    for (auto &p : fs::directory_iterator(data_dir)) {
        // from data_dir get file ....
        fi.open(data_dir + to_string(count) + ".txt", ios::in);

        // GET TITLENAME
        getline(fi, title_name);
        if (title_name == previous) {
            continue;
        }
        previous = title_name;
        title_string.push_back(title_name);  // store titles of all files for output

        // GET TITLENAME WORD ARRAY
        tmp_string = split(title_name, " ");

        // trie for this current file
        struct TrieNode *root = getNode();
        struct TrieNode *root_reversed = getNode();  // words are reversed before been inserted into this trie

        vector<string> title = word_parse(tmp_string);
        for (int i = 0; i < title.size(); i++) {
            insert(root, title[i]);
            reverse(title[i].begin(), title[i].end());
            insert(root_reversed, title[i]);
        }

        // GET CONTENT LINE BY LINE
        while (getline(fi, tmp)) {
            // GET CONTENT WORD VECTOR
            tmp_string = split(tmp, " ");

            // PARSE CONTENT
            vector<string> content = word_parse(tmp_string);

            for (int i = 0; i < content.size(); i++) {
                insert(root, content[i]);
                reverse(content[i].begin(), content[i].end());
                insert(root_reversed, content[i]);
            }
        }
        // push root of current file into tries vector
        tries.push_back(root);
        tries_reversed.push_back(root_reversed);

        // CLOSE FILE
        fi.close();
        count++;
    }

    fo.open(output, ios::out);
    fi.open(query, ios::in);
    string key;
    vector<string> all_query;        // store all previous queries
    vector<vector<int>> all_titles;  // store all previous titles corresponding to each query
    while (!fi.eof()) {
        getline(fi, key);
        bool repeated = false;
        int index = 0;
        // check for repeated queries. If not, then store the query into all_query vector
        for (auto i : all_query) {
            if (i == key) {
                for (auto j : all_titles[index]) {
                    if (j == -1) {
                        fo << "Not Found!\n";
                    } else {
                        fo << title_string[j] << "\n";
                    }
                }
                repeated = true;
                break;
            }
            index++;
        }
        if (repeated) {
            continue;
        }
        all_query.push_back(key);
        //
        vector<string> key_string;
        key_string = split(key, " ");
        vector<string> key_postfix = infixToPostfix(key_string);
        vector<int> result;  // store all resulting titles for this query

        bool found = false;
        int i = 0;
        for (auto &root : tries) {
            if (eval(root, key_postfix, i) == true) {
                fo << title_string[i] << "\n";
                found = true;
                result.push_back(i);
            }
            i++;
        }
        if (!found) {
            fo << "Not Found!\n";
            result.push_back(-1);
        }
        all_titles.push_back(result);
    }
    fi.close();
    fo.close();
    //cout << double(clock()) / CLOCKS_PER_SEC << endl;  // for testing purposes
    return 0;
}

// 1. UPPERCASE CHARACTER & LOWERCASE CHARACTER ARE SEEN AS SAME.
// 2. FOR SPECIAL CHARACTER OR DIGITS IN CONTENT OR TITLE -> PLEASE JUST IGNORE, YOU WONT NEED TO CONSIDER IT.
//    EG : "AB?AB" WILL BE SEEN AS "ABAB", "I AM SO SURPRISE!" WILL BE SEEN AS WORD ARRAY AS ["I", "AM", "SO", "SURPRISE"].
// 3. THE OPERATOR IN "QUERY.TXT" IS LEFT ASSOCIATIVE
//    EG : A + B / C == (A + B) / C
