#include <list>
#include <string>
#include <unordered_map>


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

    double df(const std::string& token)
    {   return index[token].size(); }

    double idf(const std::string& token)
    {   return log10(documents.size() / df(token)); }


    void add_document(int id, ...)
    {
        Inline_Stack_Vars; // $self, $id, @tokens
        std::unordered_map<std::string, int> vec;

        for (int i = 2; i < Inline_Stack_Items; ++i)
        {
            STRLEN      len;
            const char* str = SvPV(Inline_Stack_Item(i), len);
            ++vec[std::string(str, len)];
        }

        double veclen = 0;
        for (const auto& p : vec)
        {
            index[p.first].push_back({id, p.second});
            veclen += pow(p.second, 2);
        }
        documents[id] = sqrt(veclen);
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
