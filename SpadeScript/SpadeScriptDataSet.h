#ifndef _SPADESCRIPTDATASET_H
#define _SPADESCRIPTDATASET_H

#include <string>
#include <set>
#include <map>
#include "SpadeScript.h"

class SpadeScriptDataSet : public SpadeScript
{
public:
    std::map<std::string,std::set<std::string>> m_DataSet;

    std::map<std::string, std::string (*)(SpadeScriptDataSet*, std::string, std::string)> m_Binds;

    SpadeScriptDataSet() {
        m_Binds.insert( std::make_pair( "add", &SpadeScriptDataSet::cmd_add ));
        m_Binds.insert( std::make_pair( "remove", &SpadeScriptDataSet::cmd_remove ));
        m_Binds.insert( std::make_pair( "has", &SpadeScriptDataSet::cmd_has ));
        m_Binds.insert( std::make_pair( "clear", &SpadeScriptDataSet::cmd_clear ));
    }
    virtual ~SpadeScriptDataSet() {}
    virtual void onSpawn(std::string inst, std::string args) {m_DataSet[inst] = std::set<std::string>();}
    virtual void onRemove(std::string inst, std::string args) {m_DataSet[inst].clear();}

    virtual bool command(const std::string& s,
        SpadeScriptContext* context,
        std::string& result,
        const std::string& inst = "",
        const std::string& args = "");

    static std::string cmd_add(SpadeScriptDataSet* sc, std::string inst, std::string arguments) {
        sc->m_DataSet[inst].insert(arguments);
        return "1";
    }
    static std::string cmd_remove(SpadeScriptDataSet* sc, std::string inst, std::string arguments) {
        std::set<std::string>::iterator itr;
        if((itr = sc->m_DataSet[inst].find(arguments)) != sc->m_DataSet[inst].end()) {
            sc->m_DataSet[inst].erase(itr);
            return "1";
        }
        return "0";
    }
    static std::string cmd_has(SpadeScriptDataSet* sc, std::string inst, std::string arguments) {
        if(sc->m_DataSet[inst].find(arguments) != sc->m_DataSet[inst].end()) {
            return "1";
        }
        return "0";
    }
    static std::string cmd_clear(SpadeScriptDataSet* sc, std::string inst, std::string arguments) {
        sc->m_DataSet[inst].clear();
        return "1";
    }
};

#endif
