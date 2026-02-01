//
// Created by wpsimon09 on 24/01/25.
//

#ifndef APPLICATIONUTILS_HPP
#define APPLICATIONUTILS_HPP
#include <string>
#include <vector>

#include "Application/Enums/ClientEnums.hpp"

#include <cstdint>
#include <istream>

namespace VulkanStructs {
struct VBounds;
}
namespace ApplicationCore {
struct Vertex;


VulkanStructs::VBounds CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices);
}  // namespace ApplicationCore

template <typename T, typename Compare, typename Swap>
int Partition(std::vector<T>& arr, int low, int high, Compare comp, Swap swap)
{
    T   pivot = arr[high];
    int i     = low - 1;
    for(int j = low; j <= high; j++)
    {
        if(comp(arr[j], pivot))
        {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}


template <typename T, typename Compare, typename Swap>
void QuickSort(std::vector<T>& arr, int low, int high, Compare comp, Swap swap)
{
    if(low < high)
    {
        int pi = Partition(arr, low, high, comp, swap);
        QuickSort(arr, low, pi, comp, swap);
        QuickSort(arr, pi + 1, high, comp, swap);
    }
}

template <typename T, typename Compare>
int Partition(std::vector<T>& arr, int low, int high, Compare comp)
{
    T   pivot = arr[high];
    int i     = low - 1;
    for(int j = low; j <= high; j++)
    {
        if(comp(arr[j], pivot))
        {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}


template <typename T, typename Compare>
void QuickSort(std::vector<T>& arr, int low, int high, Compare comp)
{
    if(low < high)
    {
        int pi = Partition(arr, low, high, comp);
        QuickSort(arr, low, pi, comp);
        QuickSort(arr, pi + 1, high, comp);
    }
}

std::string ThemeToString(ETheme theme);

bool StringContains(const std::string& text, const std::string& search);


inline void WriteString(std::ostream& out, const std::string& s)
{
    unsigned size = s.size();
    out.write(reinterpret_cast<const char*>(&size), sizeof(unsigned));
    out.write(s.c_str(), size * sizeof(char));
}

/*
 * Reads string from the stream
 */
inline void ReadString(std::istream& in, std::string& s)
{
    unsigned stringsize;
    in.read(reinterpret_cast<char*>(&stringsize), sizeof(unsigned));
    if(stringsize == 0)
    {
        s = std::string();
        return;
    }
    std::vector<char> temp(stringsize);
    in.read(reinterpret_cast<char*>(&temp[0]), stringsize * sizeof(char));
    s = std::string(temp.begin(), temp.end());
}


template <typename T>
inline void WritePod(std::ostream& out, const T& v)
{
    static_assert(std::is_trivially_copyable_v<T>);
    out.write(reinterpret_cast<const char*>(&v), sizeof(T));
}


template <typename T>
inline void ReadPod(std::istream& in, T& v)
{
    static_assert(std::is_trivially_copyable_v<T>);
    in.read(reinterpret_cast<char*>(&v), sizeof(T));
}

#endif  //APPLICATIONUTILS_HPP
