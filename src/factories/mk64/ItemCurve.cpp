#include "ItemCurve.h"
#include "spdlog/spdlog.h"

#include "Companion.h"
#include "utils/Decompressor.h"

#define NUM(x) std::dec << std::setfill(' ') << std::setw(6) << x
#define COL(c) "0x" << std::hex << std::setw(2) << std::setfill('0') << c

ExportResult MK64::ItemCurveHeaderExporter::Export(std::ostream &write, std::shared_ptr<IParsedData> raw, std::string& entryName, YAML::Node &node, std::string* replacement) {
    const auto symbol = GetSafeNode(node, "symbol", entryName);

    if(Companion::Instance->IsOTRMode()){
        write << "static const char " << symbol << "[] = \"__OTR__" << (*replacement) << "\";\n\n";
        return std::nullopt;
    }

    write << "extern u8 " << symbol << "[];\n";
    return std::nullopt;
}

static const std::vector<std::string> ItemStrings = {
    "ITEM_NONE", 
    "ITEM_BANANA",
    "ITEM_BANANA_BUNCH",
    "ITEM_GREEN_SHELL",
    "ITEM_TRIPLE_GREEN_SHELL",
    "ITEM_RED_SHELL",
    "ITEM_TRIPLE_RED_SHELL",
    "ITEM_BLUE_SPINY_SHELL",
    "ITEM_THUNDERBOLT",
    "ITEM_FAKE_ITEM_BOX",
    "ITEM_STAR",
    "ITEM_BOO",
    "ITEM_MUSHROOM",
    "ITEM_DOUBLE_MUSHROOM",
    "ITEM_TRIPLE_MUSHROOM",
    "ITEM_SUPER_MUSHROOM",
};

ExportResult MK64::ItemCurveCodeExporter::Export(std::ostream &write, std::shared_ptr<IParsedData> raw, std::string& entryName, YAML::Node &node, std::string* replacement ) {
    auto items = std::static_pointer_cast<ItemCurveData>(raw)->mItems;
    const auto symbol = GetSafeNode(node, "symbol", entryName);
    const auto offset = GetSafeNode<uint32_t>(node, "offset");

    const auto searchTable = Companion::Instance->SearchTable(offset);

    if(searchTable.has_value()){
        const auto [name, start, end, mode] = searchTable.value();

        if(start == offset){
            write << GetSafeNode<std::string>(node, "ctype", "const u8") << " " << name << "[][" << items.size() << "] = {\n";
        }

        write << fourSpaceTab << "{";
        for (size_t i = 0; i < items.size(); ++i) {
            uint8_t value = items[i];

            if (i % 10 == 0) {
                write << "\n" << fourSpaceTab << fourSpaceTab << ItemStrings[value] << ", ";
            } else {
                write << ItemStrings[value] << ", ";
            }
        }
        write << "\n" << fourSpaceTab << "},\n";

        if(end == offset){
            write << "};\n";
        }

    } else {

        write << "u8 " << symbol << "[] = {";


        for (size_t i = 0; i < items.size(); ++i) {
            uint8_t value = items[i];

            if (i % 10 == 0) {
                write << "\n" << fourSpaceTab << ItemStrings[value] << ", ";
            } else {
                write << ItemStrings[value] << ", ";
            }
        }
        write << "\n};\n";
    }

    return offset + items.size() * sizeof(uint8_t);
}

ExportResult MK64::ItemCurveBinaryExporter::Export(std::ostream &write, std::shared_ptr<IParsedData> raw, std::string& entryName, YAML::Node &node, std::string* replacement ) {
    throw std::runtime_error("Decomp ItemCurve is only implemented in decomp.\nuk64 and port use a new system for ease of modding and bug fixes.");
    return std::nullopt;
}

std::optional<std::shared_ptr<IParsedData>> MK64::ItemCurveFactory::parse(std::vector<uint8_t>& buffer, YAML::Node& node) {
    auto [_, segment] = Decompressor::AutoDecode(node, buffer);
    LUS::BinaryReader reader(segment.data, (10 * 10) * sizeof(uint8_t));

    reader.SetEndianness(LUS::Endianness::Big);
    std::vector<uint8_t> items;

    // Each array is size of 10*10.
    for(size_t i = 0; i < 10*10; i++) {
        items.push_back(reader.ReadUByte());
    }

    return std::make_shared<ItemCurveData>(items);
}