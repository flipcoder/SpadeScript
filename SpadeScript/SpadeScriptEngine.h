

#ifndef _SPADESCRIPTENGINE_H
#define _SPADESCRIPTENGINE_H

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stack>
#include <cstdlib>
#include <cstring>
#include <cmath>
//#include <locale>
//#include <ios> 
//#include <iomanip>
//#include <fstream>
#include <memory>

#include "SpadeScript.h"
#include "SpadeScriptStructure.h"
#include "SpadeScriptContext.h"

// A Spade Script Object
class SpadeScriptEngine
{
protected:

	// Script Name -> Script Pointer
	std::map<std::string, SpadeScript*> m_Scripts;

	// Allocated Scripts
	std::map<std::string, SpadeScript*> m_Memory;

	//mu::Parser m_muParser;

public:

	SpadeScriptEngine() {
		srand((unsigned int)time(NULL));
	}
	~SpadeScriptEngine() {}

	static bool stringBeginsWith(const std::string& str, const std::string &begin) {
		std::string test = str;
		test = test.substr(0,begin.length());
		return(test.compare(begin)==0);
	}

	static bool stringEndsWith(const std::string& str, const std::string &end) {
		std::string test = str;
		test = test.substr(test.length() - end.length());
		return(test.compare(end)==0);
	}

	static std::string trim(const std::string &s, const std::string &chars);

	static std::string readLine(std::ifstream& file) {
		char line[256];
		file.getline(line, 256);
		return (std::string)line;
	}

	static int explode(char delim, const std::string& list, std::vector<std::string>& result, bool append = false);

	static std::string safe_lookup(const std::vector<std::string>& v, unsigned int idx, std::string default_val = "");

	static bool vectorHas(const std::vector<std::string>& v, std::string element, bool ignoreCase = false);

	bool addScript(SpadeScript* s, const std::string name) {
		m_Scripts[name] = s;
		return true;
	}

	bool isScript(const std::string name) {
		std::map<std::string, SpadeScript*>::iterator itr = m_Scripts.find(name);
		return (itr != m_Scripts.end());
	}

	void unloadScript(const std::string name) {
		std::map<std::string, SpadeScript*>::iterator itr = m_Scripts.find(name);
		if(itr != m_Scripts.end()) {
			itr->second->removeAll();
			itr->second->clear();
			m_Scripts.erase(itr);
		}
		itr = m_Memory.find(name);
		if(itr != m_Memory.end()) {
			delete itr->second;
			m_Memory.erase(itr);
		}
	}

	// process a global command
	bool processCommand(const std::string& cmd, const std::string& argu, std::string& result, SpadeScriptContext* context) {
		
		/*std::istringstream iss(s);
		std::string token;
		std::string args;

		if(std::getline(iss, token, ' ')){

			if(token.compare("call")==0){
				std::getline(iss, args, ' ');
				result = m_Scripts[args]->call(args, context);
			} else {
				return false;
			}

		}*/

		return false;
	}

	bool isBlock(const std::string& cmd) {
		if(cmd.compare("if")==0 || cmd.compare("loop")==0) {
			return true;
		}
		return false;
	}

	// doBlock
	// Return codes same as a script call(), except
	// doBlock returning 5 means end of block or break
	// call returning 5 means continue (break only current iteration) command
	int doBlock(const std::string& function,
			const std::string& cmd,
			SpadeScript* ss,
			SpadeScriptContext* context,
			const std::string& inst,
			std::string& result,
			// Pair: line#, condition
			const std::vector<std::pair<unsigned int,std::string>>& blockStarts);

	

	void stackTrace(SpadeScriptContext* c) {/* TODO */}

	void clear();

	bool allocate(std::string fn,std::string override_name = "");

	std::string itos(int i) {
		std::stringstream ss;
		ss << i;
		return ss.str();
	}

	std::string evaluate(std::string str, int skiplevel = 0);
	bool condition(const std::string& str);

	SpadeScript* getScript(const std::string& name) {
		if(m_Scripts.find(name)!=m_Scripts.end())
			return m_Scripts[name];
		return NULL;
	}

	bool runScript(std::string script, std::string inst = "", std::string func = "main", std::string args = "", SpadeScriptContext* context = NULL);
	//bool runScriptOnAllInstances(std::string script, std::string func = "main", SpadeScriptContext* context = NULL);

	bool callFunction(std::string script, std::string function);
};

#endif
