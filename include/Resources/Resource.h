#pragma once

#include <string>
#include <vector>

#include "Glacier/Resource/SResourceHeaderHeader.h"
#include "Glacier/Resource/ZRuntimeResourceID.h"
#include "IO/BinaryReader.h"

class Resource
{
public:
    Resource();
    ~Resource();
    const std::string GetResourceLibraryFilePath() const;
    const std::string GetName() const;
    const SResourceHeaderHeader& GetResourceHeaderHeader() const;
    const ZRuntimeResourceID& GetRuntimeResourceID() const;
    std::string GetResourceID() const;
    const unsigned int GetOffsetInResourceLibrary() const;
    const void* GetResourceData() const;
    void* GetResourceData();
    const unsigned int GetResourceDataSize() const;
    void GetReferences(std::vector<Resource*>& references) const;
    void GetReferences(std::vector<Resource*>& references);
    void SetResourceLibraryFilePath(std::string resourceLibraryFilePath);
    void SetName(std::string name);
    void SetResourceHeaderHeader(const SResourceHeaderHeader& resourceHeaderHeader);
    void SetRuntimeResourceID(const ZRuntimeResourceID& runtimeResourceID);
    void SetResourceID(std::string resourceID);
    void SetResourceDataSize(const unsigned int resourceDataSize);
    void SetOffsetInResourceLibrary(const unsigned int offsetInResourceLibrary);
    void LoadResourceData();
    void DeleteResourceData();
    void LoadResource(BinaryReader& headerLibraryBinaryReader, const unsigned int resourceHeaderOffset);
    void LoadReferences(const BinaryReader& headerDataBinaryReader);

protected:
    std::string resourceLibraryFilePath;
    std::string name;
    SResourceHeaderHeader resourceHeaderHeader;
    ZRuntimeResourceID runtimeResourceID;
    std::string resourceID;
    unsigned int offsetInResourceLibrary;
    void* resourceData;
    unsigned int resourceDataSize;
    std::vector<unsigned long long> references;
};
