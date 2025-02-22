//
// Created by wpsimon09 on 24/01/25.
//

#ifndef APPLICATIONUTILS_HPP
#define APPLICATIONUTILS_HPP
#include <vector>
namespace VulkanStructs{
    struct Bounds;
}
namespace ApplicationCore{
    struct Vertex;
    
    
    VulkanStructs::Bounds CalculateBounds(const std::vector<ApplicationCore::Vertex>& vertices);
}

template<typename T, typename Compare, typename Swap>
int Partition(std::vector<T>& arr, int low, int high, Compare comp,Swap swap)
{
    T pivot = arr[high];
    int i = low - 1;
    for (int j = low; j <= high; j++)
    {
        if (comp(arr[j],pivot))
        {
            i++;
            swap(arr[i], arr[j]);
        }

    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}


template<typename T, typename Compare, typename Swap>
void QuickSort(std::vector<T>& arr,  int low, int high, Compare comp, Swap swap)
{
    if (low<high)
    {
        int pi  = Partition(arr, low, high, comp, swap);
        QuickSort(arr, low, pi, comp, swap);
        QuickSort(arr, pi + 1, high, comp, swap);
    }
}

template<typename T, typename Compare>
int Partition(std::vector<T>& arr, int low, int high, Compare comp)
{
    T pivot = arr[high];
    int i = low - 1;
    for (int j = low; j <= high; j++)
    {
        if (comp(arr[j],pivot))
        {
            i++;
            std::swap(arr[i], arr[j]);
        }

    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}


template<typename T, typename Compare>
void QuickSort(std::vector<T>& arr,  int low, int high, Compare comp)
{
    if (low<high)
    {
        int pi  = Partition(arr, low, high, comp);
        QuickSort(arr, low, pi, comp);
        QuickSort(arr, pi + 1, high, comp);
    }
}





#endif //APPLICATIONUTILS_HPP
