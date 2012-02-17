
#include <iostream>
#include <sstream>
using namespace std;
#include "../SpadeScript/SpadeScriptEngine.h"
#include "../SpadeScript/SpadeScriptDataSet.h"

#include <ctime>

#include <memory>

string getTime()
{
    time_t now;
    time ( &now );
    stringstream stream;
    stream << time( &now ) << flush;
    return stream.str();
}

int main(int argc, char** argv)
{
    auto_ptr<SpadeScriptEngine> engine(new SpadeScriptEngine());
    auto_ptr<SpadeScript> object(new SpadeScript());
    auto_ptr<SpadeScriptDataSet> dataset(new SpadeScriptDataSet());
    auto_ptr<SpadeScriptContext> defaultContext(new SpadeScriptContext());

    dataset->load("scripts/dataset.lib.ssf", engine.get());

    string script = "";

    cout<<"Spade Script"<<endl;
    cout<<"Filename> ";
    getline(cin, script);

    if(object->load(script, engine.get()))
        cout<<"Started at: "<<getTime()<<endl;
    else
        return 0;

    defaultContext->start();
    string r = "";
    if(object->call("main", defaultContext.get(), r, "default")) {
        cout << "Script completed successfully. at " <<getTime()<< endl;
    }
    defaultContext->stop();

    system("pause");

    return 0;
}
