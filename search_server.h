#pragma once

#include <istream>
#include <ostream>
#include <utility>
#include <set>
#include <list>
#include <unordered_map>
#include <vector>
#include <map>
#include <string>
using namespace std;

class InvertedIndex {
public:
    struct Entry
    {
        size_t docid, hitcount;
    };
    void Add(const string& document);
    vector<Entry> Lookup(const string& word) const;

    const string& GetDocument(size_t id) const {
        return docs[id];
    }

    const size_t GetDocsSize()
    {
        return docs.size();
    }

private:
    map<string, vector<Entry>> index;
    vector<string> docs;
};

class SearchServer {
public:
    SearchServer() = default;
    explicit SearchServer(istream& document_input);
    void UpdateDocumentBase(istream& document_input);
    void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
    InvertedIndex index;
};
