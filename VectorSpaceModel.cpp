#include <list>
#include <string>
#include <unordered_map>


static std::string string_from_sv(SV* sv)
{
    STRLEN len;
    const char* str = SvPV(sv, len);
    return std::string(str, len);
}


static double rank_from_result(SV* result)
{   return SvNV(*av_fetch(reinterpret_cast<AV*>(SvRV(result)), 1, 0)); }

static I32 sort_ratings(pTHX_ SV* const sv1, SV* const sv2)
{
    double rank1 = rank_from_result(sv1),
           rank2 = rank_from_result(sv2);
    return rank1 < rank2 ?  1
         : rank1 > rank2 ? -1
         :                  0;
}


struct Entry
{
    int id, frequency;

    SV* dump() const
    {
        AV* entry = newAV();
        av_push(entry, newSViv(id));
        av_push(entry, newSViv(frequency));
        return newRV_noinc(reinterpret_cast<SV*>(entry));
    }
};


class InvertedIndex
{

public:

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


    SV* fetch(SV* tokens)
    {
        std::unordered_map<int, double> rankings;
        AV* av = reinterpret_cast<AV*>(SvRV(tokens));

        for (int i = 0; i <= av_top_index(av); ++i)
        {
            std::string token = string_from_sv(*av_fetch(av, i, 0));
            auto it = index.find(token);
            if (it != index.end())
            {
                double w_global = 1.0 * documents.size() / it->second.size();
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


    SV* dump_index() const
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


    SV* dump_documents() const
    {
        HV* docs = newHV();

        for (const auto& p : documents)
        {
            std::string key = std::to_string(p.first);
            hv_store(docs, key.c_str(), key.size(), newSVnv(p.second), 0);
        }

        return newRV_noinc(reinterpret_cast<SV*>(docs));
    }


private:

    std::unordered_map<std::string, std::list<Entry> > index;
    std::unordered_map<int, double> documents;

};
