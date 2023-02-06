#pragma once

#include <parser/parameters/CommandLineParameters.h>

namespace VulkanRenderer
{
struct VulkanParameters : bsc::CommandLineParameters
{
    bsc::Flag verbose = {{.shortKey = 'v', .longKey = "verbose", .doc = "Enable verbose log"}};
    bsc::Parameter<int> forceSelectedDevice = {
        {.longKey = "device",
         .argumentName = "DEVICE",
         .doc = "Force given physical device. Use verbose to know order of devices."}};

    const char* execPath = "";

    static void Initialize(int argc, char* argv[])
    {
        if (s_instance != nullptr)
            return;

        static bsc::CommandLineParser parser;
        s_instance = new VulkanParameters();
        s_instance->parser->prepareParser(parser.parseConfiguration, parser.parser);
        s_instance->parser->parse(argc, argv);

        if (argc > 0)
            s_instance->execPath = argv[0];
    }

    ~VulkanParameters() { delete s_instance; }

    static const VulkanParameters& GetInstance() { return *s_instance; }

protected:
    VulkanParameters() = default;

    inline static VulkanParameters* s_instance = nullptr;
};

inline const VulkanParameters& Parameters() { return VulkanParameters::GetInstance(); }
} // namespace VulkanRenderer
