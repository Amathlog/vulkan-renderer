#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace VulkanRenderer
{
namespace Utils
{
// Validate that all inNames are present in others
// others can be any type (basically any Vulkan structure), with a callback to get the string
// from it.
// Return inNamesCount if all found, otherwise, return the index of the first not found item
template <typename T, typename Func>
int ValidateStrings(const char* const* inNames, uint32_t inNamesCount, const T* others, uint32_t othersCount,
                    Func getStringCallback)
{
    for (uint32_t i = 0; i < inNamesCount; ++i)
    {
        bool found = false;
        for (uint32_t j = 0; j < othersCount; ++j)
        {
            if (std::strcmp(getStringCallback(others[j]), inNames[i]) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return i;
        }
    }

    return inNamesCount;
}
} // namespace Utils
} // namespace VulkanRenderer