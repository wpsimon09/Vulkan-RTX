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
    const std::string& getCmdOption(const std::string& option, const std::string& shorthand) const
    {
        std::vector<std::string>::const_iterator itr;

        // Try long option first
        itr = std::find(tokens.begin(), tokens.end(), option);
        if(itr != tokens.end() && ++itr != tokens.end())
        {
            return *itr;
        }

        // Try shorthand if provided
        if(!shorthand.empty())
        {
            itr = std::find(tokens.begin(), tokens.end(), shorthand);
            if(itr != tokens.end() && ++itr != tokens.end())
            {
                return *itr;
            }
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


}  // namespace CLI

#endif  //VULKAN_RTX_INPUTPARSER_HPP
