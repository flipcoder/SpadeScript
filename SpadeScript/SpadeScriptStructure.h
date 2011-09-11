#ifndef _SPADESCRIPTSTRUCTURE_H
#define _SPADESCRIPTSTRUCTURE_H

class SpadeScriptStructure {
public:
	SpadeScriptStructure(std::string n,std::string t) {
		name = n;
		type = t;
	}
	~SpadeScriptStructure() {}
	std::string name; // not required
	std::string type; // required
	std::map<std::string,std::string> vars; // local scope variables
};


#endif
