#include "VAsset.hpp"

namespace ApplicationCore{
    template <typename T>
    inline VAsset<T>::VAsset(const std::string &name)
    {
        ID++;
    }
}