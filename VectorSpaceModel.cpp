#include <fstream>
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


template <typename T> static void
pack(const T& t, std::ofstream& out)
{   out.write(reinterpret_cast<const char*>(&t), sizeof(T)); }

template <typename T> static T
unpack(std::ifstream& in)
{
    T t;
    in.read(reinterpret_cast<char*>(&t), sizeof(T));
    return t;
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

        for (const auto& p : vec)
        {   index[p.first].push_back({id, p.second}); }

        ++documents;
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
                double w_global = log10(1.0 * documents / it->second.size());
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
            av_push(entry, newSVnv(p.second));
            av_push(results, newRV_noinc(reinterpret_cast<SV*>(entry)));
        }

        sortsv(AvARRAY(results), av_top_index(results) + 1, sort_ratings);
        return newRV_noinc(reinterpret_cast<SV*>(results));
    }


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

        return newRV_noinc(reinterpret_cast<SV*>(idx));
    }


    bool stash(const char* path)
    {
        std::ofstream out(path);
        if (!out)
        {
            warn("Couldn't write to stash.\n");
            return false;
        }

        pack(documents, out);
        for (auto p : index)
        {
            pack(p.first.size(), out);
            out.write(p.first.c_str(), p.first.size());

            pack(p.second.size(), out);
            for (const Entry& e : p.second)
            {   pack(e, out); }
        }

        out.close();
        return out.good();
    }


    bool unstash(const char* path)
    {
        std::ifstream in(path);
        if (!in)
        {
            warn("Couldn't read from stash.\n");
            return false;
        }

        documents = unpack<int>(in);
        while (in.peek() != EOF)
        {
            auto length = unpack<std::string::size_type>(in);
            std::string token(length, ' ');
            in.read(&token[0], length);

            std::list<Entry>& entries = index[token];
            auto count = unpack<std::list<Entry>::size_type>(in);
            while (count--)
            {   entries.push_back(unpack<Entry>(in)); }
        }

        return true;
    }


private:

    int documents;

    /* Map from token to document ID and token frequency.
     * The list of entries are be sorted ascending by ID,
     * unless someone broke contract when calling `add_document`. */
    std::unordered_map<std::string, std::list<Entry> > index;

};
