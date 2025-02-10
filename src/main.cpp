#include <Compiler.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

int main()
{
    std::ifstream file("test.mano");
    if (!file)
    {
        std::cerr << "Failed to open test.mano" << "\n";
        return 1;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Arcanelab::Mano::Compiler compiler;
    compiler.Run(source);
    
    return 0;
}
