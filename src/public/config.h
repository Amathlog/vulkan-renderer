#pragma once

#include <parser/parameters/CommandLineParameters.h>

namespace VulkanRenderer
{
struct VulkanParameters : bsc::CommandLineParameters
{
    bsc::Flag verbose = {{.shortKey = 'v', .longKey = "verbose", .doc = "Enable verbose log"}};

    static void Initialize(int argc, char* argv[])
    {
        if (s_instance != nullptr)
            return;

        static bsc::CommandLineParser parser;
        s_instance = new VulkanParameters();
        s_instance->parser->prepareParser(parser.parseConfiguration, parser.parser);
        s_instance->parser->parse(argc, argv);
    }

    ~VulkanParameters() { delete s_instance; }

    static VulkanParameters& GetInstance() { return *s_instance; }

protected:
    VulkanParameters() = default;

    inline static VulkanParameters* s_instance = nullptr;
};
} // namespace VulkanRenderer
