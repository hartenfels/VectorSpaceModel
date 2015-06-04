#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>


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


/* Pack the given thing into the given output stream as raw bytes. */
template <typename T> static void
pack(const T& t, std::ofstream& out)
{   out.write(reinterpret_cast<const char*>(&t), sizeof(T)); }

/* Unpack raw bytes into a thing of the given type. */
template <typename T> static T
unpack(std::ifstream& in)
{
    T t;
    in.read(reinterpret_cast<char*>(&t), sizeof(T));
    return t;
}


/* The best inverted index in the solar system and beyond. */
class InvertedIndex
{

public:

    ~InvertedIndex()
    {
        if (worker.joinable())
        {   worker.join(); }
    }

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

        for (const auto& token2tf : vec)
        {   index[token2tf.first].push_back({id, token2tf.second}); }

        lengths[id] = 0;
    }


    /* Calculates all document lengths.
     * Call this once you're done adding documents. */
    void calculate_lengths()
    {
        const double N = lengths.size();
        for (const auto& token2entries : index)
        {
            double w_global = log10(N / token2entries.second.size());
            for (const auto& id2tf : token2entries.second)
            {
                double& length = lengths[id2tf.first];
                length = sqrt(pow(length, 2) + pow(id2tf.second * w_global, 2));
            }
        }
    }


    /* Run a query and return a list of ranked results.
     * Call this from Perl like `$index->fetch(['water', 'jar'])`. The result
     * will be an arrayref, with each entry of it being an arrayref of the form
     * [id, rank]. They will be sorted by their rank in descending order, so the
     * most relevant document will be at the top. */
    SV* fetch(SV* tokens) const
    {
        const double N = lengths.size();
        std::unordered_map<int, double> rankings;
        AV* av = reinterpret_cast<AV*>(SvRV(tokens));

        if (worker.joinable())
        {   worker.join(); }

        for (int i = 0; i <= av_top_index(av); ++i)
        {
            std::string token = string_from_sv(*av_fetch(av, i, 0));
            auto token2entries = index.find(token);
            if (token2entries != index.end())
            {
                double w_global = log10(N / token2entries->second.size());
                double w_query  = w_global; // FIXME this ain't right

                for (const auto& id2tf : token2entries->second)
                {   rankings[id2tf.first] += w_global * w_query * id2tf.second; }
            }
        }

        AV* results = newAV();
        for (const auto& id2rank : rankings)
        {
            AV*    entry  = newAV();
            double length = lengths.find(id2rank.first)->second;

            av_push(entry, newSViv(id2rank.first));
            av_push(entry, newSVnv(id2rank.second / length));

            av_push(results, newRV_noinc(reinterpret_cast<SV*>(entry)));
        }

        sortsv(AvARRAY(results), av_top_index(results) + 1, sort_ratings);
        return newRV_noinc(reinterpret_cast<SV*>(results));
    }


    /* Dump this index into Perl.
     * Used in testing only. */
    SV* dump() const
    {
        HV* idx = newHV();
        for (const auto& token2entries : index)
        {
            HV* entries = newHV();

            for (const auto& id2tf : token2entries.second)
            {
                std::string k = std::to_string(id2tf.first);
                hv_store(entries, k.c_str(), k.size(),
                         newSViv(id2tf.second), 0);
            }

            hv_store(idx, token2entries.first.c_str(), token2entries.first.size(),
                     newRV_noinc(reinterpret_cast<SV*>(entries)), 0);
        }

        HV* len = newHV();
        for (const auto& id2length : lengths)
        {
            std::string id = std::to_string(id2length.first);
            hv_store(len, id.c_str(), id.size(),
                     newSVpvf("%.2f", id2length.second), 0);
        }

        HV* dump = newHV();
        hv_stores(dump, "index",   newRV_noinc(reinterpret_cast<SV*>(idx)));
        hv_stores(dump, "lengths", newRV_noinc(reinterpret_cast<SV*>(len)));
        return newRV_noinc(reinterpret_cast<SV*>(dump));
    }


    /* Stash this index into the given file.
     * The file format is a simple binary file with the following structure:
     *     lengths.size()
     *     elements of lengths:
     *         document id
     *         length
     *     index.size()
     *     elements of index:
     *         length of token
     *         token
     *         entries.size()
     *         elements of entries:
     *             document id
     *             term frequency
     * Returns wether writing succeeded. */
    bool stash(const char* path)
    {
        std::ofstream out(path);
        if (!out)
        {
            warn("Couldn't write to stash.\n");
            return false;
        }

        pack(lengths.size(), out);
        for (const auto& id2length : lengths)
        {
            pack(id2length.first,  out);
            pack(id2length.second, out);
        }

        pack(index.size(), out);
        for (const auto& token2entries : index)
        {
            pack(token2entries.first.size(), out);
            out << token2entries.first;

            pack(token2entries.second.size(), out);
            for (const auto& id2tf : token2entries.second)
            {
                pack(id2tf.first,  out);
                pack(id2tf.second, out);
            }
        }

        out.close();
        return out.good();
    }


    /* Reconstitutes a stashed file and returns wether it worked.
     * See also `stash`. */
    bool unstash(const char* path)
    {
        instream = new std::ifstream(path);
        if (!*instream)
        {
            warn("Couldn't read from stash.\n");
            return false;
        }
        std::cout << "Unstashing in background.\n";
        worker = std::thread([this](){ unstash_in_background(); });
        return true;
    }


private:

    void unstash_in_background()
    {
        std::ifstream& in = *instream;

        auto lengths_size = unpack<decltype(lengths)::size_type>(in);
        while (lengths_size--)
        {
            int id      = unpack<int   >(in);
            lengths[id] = unpack<double>(in);
        }

        auto index_size = unpack<decltype(index)::size_type>(in);
        while (index_size--)
        {
            auto length = unpack<std::string::size_type>(in);
            std::string token(length, ' ');
            in.read(&token[0], length);

            auto  count   = unpack<decltype(index)::mapped_type::size_type>(in);
            auto& entries = index[token];
            entries.reserve(count);
            while (count--)
            {
                int id = unpack<int>(in);
                entries.push_back({id, unpack<int>(in)});
            }
        }

        delete instream;
        instream = nullptr;
    }

    /* Map from token to document ID and token frequency.
     * The list of entries are be sorted ascending by ID,
     * unless someone broke contract when calling `add_document`. */
    std::unordered_map<std::string, std::vector<std::pair<int, int> > > index;

    /* Map from document ID to document length. */
    std::unordered_map<int, double> lengths;

    std::ifstream* instream;
    mutable std::thread worker;

};
