
#include "SpadeScriptDataSet.h"

bool SpadeScriptDataSet :: command(const std::string& s,
        SpadeScriptContext* context,
        std::string& result,
        const std::string& inst,
        const std::string& args)
{
    if(m_Binds.find(s) != m_Binds.end()) {
        result = (*m_Binds[s])(this,inst,args);
        return true;
    }

    return false;
}
