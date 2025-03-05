#include <Compiler.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

int main()
{
    const std::string fileName = "semantictest.mano";
    std::ifstream file(fileName);
    if (!file)
    {
        std::cerr << "Failed to open " << fileName << "\n";
        return 1;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Arcanelab::Mano::Compiler compiler;
    compiler.Run(source);
    
    return 0;
}
