//
// Created by simon on 14/01/2026.
//

#ifndef VULKAN_RTX_INPUTPARSER_HPP
#define VULKAN_RTX_INPUTPARSER_HPP
#include <algorithm>
#include <string>
#include <vector>

namespace CLI {


// Source - https://stackoverflow.com/a
// Posted by iain, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-14, License - CC BY-SA 3.0
class InputParser
{
  public:
    InputParser(int& argc, char** argv)
    {
        for(int i = 1; i < argc; ++i)
            this->tokens.push_back(std::string(argv[i]));
    }
    /// @author iain
    const std::string& getCmdOption(const std::string& option) const
    {
        std::vector<std::string>::const_iterator itr;
        itr = std::find(this->tokens.begin(), this->tokens.end(), option);
        if(itr != this->tokens.end() && ++itr != this->tokens.end())
        {
            return *itr;
        }
        static const std::string empty_string("");
        return empty_string;
    }
    /// @author iain
    bool cmdOptionExists(const std::string& option) const
    {
        return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
    }

  private:
    std::vector<std::string> tokens;
};

int main(int argc, char** argv)
{
    InputParser input(argc, argv);
    if(input.cmdOptionExists("-h"))
    {
        // Do stuff
    }
    const std::string& filename = input.getCmdOption("-f");
    if(!filename.empty())
    {
        // Do interesting things ...
    }
    return 0;
}


}  // namespace CLI

#endif  //VULKAN_RTX_INPUTPARSER_HPP
