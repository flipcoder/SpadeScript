
#ifndef _SPADESCRIPT_H
#define _SPADESCRIPT_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
class SpadeScriptEngine;
class SpadeScriptContext;
#include "SpadeScriptContext.h"
#include <ctime>
#include <sstream>

//#include "../Log.h"

// A Spade Script Object
class SpadeScript
{
protected:

    // Function Name -> Code (array of strings, line by line)
    // Example:
    //  function kill(player)  // Function Name is "kill"
    //  {
    //        do stuff; // code line #1
    //        do more stuff; // code line #2
    //  }

    // Instance Model
    //  This stores each required instance variable with its default value
    //  in a collection known as "model"
    //  Used to spawn instances of the script's object
    // Instance Model Name -> Variable Name -> Default Value
    //std::map<std::string,std::map<std::string,std::string>> m_InstanceModels;

    // Instance Name -> Variable Name -> Value
    std::map<std::string,std::map<std::string,std::string>> m_InstanceVariables;

    // Variable Name -> Value
    std::map<std::string,std::string> m_StaticVariables;

    // Function Name -> Code Lines
    std::map<std::string,std::vector<std::string>> m_Functions;

    // Garbage Collection
    // Instance Name -> vector of associated script -> instances that need to be destroyed (onRemove())
    //std::map<std::string,std::vector<std::string,std::string>> m_Garbage;

    // Optional:
    // First: A string description of the context in which the script is running
    // Second: More detailed reference if needed
    //  Example would be "client" or "server"
    //std::pair<std::string,std::string> m_sContext; // Replaced with context reference

    // Required reference pointer to script engine
    SpadeScriptEngine* m_pEngine;
    std::string m_sName;
    
    //Log* m_pLog; //weakref

public:

    //friend class SpadeScriptEngine;
    
    SpadeScript():
        m_pEngine(NULL){}
    /*SpadeScript(std::string name):
        m_pEngine(NULL),
        m_sName(name) {}*/
    virtual ~SpadeScript() {}
    virtual void onSpawn(std::string inst, std::string args = "") {}
    virtual void onRemove(std::string inst, std::string args = "") {}
    std::string getName() const { return m_sName; }

    //void setLogger(Log* log) { m_pLog = log; }
    //Log* getLogger() const { return m_pLog; }

    // Call a script function
    // This method parses all the code inside the function and executes any appropriate calls using the context provided
    int call(const std::string& function,
        SpadeScriptContext* context,
        std::string& result,
        std::string inst = "",
        const std::string& args = "",
        int lineStart = 0);

    //bool doBlock() {}

    // Answer an appropriate internal call
    //  Return true on success, false if command not recognized
    //  Store return value in result
    virtual bool command(const std::string& s,
        SpadeScriptContext* context,
        std::string& result,
        const std::string& inst = "",
        const std::string& args = "") { return false; }

    // Script file load method
    bool load(const std::string& fn, SpadeScriptEngine* eng, bool use_file = true, std::string override_name = "");
    //bool load(const std::vector<std::string> lines, SpadeScriptEngine* eng); // add this later for raw scripts

    bool substitute(const std::string& expression, std::string& result, SpadeScriptContext* c, const std::string& inst);

    // Clears all loaded function data
    void clear() { removeAll(); m_Functions.clear(); }

    // Spawn an instance of the object
    std::string spawn(const std::string& InstanceName, SpadeScriptContext* context = NULL, std::string args = "", const std::string CopyFrom = "");

    bool remove(const std::string InstanceName, SpadeScriptContext* context = NULL);
    bool removeAll( SpadeScriptContext* context = NULL );

    SpadeScriptEngine* getEngine() { return m_pEngine; }

    std::string getVariable(const std::string& varname, const std::string inst = "default", std::string def = "") {
        if(inst.length()==0 || inst.compare("default")==0) {
            if(m_StaticVariables.find(varname)!=m_StaticVariables.end())
                return m_StaticVariables[varname];
            else
                return def;
        }
        if(m_InstanceVariables.find(inst)!=m_InstanceVariables.end() &&
            m_InstanceVariables[inst].find(varname)!=m_InstanceVariables[inst].end())
            return m_InstanceVariables[inst][varname];
        else
            return def;
    }
    bool setVariable(const std::string& varname, const std::string val, const std::string inst = "default") {
        if(inst.length()==0 || inst.compare("default")==0) {
            m_StaticVariables[varname] = val;
            return true;
        }
        m_InstanceVariables[inst][varname] = val;
        return true;
    }

    std::vector<std::string>* getFunction(const std::string n) {
        if(m_Functions.find(n)!=m_Functions.end())
            return &m_Functions[n];
        return NULL;
    }

    void clearVars(std::string inst) {

        // TODO: deal with '*'

        clearVirtualVars(inst);

        if(inst.compare("default")==0 || inst.length()==0)
        {
            m_StaticVariables.clear();
        }
        else
        {
            if(m_InstanceVariables.find(inst)!=m_InstanceVariables.end())
                m_InstanceVariables[inst].clear();
        }
    }
    virtual void clearVirtualVars(std::string inst = "") {}

    /*std::map<std::string,std::map<std::string,std::string>>* getInstanceVars() {
        return &m_InstanceVariables;
    }
    std::map<std::string,std::map<std::string,std::string>>* getStaticVars() {
        return &m_StaticVariables;
    }*/

    //std::vector<std::string>* getFunction(const std::string& s) const { return &m_Functions[s]; }
    
};

#endif
