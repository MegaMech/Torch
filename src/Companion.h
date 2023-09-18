#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "factories/RawFactory.h"

class Companion {
public:
    static Companion* Instance;
    explicit Companion(std::filesystem::path  rom) : gRomPath(std::move(rom)) {}
    void Init();
    void Process();
private:
    std::filesystem::path gRomPath;
    std::vector<uint8_t> gRomData;
    std::unordered_map<std::string, RawFactory*> gFactories;

    void RegisterFactory(const std::string& type, RawFactory* factory);
    RawFactory* GetFactory(const std::string& type);
};