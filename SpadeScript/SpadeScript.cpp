#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <algorithm>
#include <string>
using namespace std;

#include "SpadeScriptEngine.h"
#include "SpadeScript.h"


// Calls within the current context, not affecting the stack
int SpadeScript :: call(const string& function,
						SpadeScriptContext* context,
						string& result,
						string inst,
						const string& args,
						int lineStart)
{
	// Call all instances of object if instance name is wildcard ('*') symbol
	if(inst.compare("*")==0)
	{
		//map<string,map<string,string>> m_InstanceVariables
		for(map<string,map<string,string>>::iterator itr = m_InstanceVariables.begin();
			itr != m_InstanceVariables.end();
			itr++)
		{
			string subresult = "";
			SpadeScriptContext subcontext;
			subcontext.start();
			int subr = call(function,&subcontext,subresult,itr->first,args);
			if(subr!=1)
				return subr;
		}
		return 1;
	}

	// Randomly select an instance if instance name is question mark ('?') symbol
	if(inst.compare("?")==0) // TODO: Add this for variable substitution as well
	{
		if(m_InstanceVariables.size()==0)
			return 1;

		unsigned int j = (unsigned int)(rand()%m_InstanceVariables.size());
		int i = 0;
		for(map<string,map<string,string>>::iterator itr = m_InstanceVariables.begin();
			itr != m_InstanceVariables.end();
			++itr)
		{
			if(i==j){
				inst = itr->first;
				break;
			}
			++i;
		}
		//inst = (*(randomInst->first));
	}

	// Check if instance name is valid (we've already checked for *'s (wildcards)
	if(m_InstanceVariables.find(inst) == m_InstanceVariables.end()) { // refers to specific instance name
		if(inst.length()>0 && inst.compare("default") != 0) { // refers to static?
			// [DISABLED] Auto-Create if no instance
			//	spawn(inst,context);
			return 0;
		}
	}

	// If this function is NOT part of the script, then we have nothing to call here, so check for internal function
	if(m_Functions.find(function) == m_Functions.end()) {
		string r = "";
		// Attempt to find internal function
		//context->stack.push(SpadeScriptStructure("function",function));
		if(!command(function,context,r,inst,args)) {
			cout<<"ERROR: No such function '"<<function <<"'"<<endl;
			return 0;
		}
		//context->stack.pop();
		result = r;
		return 1;
	}

	string r = ""; // return value

	// MAIN SCRIPT PARSER
	// - Loop through each line, and figure out what to do...
	//cout<<"Parsing... "<<function<<" ("<<args<<")... at line "<<lineStart<<endl;
	for(vector<string>::iterator itr = m_Functions[function].begin() + lineStart;
		itr != m_Functions[function].end();
		itr++)
	//for(unsigned int i=lineStart; i<=m_Functions[function].size()-1; i++)
	{
		unsigned int i = itr - m_Functions[function].begin();
		//string* itr = &m_Functions[function][i]; // REMOVED
		//cout<<"COMMAND: "<<*itr<<endl;

		// format command token
		unsigned int first = itr->find_first_of(" ");

		string cmd = itr->substr(0, first);
		SpadeScriptEngine::trim(cmd," \t");

		string argu = "";

		if(first!=string::npos) {
			argu = itr->substr(first);
			argu = SpadeScriptEngine::trim(argu," \t"); // Take off parenthesis?
		}

		substitute(cmd, cmd, context, inst); // substitute command variables
		substitute(argu, argu, context, inst); // substitute argument variables

		if(cmd.find(".") != string::npos) {

			string c_script = "";
			string c_inst = "";
			string c_func = "";

			unsigned int idx = cmd.find(".");
			
			int n = 0;
			for(unsigned int k = 0; k < cmd.size(); k++)
				if(cmd[k] == '.')
					n++;

			if(n==1) // 1 dot
			{
				string c_parent = cmd.substr(0,idx);
				//c_inst = inst;
				c_func = cmd.substr(idx+1);

				if(c_parent.compare("default")==0) {
					// run default function
					c_inst = "default";
					c_script = getName();
				} else if(m_InstanceVariables.find(c_parent) != m_InstanceVariables.end()) {
					// instance variable
					c_inst = c_parent;
					c_script = getName();
				} else {
					// Parent attribute is definitely a script (???)
					if(!m_pEngine->isScript(c_parent)) {
						// error: no such script or local instance called 'c_script'
						return 0;
					}
					
					c_inst = "default"; // WAS inst
					c_script = c_parent;
				}
			}
			else if(n==2) // 2 
			{
				unsigned int jdx = cmd.find(".",idx+1);

				c_script = cmd.substr(0,idx);
				c_inst = cmd.substr(idx+1, jdx-(idx+1)); // could be default
				c_func = cmd.substr(jdx+1);
			}
			else
			{
				// error
				return 0;
			}

			// Call the function!
			// Local function call
		
			context->stack.push(SpadeScriptStructure(cmd,"function"));
			string callresult = "";
			
			if(! m_pEngine->getScript(c_script)->call(c_func, context, callresult, c_inst, argu))
				return 0;

			context->stack.pop();
			context->stack.top().vars[string("_result")] = callresult;
			
		} else if (cmd.compare("}")==0) {
			// End of the current block
			// This will only trigger on control structures, not current functions
			return 2; // end block
		} else if(cmd.compare("{")==0) {
			// ignore (???)
			//return 0; // error
		} else if (m_pEngine->isBlock(cmd)) {
			
			unsigned int reentry = m_Functions[function].end() - m_Functions[function].begin() - 1;
			result = "";
			//unsigned int i = 0; // TEMP
			
			vector<pair<unsigned int, string>> vConnected;
			vConnected.push_back(make_pair(i,argu));

			//if(cmd.compare("if")==0) {
				// recursively go through trying to find connected blocks (else, else if, etc.)
			
			// The below code has an error in it.

			//cout<<"CONTROL TYPE: "<<cmd<<endl;
				int relative_depth = 0;
				for(vector<string>::iterator jtr = m_Functions[function].begin() + i + 1;
					jtr != m_Functions[function].end();
					jtr++)
				{
					//cout<<"looped @"<<relative_depth<<endl;
					if(cmd.compare("if")==0 && SpadeScriptEngine::stringBeginsWith(*jtr, "else"))
					{
						//cout<<"else! @"<<relative_depth<<endl;
						if(relative_depth == 0)
						{	
							string block_token = "";
							if(jtr->find_first_of(" ") != string::npos)
								block_token = jtr->substr(itr->find_first_of(" "));

							vConnected.push_back(make_pair(
								jtr - m_Functions[function].begin(), // line #
								block_token // condition
							));
						}
						// else keep going
					} else if(SpadeScriptEngine::stringBeginsWith(*jtr,"{")) {
						relative_depth++;
						//cout<<"rel depth ++ is now at"<<relative_depth<<endl;
					} else if(SpadeScriptEngine::stringBeginsWith(*jtr,"}")) {
						relative_depth--;

						//cout<<"rel depth -- is now at"<<relative_depth<<endl;

						if(cmd.compare("loop")==0 && relative_depth <= 0) {
							//cout<<"done building brackets (2)"<<endl;
							reentry = jtr - m_Functions[function].begin();
							break;
						}
						if(cmd.compare("if")==0 && relative_depth==-1) {
							//cout<<"done building brackets (3)"<<endl;
							reentry = jtr - m_Functions[function].begin() - 1;
							break;
						}
						
					}
					else if(relative_depth <= 0) { // if depth is back to start line doesnt match a connector
						//cout<<"done building brackets"<<endl;
						reentry = jtr - m_Functions[function].begin() - 1;
						break;
					}
				}
			//}

			int blockr = m_pEngine->doBlock(function,cmd,this,context,inst,result,vConnected);

			//itr = m_Functions[function].end() - 2;
			itr = m_Functions[function].begin() + reentry;

			// if block is if statement, pass break/continue down to the outer loop
			if(cmd.compare("if")==0)
				if(blockr == 5 || blockr == 1) {
					return blockr;
				}

			// old comments:
			//  execute the control structure
			//  give the pointers for the start, stop lines to block evaluator ( in THIS class )

		} else if (cmd.compare("break")==0) {
			return 5;
		} else if (cmd.compare("continue")==0) {
			return 1;
		} else if (cmd.compare("_args")==0) {
			// transform arguments in given list to vector
			
			//argu.erase(remove_if(argu.begin(), argu.end(), isspace), argu.end());
			istringstream iss_names(argu);
			istringstream iss_values(args);
			string n,v;
			while(getline(iss_names, n, ',') && getline(iss_values, v, ',')) {
				v = SpadeScriptEngine::trim(v," ");
				context->stack.top().vars[n] = v;
			}

		} else if (cmd.compare("random")==0) {
			unsigned int n = rand() % atoi(argu.c_str());
			stringstream ss;
			ss << n;
			context->stack.top().vars[string("_result")] = n;
		}
		//else if(command(cmd,context,r,inst,argu)) { // TODO: Push element on context stack
		//	return 1; // User (virtual) command
		//}
		else if (cmd.compare("return")==0) {
			result = argu;
			return 1;
		} else if (cmd.compare("log")==0) {
			//cout<<argu<<endl; // make way to disable logging
			// FIX: CHANGED FOR SPADE ENGINE
			//OgreFramework::getSingletonPtr()->m_pLog->logMessage(argu);
			Log::get().write(argu);
		} else if (cmd.compare("math")==0) {
			//erase spaces in math string
			//while(argu.find_first_of(" ")!=string::npos)
			//	argu.erase(argu.find(" "),1);

			// feed to engine math evaluate(), return result
			context->stack.top().vars["_result"] = m_pEngine->evaluate(argu);
		} else if (cmd.compare("set")==0) {

			//istringstream iss(argu);
			string var,val;
			//getline(iss, var, '=');
			var = argu.substr(0, argu.find_first_of("="));
			var = SpadeScriptEngine::trim(var," \t");
			//getline(iss, val, ' ');
			val = argu.substr(argu.find_first_of("=")+1);

			//while(val.find_first_of(" ")!=string::npos)
			//	val.erase(val.find(" "),1);

			//cout<<val<<endl;
			val = m_pEngine->evaluate(val);
			//cout<<val<<endl;

			// number of .'s (dots) in variable name
			int n = 0;
			for(unsigned int k = 0; k < var.size(); k++)
				if(var[k] == '.')
					n++;

			unsigned int idx = var.find(".");

			// Set Context Variable
			if(n==0)
				context->stack.top().vars[var] = val;
			else if(n==1) // Set instance or script static variable
			{
				string sel_inst = var.substr(0,idx);
				string sel_var = var.substr(idx+1);

				if(sel_inst.length()==0)
					m_InstanceVariables[inst][sel_var] = val;
				else if(sel_inst.compare("default")==0)
					m_StaticVariables[sel_var] = val;
				else if(m_InstanceVariables.find(sel_inst) != m_InstanceVariables.end()) {
					m_InstanceVariables[sel_inst][sel_var] = val;
				}else if(m_pEngine->isScript(sel_inst)) {
					// use script.variable AS script.default.variable
					m_pEngine->getScript(sel_inst)->setVariable(sel_var, val);
				}else{
					// Spawn Script?  (maybe if auto-spawn is on?)
				}
			}
			else if(n==2)
			{
				unsigned int jdx = var.find(".",idx+1);
				string sel_script = var.substr(0,idx);
				string sel_inst = var.substr(idx+1,jdx-(idx+1));
				string sel_var = var.substr(jdx+1);
				if(m_pEngine->isScript(sel_script)) {
					// get script variable
					m_pEngine->getScript(sel_script)->setVariable(sel_var, val, sel_inst);
				} else {
					// Error: 
				}
			}

		} else if (cmd.compare("exit")==0) {
			return 1;
		} else if (cmd.compare("die")==0) {
			return 0;
		} /*else if (cmd.compare("#")==0) {  // Comments are removed on script load
			// ignore comments
		} */else if (cmd.compare("_printstack")==0) {
			// PRINT VARIABLES
			cout<<"> Stack element '"<<context->stack.top().name<<"' @ "<<context->stack.top().type<<endl;
			for(map<string,string>::iterator jtr = context->stack.top().vars.begin();
				jtr != context->stack.top().vars.end();
				jtr++)
			{
				cout<<"> "<<jtr->first<<" : "<<jtr->second<<endl;
			}
		} else if (cmd.compare("_debug")==0) {
			if(argu.compare("CONTEXT")==0 || argu.compare("STACK")==0)
			{
				// PRINT VARIABLES
				cout<<"> Stack element '"<<context->stack.top().name<<"' OF "<<context->stack.top().type<<endl;
				for(map<string,string>::iterator jtr = context->stack.top().vars.begin();
					jtr != context->stack.top().vars.end();
					jtr++)
				{
					cout<<"> "<<jtr->first<<" : "<<jtr->second<<endl;
				}
			} else if(argu.compare("INSTANCE")==0) {

				cout<<"> Instance element '"<<inst<<"' OF "<<getName()<<endl;
				for(map<string,string>::iterator jtr = m_InstanceVariables[inst].begin();
					jtr != m_InstanceVariables[inst].end();
					jtr++)
				{
					cout<<"> "<<jtr->first<<" : "<<jtr->second<<endl;
				}
					
			} else if(argu.compare("STATIC")==0) {
				cout<<"> Static Element OF "<<getName()<<endl;
				for(map<string,string>::iterator jtr = m_StaticVariables.begin();
					jtr != m_StaticVariables.end();
					jtr++)
				{
					cout<<"> "<<jtr->first<<" : "<<jtr->second<<endl;
				}
			} else if(argu.compare("SCRIPT")==0) {
				cout<<"> All Elements OF "<<getName()<<endl;
				for(map<string,string>::iterator jtr = m_StaticVariables.begin();
					jtr != m_StaticVariables.end();
					jtr++)
				{
					cout<<"> "<<jtr->first<<" : "<<jtr->second<<endl;
				}
				
				for(map<string,map<string,string>>::iterator jtr = m_InstanceVariables.begin();
					jtr != m_InstanceVariables.end();
					jtr++)
				{
					cout<<"> Instance element '"<<jtr->first<<"' OF "<<getName()<<endl;
					for(map<string,string>::iterator ktr = jtr->second.begin();
						ktr != jtr->second.end();
						ktr++)
					{
						cout<<"> "<<ktr->first<<" : "<<ktr->second<<endl;
					}
				}
			}
		} else if (cmd.compare("spawn")==0) {

			vector<string> vArguments;
			SpadeScriptEngine::explode(',', argu, vArguments);

			// '?' means spawn instance with random, unique name.  Clear it so its compatible with spawn length() check below
			if(SpadeScriptEngine::safe_lookup(vArguments, 0, "")=="?") vArguments[0] = "";

			unsigned int idx = SpadeScriptEngine::safe_lookup(vArguments, 0, "").find(".");

			// get spawn arguments
			string SpawnArgs = SpadeScriptEngine::safe_lookup(vArguments, 1, "");
			
			if(SpadeScriptEngine::safe_lookup(vArguments,0,"").length()==0) {
				// spawn with random, unique name
				context->stack.top().vars[string("_result")] = spawn("",context,SpawnArgs);
			}
			else if(idx == string::npos) { // no dots
				string c_inst = SpadeScriptEngine::safe_lookup(vArguments,0,"");

				if(m_pEngine->isScript(c_inst)) {
					context->stack.top().vars[string("_result")] =
						m_pEngine->getScript(c_inst)->spawn("",context);
				}
				else if(m_InstanceVariables.find(c_inst) == m_InstanceVariables.end())
					spawn(c_inst,context,SpawnArgs);
				else
					{ return 1; }
			}
			else // 1 or more dots
			{
				string c_script = SpadeScriptEngine::safe_lookup(vArguments,0,"").substr(0,idx);
				string c_inst = SpadeScriptEngine::safe_lookup(vArguments,0,"").substr(idx+1);

				if(m_pEngine->isScript(c_script) && m_InstanceVariables.find(c_inst) == m_InstanceVariables.end())
					m_pEngine->getScript(c_script)->spawn(c_inst,context,SpawnArgs);
				else
					{ return 1; }
			}

		} else if (cmd.compare("remove")==0) {

			unsigned int idx = argu.find(".");

			if(idx == string::npos) {
				string c_inst = argu.substr(0,idx);
				remove(c_inst,context);
			}
			else // 1 or more dots
			{
				string c_script = argu.substr(0,idx);
				string c_inst = argu.substr(idx+1);

				if(m_pEngine->isScript(c_script))
					m_pEngine->getScript(c_script)->remove(c_inst,context);
				else
					{ return 1; }
			}

		} /* else if (cmd.compare("macro")==0) {

			// Local function call
			
			//context->stack.push(SpadeScriptStructure(cmd,"function"));
			int ret = 0;
			string callresult = "";
			ret = call(cmd, context, callresult, inst, "");
			if(ret!=1)
				return ret;
			//context->stack.pop();
			//context->stack.top().vars[string("_result")] = callresult;

			} */
		
		else if (cmd.compare("include")==0) {
			
			// TODO: Dynamic script loading (coming soon?)

			/*string args = "";
			string override_name = "";
			vector<string> include_args;

			unsigned int first = line.find_first_of(" \t");

			if(first != string::npos) {
				args = line.substr(first);
				args = SpadeScriptEngine::trim(args, " \t()");
				SpadeScriptEngine::explode(' ',args,include_args);
			}

			if(SpadeScriptEngine::safe_lookup(include_args,1,"").compare("as")==0) {
				override_name = SpadeScriptEngine::safe_lookup(include_args,2,"");
			}

			if(!eng->allocate(SpadeScriptEngine::safe_lookup(include_args,0,""),override_name))
				throw "File inclusion error.";*/

		} else {// if(m_Functions.find(cmd) != m_Functions.end()) {

			// Local function call
		
			context->stack.push(SpadeScriptStructure(cmd,"function"));
			string callresult = "";
			
			if(!call(cmd, context, callresult, inst, argu))
				return 0;
			context->stack.pop();
			context->stack.top().vars[string("_result")] = callresult;

			//cout<<"Unrecognized Command: "<<endl;
			return 0;
		}
	}
	result = r;

	return 1;
}

bool SpadeScript :: load(const string& fn, SpadeScriptEngine* eng, bool use_file, string override_name) {

	// Load script without file
	if(!use_file) {
		m_pEngine = eng;
		if(m_pEngine) {
			if(!eng->addScript(this,fn))
				return false;
		}
		return (m_pEngine!=NULL);
	}

	// Load script from file
	int i=0;
	try {
		//stack<string> fstack;

		ifstream file(fn.c_str());
		if(!file.is_open())
			throw "Error opening script file.";

		int block_depth = 0;
		string FunctionName = "";

		// Get script name from file name
		if(override_name.length()==0)
			m_sName = fn.substr(0,fn.find('.'));
		else
			m_sName = override_name;

		while(!file.eof()) {
			
			string line = SpadeScriptEngine::readLine(file); i++;
			line = SpadeScriptEngine::trim(line," \t");
			if(line.length()==0) // ignore blank lines
				continue;
			if(SpadeScriptEngine::stringBeginsWith(SpadeScriptEngine::trim(line," \t"), "#")) //ignore comments
				continue;

			istringstream iss(line);
			string token = "";

			// tokenize on standard commands
			if (SpadeScriptEngine::stringBeginsWith(line,"function")) {
				//cout << "FUNCTION HERE" << endl;

				//string args = line.substr(line.find("(")+1, line.find(")")-line.find("(")-1);
				//while(args.find(" ")!=string::npos)
				//	args.erase(args.find(" "),1);

				string args = "";
				unsigned int first = line.find_first_of("(");

				if(first != string::npos) {
					args = line.substr(first);
					args = SpadeScriptEngine::trim(args, "\t()");
				}

				string fname = line.substr(line.find_first_of(" \t")+1, line.find_first_of("(")-1-line.find_first_of(" \t"));
				fname = SpadeScriptEngine::trim(fname, " \t");
				
				if(SpadeScriptEngine::stringEndsWith(line, "{")) {
					//FunctionName = fname;
					block_depth++;
					FunctionName = fname;
					//fstack.push(fname);
					//cout << "PUSH " << args << endl;
				}
				else
				{
					line = SpadeScriptEngine::readLine(file); i++; // read next line
					line = SpadeScriptEngine::trim(line," \t"); // trim
					if(SpadeScriptEngine::stringBeginsWith(line, "{")) {
						block_depth++;
						FunctionName = fname;
						//fstack.push(fname);
						//cout << "PUSH " << args << endl;
					}
					else
						throw "Expected '{' symbol";
				}

				m_Functions[fname] = vector<string>(); // new function definition
				if(args.length()>0)
					m_Functions[fname].push_back(string("_args " + args)); // argument names

			} else if(SpadeScriptEngine::stringBeginsWith(line,"include")) {

				string args = "";
				string override_name = "";
				vector<string> include_args;

				unsigned int first = line.find_first_of(" \t");

				if(first != string::npos) {
					args = line.substr(first);
					args = SpadeScriptEngine::trim(args, " \t()");
					SpadeScriptEngine::explode(' ',args,include_args);
				}

				if(SpadeScriptEngine::safe_lookup(include_args,1,"").compare("as")==0) {
					override_name = SpadeScriptEngine::safe_lookup(include_args,2,"");
				}

				if(!eng->allocate(SpadeScriptEngine::safe_lookup(include_args,0,""),override_name))
					throw "File inclusion error.";

			} else if (SpadeScriptEngine::stringBeginsWith(line,"{")) {
				block_depth++;
				m_Functions[FunctionName].push_back(line);
			} else if (SpadeScriptEngine::stringBeginsWith(line,"}")) {
				block_depth--;

				if(block_depth==0)
					FunctionName = "";
				else
					m_Functions[FunctionName].push_back(line);
			} else {
				if(block_depth == 0)
					throw "Comand outside of function.";
				m_Functions[FunctionName].push_back(line);
			}
		}
		
		//if(!fstack.empty())
		//	throw "Script syntax error. Check brackets.";
		if(block_depth != 0)
			throw "Script syntax error. Check brackets. ";

		m_pEngine = eng;
		if(!eng->addScript(this, m_sName))
			throw "Failed to add script '" + fn + "' to engine.";

	} catch (const char* err) {
		cout << "Error (" << fn << ", line " << i << "): " << err << endl;
		return false;
	}
	
	//m_StaticVariables[];
	return true;
}

// substitute variables and function calls to their actual values

bool SpadeScript :: substitute(const string& expression, string& result, SpadeScriptContext* c, const string& inst)
{
	string exp = expression;
	unsigned int i = 0;
	unsigned int j = 0;

	/*do
	{
		i = exp.find_last_of("~");
		j = 0;
		string varname = "";
		string varvalue = ""
	} while(i != string.npos);*/

	do
	{
		i = exp.find_last_of("$");
		j = 0;
		string varname = "";
		string varvalue = "";

		if(i != string::npos) {
			j = exp.find("}", i+2); // i+1

			// evaluate function
			if(exp[j-1]==')') { /* evaluate function */ }

			varname = exp.substr(i+2, j-i-2); //exp.substr(i+1,j-i-1); 
			//varname = SpadeScriptEngine::trim(varname,"%");
			//cout<<"Variable name is "<<varname<<endl;
			unsigned int idx = varname.find(".");

			// ways to access variables:
			// ${variable} = local variable, current script, current object
			// ${script.variable} = static variable of class 'script'
			// ${script.default.variable} = same as above
			// ${script.instance.variable} = specific variable belonging to instance

			// Context variable
			if(idx == string::npos) // no dot
			{
				//if(varname.compare("_random")!=0)
					varvalue = c->getVariable(varname); // context variable
				//else {
				//}
			}
			else
			{
				// object variable (instance or static)

				// ${.variable} = local variable of current script, current object
				if(varname[0]=='.')
				{
					string sel_var = varname.substr(1);
					varvalue = m_InstanceVariables[inst][sel_var]; // this instance
				}
				else 
				{
					int n = 0;

					// Count num of '.' occuranaces in varname
					for(unsigned int i = 0; i < varname.size(); i++)
						if(varname[i] == '.')
							n++;

					// ${script.instance.variable} = specific variable belonging to specific script and instance
					// if 2 dots...   script.instance.variable
					if(n==2)
					{
						unsigned int jdx = varname.find(".",idx+1);
						string sel_script = varname.substr(0,idx);
						string sel_inst = varname.substr(idx+1,jdx-(idx+1));
						string sel_var = varname.substr(jdx+1);
						if(m_pEngine->isScript(sel_script)) {
							// get script variable
							varvalue = m_pEngine->getScript(sel_script)->getVariable(sel_var, sel_inst);
						} else {
							// Error: 
						}
					}
					else if(n==1) // 1 dot
					{
						// script or instance name
						string sel_inst = varname.substr(0,idx); // instance or script name
						string sel_var = varname.substr(idx+1); // variable name

						if(m_pEngine->isScript(sel_inst)) {
							// ${script.variable} = static (default) variable of class 'script'
							varvalue = m_pEngine->getScript(sel_inst)->getVariable(sel_var);
						} else {
							
							// is instance name = default?  if so, so simply get
							// the current script's static variable
							if(sel_inst.compare("default")==0)
								varvalue = m_StaticVariables[sel_var];
							else
							{
								// instance name is first and specific! ( instance.variable )
								// This script, specific instance, specific variable
								varvalue = m_InstanceVariables[sel_inst][sel_var];
							}
						}
					}
					else // if more than 2 dots... error
					{
						// error
					}
				}

				
				/*string::iterator itr;
				if((itr = m_StaticVariables.find(varname) != string::npos) {
					varvalue = m_StaticVariables[varname];
				} else if(itr = m_InstanceVariables.find(varname))!=string::npos) {
					varvalue = m_InstanceVariables[varname];
				} else {
					// must be part of the engine.  if not, return false
				}*/
			}
			
			// Get variable value
			exp = exp.substr(0,i) + varvalue + exp.substr(j+1);

		}

	} while(i != string::npos);

	result = exp;
	return true;
}


string SpadeScript :: spawn(const string& InstanceName,
		SpadeScriptContext* context,
		string args,
		const string CopyFrom)
{
	// if instance already exists
	if(m_InstanceVariables.find(InstanceName)!=m_InstanceVariables.end()) {
		// error
		return "";
	}

	// Create context if not given one
	auto_ptr<SpadeScriptContext> pContext;
	if(!context) {
		pContext->start();
		context = pContext.get();
	}

	string Name = InstanceName;

	// If no name given, generate name
	if(InstanceName.length()==0) {
		unsigned int id = 0;
		do{
			id = rand() % 100000;
			stringstream ss;
			ss << id;
			Name = ss.str();
		}while(m_InstanceVariables.find(Name) != m_InstanceVariables.end()); // while var already exists
	}

	m_InstanceVariables[Name] = map<string,string>();
	m_InstanceVariables[Name]["_instance"] = Name;
	m_InstanceVariables[Name]["_script"] = getName();

	onSpawn(Name,args);

	if(context && m_Functions.find("_") != m_Functions.end()) {
		context->stack.push(SpadeScriptStructure("_","function"));
		string callresult = "";
		
		string r = "";
		if(! call("_", context, r, Name, ""))
			return "";

		context->stack.pop();
		//context->stack.top().vars[string("_result")] = callresult;
	}

	return Name;
}

bool SpadeScript :: remove(const string InstanceName, SpadeScriptContext* context) {
	
	// Create context if not given one
	/*auto_ptr<SpadeScriptContext> pContext;
	if(!context) {
		pContext->start();
		context = pContext.get();
	}*/

	// Remove the specific instance
	string Name = InstanceName;
	map<string,map<string,string>>::iterator itr = m_InstanceVariables.find(InstanceName);
	if(itr != m_InstanceVariables.end()) {

		// Call Deconstructor
		if(context && m_Functions.find("~") != m_Functions.end()) {
			context->stack.push(SpadeScriptStructure("~","function"));
			string r = "";
			int rv = call("~", context, r, Name, "");
			context->stack.pop();
			if(rv==0)
				return false;
		}

		onRemove(Name);
		m_InstanceVariables.erase(itr);

		return true;
	}
	return false;
}

bool SpadeScript :: removeAll(SpadeScriptContext* context)
{
	// Create context if not given one
	/*auto_ptr<SpadeScriptContext> pContext;
	if(!context) {
		pContext->start();
		context = pContext.get();
	}*/

	// Remove the specific instance
	for(map<string,map<string,string>>::iterator itr = m_InstanceVariables.begin();
		itr != m_InstanceVariables.end();
		itr ++)
	{
		// Call Deconstructor
		if(context && m_Functions.find("~") != m_Functions.end()) {
			context->stack.push(SpadeScriptStructure("~","function"));
			string r = "";
			int rv = call("~", context, r, itr->first, "");
			context->stack.pop();
			if(rv==0)
				return false;
		}

		onRemove(itr->first);
		//m_InstanceVariables.erase(itr);

		return true;
	}

	m_InstanceVariables.clear();

	return true;
}
