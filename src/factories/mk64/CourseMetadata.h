#pragma once

#include "../BaseFactory.h"

namespace MK64 {

    struct BombKartSpawns {
        uint16_t waypointIndex;
        uint16_t startingState;
        float unk_04;
        float x;
        float z;
        float unk10;
        float unk14;
    };

    struct CourseMetadata {
        uint32_t id;
        std::string name;
        std::string debugName;
        std::string cup;
        uint32_t cupIndex;
        std::string waypointWidth;
        std::string waypointWidth2;
        std::string D_800DCBB4;
        uint32_t steeringSensitivity;
        std::vector<BombKartSpawns> bombKartSpawns;
        std::vector<uint16_t> pathSizes;
        std::vector<std::string> D_0D009418;
        std::vector<std::string> D_0D009568;
        std::vector<std::string> D_0D0096B8;
        std::vector<std::string> D_0D009808;
        std::vector<std::string> pathTable;
        std::vector<std::string> pathTableUnknown;
        std::vector<int16_t> skyColors;
        std::vector<int16_t> skyColors2;
    };

    class MetadataData : public IParsedData {
    public:
        std::vector<CourseMetadata> mMetadata;

        explicit MetadataData(std::vector<CourseMetadata> metadata) : mMetadata(metadata) {}
    };

    class CourseMetadataHeaderExporter : public BaseExporter {
        void Export(std::ostream& write, std::shared_ptr<IParsedData> data, std::string& entryName, YAML::Node& node, std::string* replacement) override;
    };

    class CourseMetadataBinaryExporter : public BaseExporter {
        void Export(std::ostream& write, std::shared_ptr<IParsedData> data, std::string& entryName, YAML::Node& node, std::string* replacement) override;
    };

    class CourseMetadataCodeExporter : public BaseExporter {
        void Export(std::ostream& write, std::shared_ptr<IParsedData> data, std::string& entryName, YAML::Node& node, std::string* replacement) override;
    };

    class CourseMetadataFactory : public BaseFactory {
    public:
        std::optional<std::shared_ptr<IParsedData>> parse(std::vector<uint8_t>& buffer, YAML::Node& data) override;
        std::optional<std::shared_ptr<IParsedData>> parse_modding(std::vector<uint8_t>& buffer, YAML::Node& data) override {
            return std::nullopt;
        }
        inline std::unordered_map<ExportType, std::shared_ptr<BaseExporter>> GetExporters() override {
            return {
                REGISTER(Code, CourseMetadataCodeExporter)
                REGISTER(Header, CourseMetadataHeaderExporter)
                REGISTER(Binary, CourseMetadataBinaryExporter)
            };
        }
        bool SupportModdedAssets() override { return false; }
    };
}