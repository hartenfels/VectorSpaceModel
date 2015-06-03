#include <list>
#include <string>
#include <unordered_map>


/* Make an std::string from a perl scalar value (SV). */
static std::string string_from_sv(SV* sv)
{
    STRLEN len;
    const char* str = SvPV(sv, len);
    return std::string(str, len);
}


/* Utility function to fetch the rank from a result.
 * The result is an arrayref of the form [id, rank], so
 * this just does $result->[1] and returns it as a double. */
static double rank_from_result(SV* result)
{   return SvNV(*av_fetch(reinterpret_cast<AV*>(SvRV(result)), 1, 0)); }

/* Sort callback, used to sort the results from `fetch` by their rank. It is
 * sorted in descending order, so that the highest-ranking entry is at the top.
 * That weird pTHX_ thing is the current perl interpreter context, but we don't
 * actually need it here. */
static I32 sort_ratings(pTHX_ SV* const sv1, SV* const sv2)
{
    double rank1 = rank_from_result(sv1),
           rank2 = rank_from_result(sv2);
    return rank1 < rank2 ?  1
         : rank1 > rank2 ? -1
         :                  0;
}


/* An entry in the InvertedIndex. Maps document IDs to the
 * number of times a token appears in the document. */
struct Entry
{
    int id, frequency;

    /* Dump this Entry as an arrayref of the form [id, frequency].
     * Used for testing only. */
    SV* dump() const
    {
        AV* entry = newAV();
        av_push(entry, newSViv(id));
        av_push(entry, newSViv(frequency));
        return newRV_noinc(reinterpret_cast<SV*>(entry));
    }
};


/* The best inverted index in the solar system and beyond. */
class InvertedIndex
{

public:

    /* Add a document with the given ID and arrayref of tokens to the index.
     * Each time this is called, the given ID must be greater than the previous
     * one. The IDs need not be sequential, however.
     * Call this from Perl like `$index->add_document(123, ['cup', 'tea'])`. */
    void add_document(int id, SV* tokens)
    {
        std::unordered_map<std::string, int> vec;
        AV* av = reinterpret_cast<AV*>(SvRV(tokens));

        for (int i = 0; i <= av_top_index(av); ++i)
        {
            std::string token = string_from_sv(*av_fetch(av, i, 0));
            ++vec[token];
        }

        double veclen = 0;
        for (const auto& p : vec)
        {
            index[p.first].push_back({id, p.second});
            veclen += pow(p.second, 2);
        }
        documents[id] = sqrt(veclen);
    }


    /* Run a query and return a list of ranked results.
     * Call this from Perl like `$index->fetch(['water', 'jar'])`. The result
     * will be an arrayref, with each entry of it being an arrayref of the form
     * [id, rank]. They will be sorted by their rank in descending order, so the
     * most relevant document will be at the top. */
    SV* fetch(SV* tokens) const
    {
        std::unordered_map<int, double> rankings;
        AV* av = reinterpret_cast<AV*>(SvRV(tokens));

        for (int i = 0; i <= av_top_index(av); ++i)
        {
            std::string token = string_from_sv(*av_fetch(av, i, 0));
            auto it = index.find(token);
            if (it != index.end())
            {
                double w_global = log10(1.0 * documents.size() / it->second.size());
                double w_query  = w_global; // FIXME this ain't right

                for (const Entry& e : it->second)
                {   rankings[e.id] += w_global * w_query * e.frequency; }
            }
        }

        AV* results = newAV();
        for (const auto& p : rankings)
        {
            AV* entry = newAV();
            av_push(entry, newSViv(p.first));
            av_push(entry, newSVnv(p.second / documents.find(p.first)->second));
            av_push(results, newRV_noinc(reinterpret_cast<SV*>(entry)));
        }

        sortsv(AvARRAY(results), av_top_index(results) + 1, sort_ratings);
        return newRV_noinc(reinterpret_cast<SV*>(results));
    }


    /* Dumps this index into Perl. The result will be a hashref with the keys
     * `index` and `documents`. The index will be a mapping from tokens to
     * [id, frequency] pairs and the documents will just be a mapping from id
     * to document vector length.
     * Used in testing only. */
    SV* dump() const
    {
        HV* idx = newHV();
        for (const auto& p : index)
        {
            AV* entries = newAV();

            for (const Entry& entry : p.second)
            {   av_push(entries, entry.dump()); }

            hv_store(idx, p.first.c_str(), p.first.size(),
                     newRV_noinc(reinterpret_cast<SV*>(entries)), 0);
        }

        HV* docs = newHV();
        for (const auto& p : documents)
        {
            std::string key = std::to_string(p.first);
            hv_store(docs, key.c_str(), key.size(), newSVnv(p.second), 0);
        }

        HV* dump = newHV();
        hv_stores(dump, "index",     newRV_noinc(reinterpret_cast<SV*>(idx )));
        hv_stores(dump, "documents", newRV_noinc(reinterpret_cast<SV*>(docs)));
        return newRV_noinc(reinterpret_cast<SV*>(dump));
    }


private:

    /* Map from token to document ID and token frequency.
     * The list of entries are be sorted ascending by ID,
     * unless someone broke contract when calling `add_document`. */
    std::unordered_map<std::string, std::list<Entry> > index;

    /* Map from document ID to length of the document vector. */
    std::unordered_map<int, double> documents;

};
