#include <list>
#include <string>
#include <unordered_map>


struct Entry
{
    int id;
    int occurrences;

    SV* dump() const
    {
        AV* entry = newAV();
        av_push(entry, newSViv(id));
        av_push(entry, newSViv(occurrences));
        return newRV_noinc(reinterpret_cast<SV*>(entry));
    }
};


class InvertedIndex
{

public:

    void add_token(int id, const char* token)
    {
        auto& list = index[token];
        if (list.back().id == id)
        {   ++list.back().occurrences; }
        else
        {   list.push_back({id, 1}); }
    }


    SV* dump_index()
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


private:

    std::unordered_map<std::string, std::list<Entry> > index;
    std::unordered_map<int, double> documents;

};
