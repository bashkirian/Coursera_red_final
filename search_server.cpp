#include "search_server.h"
#include "iterators_range.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <iostream>

vector<string> SplitIntoWords(const string& line) {
    istringstream words_input(line);
    return { istream_iterator<string>(words_input), istream_iterator<string>() };
}

SearchServer::SearchServer(istream& document_input) {
    UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
    InvertedIndex new_index;

    for (string current_document; getline(document_input, current_document); ) {
        new_index.Add(move(current_document));
    }

    index = move(new_index);
}

void SearchServer::AddQueriesStream(
    istream& query_input, ostream& search_results_output
) {
    vector<size_t> docid_count(index.GetDocsSize());
    vector<size_t> search_results(docid_count.size());
    int i = 0;
    for (string current_query; getline(query_input, current_query); ) {
        docid_count.assign(docid_count.size(), 0);
        const auto words = SplitIntoWords(current_query);
        ++i;
        for (const auto& word : words) {
            for (const auto& [docid, hitcount] : index.Lookup(word)) {
                docid_count[docid] += hitcount;
            }
        }
        iota(search_results.begin(), search_results.end(), 0);
        auto middle = (5 > search_results.size()) ? search_results.end() : begin(search_results) + 5;
        partial_sort(
            begin(search_results),
            middle,
            end(search_results),
            [&docid_count](int64_t lhs, int64_t rhs) {
                return make_pair(docid_count[lhs], -lhs) > make_pair(docid_count[rhs], -rhs);
            }
        );

        search_results_output << current_query << ':';
        for (size_t& docid : Head(search_results, 5)) {
            const size_t hit_count = docid_count[docid];
            if (hit_count == 0) {
                break;
            }
            search_results_output << " {"
                << "docid: " << docid << ", "
                << "hitcount: " << hit_count << '}';
        }
        search_results_output << endl;
    }
}

void InvertedIndex::Add(const string& document) {
    docs.push_back(move(document));
    const size_t docid = docs.size() - 1;
    for (const auto& word : SplitIntoWords(document)) {
        auto& docids = index[word];
        if (!docids.empty() && docids.back().docid == docid) {
            ++docids.back().hitcount;
        }
        else {
            docids.push_back({ docid, 1 });
        }
    }
}

vector<InvertedIndex::Entry> InvertedIndex::Lookup(const string& word) const {
    if (auto it = index.find(word); it != index.end()) {
        return it->second;
    }
    else {
        return {};
    }
}
