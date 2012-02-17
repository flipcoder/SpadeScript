#ifndef _SPADESCRIPTCONTEXT_H
#define _SPADESCRIPTCONTEXT_H

#include <stack>
#include "SpadeScriptStructure.h"

class SpadeScriptContext {
public:
    std::stack<SpadeScriptStructure> stack;

    void start() {
        stack.push(SpadeScriptStructure("context","context"));
    }
    void start(std::string n, std::string d) {
        stack.push(SpadeScriptStructure(n,d));
    }
    void stop() {
        while(!stack.empty()) stack.pop();
    }
    void restart() {
        stop();start();
    }

    // getVariable(): Recursively goes through the stack to obtain a variable within context's scope
    //    Local scopes have priority
    // This could be optimized more by switching to something other than std::stack (can't read under top() with popping)
    std::string getVariable(std::string var) {
        std::map<std::string,std::string>::iterator itr = stack.top().vars.find(var);
        if(itr == stack.top().vars.end()) {
            std::stack<SpadeScriptStructure> newstack = stack; // not efficient
            while(true) {
                if(newstack.top().type.compare("function")==0)
                    break;
                newstack.pop();
                if(newstack.empty())
                    break;
                if((itr = newstack.top().vars.find(var)) != newstack.top().vars.end()) {
                    return itr->second;
                }
            }
        }
        else return itr->second;
        return "";
    }
};

#endif
