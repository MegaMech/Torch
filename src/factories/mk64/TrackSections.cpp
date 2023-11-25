#include "TrackSections.h"

#include "Companion.h"
#include "spdlog/spdlog.h"
#include "utils/MIODecoder.h"

#define NUM(x) std::dec << std::setfill(' ') << std::setw(6) << x
#define COL(c) "0x" << std::hex << std::setw(2) << std::setfill('0') << c

void MK64::TrackSectionsHeaderExporter::Export(std::ostream &write, std::shared_ptr<IParsedData> raw, std::string& entryName, YAML::Node &node, std::string* replacement) {
    const auto symbol = node["symbol"] ? node["symbol"].as<std::string>() : entryName;

    if(Companion::Instance->IsOTRMode()){
        write << "static const char " << symbol << "[] = \"__OTR__" << (*replacement) << "\";\n\n";
        return;
    }

    write << "extern TrackSections " << symbol << "[];\n";
}

void MK64::TrackSectionsCodeExporter::Export(std::ostream &write, std::shared_ptr<IParsedData> raw, std::string& entryName, YAML::Node &node, std::string* replacement ) {
    auto sections = std::static_pointer_cast<TrackSectionsData>(raw)->mSecs;

    if (!node["symbol"]) {
        SPDLOG_ERROR("Asset in yaml missing entry for symbol.\nEx. symbol: myVariableNameHere");
        return;
    }

    const auto symbol = node["symbol"].as<std::string>();
    const auto offset = node["offset"].as<uint32_t>();

    if (Companion::Instance->IsDebug()) {
        write << "// 0x" << std::hex << std::uppercase << offset << "\n";
    }

    write << "TrackSections " << symbol << "[] = {\n";

    for (int i = 0; i < sections.size(); i++) {
        auto entry = sections[i];

        auto addr = entry.addr;
        auto surf = entry.surfaceType;
        auto sect = entry.sectionId;
        auto flags = entry.flags;

        if(i <= sections.size() - 1) {
            write << fourSpaceTab;
        }

        // { addr, surface, section, flags },
        write << "{" << NUM(addr) << ", " << NUM(surf) << ", " << NUM(sect) << ", " << NUM(flags) << "},\n";
    }

    if (Companion::Instance->IsDebug()) {
        write << fourSpaceTab << "// 0x" << std::hex << std::uppercase << (offset + (16 * sections.size())) << "\n";
    }

    write << "};\n\n";
}

void MK64::TrackSectionsBinaryExporter::Export(std::ostream &write, std::shared_ptr<IParsedData> raw, std::string& entryName, YAML::Node &node, std::string* replacement ) {
    auto sections = std::static_pointer_cast<TrackSectionsData>(raw);
    auto writer = LUS::BinaryWriter();

    WriteHeader(writer, LUS::ResourceType::Vertex, 0);
    writer.Write((uint32_t) sections->mSecs.size());
    for(auto entry : sections->mSecs) {
        writer.Write(entry.addr);
        writer.Write(entry.surfaceType);
        writer.Write(entry.sectionId);
        writer.Write(entry.flags);
    }

    writer.Finish(write);
}

std::optional<std::shared_ptr<IParsedData>> MK64::TrackSectionsFactory::parse(std::vector<uint8_t>& buffer, YAML::Node& node) {
    
    if (!node["mio0"]) {
        SPDLOG_ERROR("yaml missing entry for mio0.\nEx. mio0: 0x10100");
        return nullptr;
    }

    if (!node["offset"]) {
        SPDLOG_ERROR("yaml missing entry for offset.\nEx. offset: 0x100");
        return nullptr;
    }

    if (!node["count"]) {
        SPDLOG_ERROR("Asset in yaml missing entry for vtx count.\nEx. count: 30\nThis means 30 vertices (an array size of 30).");
        return nullptr;
    }
    
    auto mio0 = node["mio0"].as<size_t>();
    auto offset = node["offset"].as<uint32_t>();
    auto count = node["count"].as<size_t>();

    auto decoded = MIO0Decoder::Decode(buffer, mio0);
    LUS::BinaryReader reader(decoded.data() + offset, count * sizeof(MK64::TrackSections) );

    reader.SetEndianness(LUS::Endianness::Big);
    std::vector<MK64::TrackSections> sections;

    for(size_t i = 0; i < count; i++) {
        auto addr = reader.ReadUInt32();
        auto surf = reader.ReadInt8();
        auto sect = reader.ReadInt8();
        auto flags = reader.ReadUInt16();

        sections.push_back(MK64::TrackSections({
           addr, surf, sect, flags,
       }));
    }

    return std::make_shared<TrackSectionsData>(sections);
}