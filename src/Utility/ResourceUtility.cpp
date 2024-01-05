#include "Utility/ResourceUtility.h"

std::string ResourceUtility::ConvertResourceIDToFilePath(std::string runtimeFolderPath, std::string& resourceID)
{
    std::string resourceID2 = StringUtility::ToLower(resourceID);
    std::string extension = resourceID2.substr(resourceID2.find_last_of('.') + 1);
    int index = static_cast<int>(resourceID2.find(':'));
    int index2 = static_cast<int>(resourceID2.find('?'));
    int index3 = static_cast<int>(resourceID2.find(']'));

    if (index2 != -1 && index < index2)
    {
        runtimeFolderPath += resourceID2.substr(index + 1, index2 - index - 1);
    }
    else
    {
        runtimeFolderPath += resourceID2.substr(index + 1, index3 - index - 1);
    }

    resourceID2 = resourceID2.substr(0, resourceID2.find_last_of('.') + 1);

    Hash::MD5Hash md5Hash = Hash::MD5(resourceID2);
    std::string fileName = Hash::ConvertMD5ToString(md5Hash);

    return std::format("{}/{}.{}", runtimeFolderPath, fileName, extension);
}

std::string ResourceUtility::GetResourceName(const std::string& resourceID)
{
    std::string name;

    if (resourceID.contains("("))
    {
        size_t index = resourceID.substr(0, resourceID.find("(")).find_last_of('/');

        name = resourceID.substr(index + 1, resourceID.find("(") - 2 - index);
    }
    else
    {
        size_t index = resourceID.find("?");

        if (index != -1 && resourceID[index + 1] != '/')
        {
            name = resourceID.substr(resourceID.find("?") + 1, resourceID.find("]") - resourceID.find("?") - 1);
        }
        else
        {
            name = resourceID.substr(resourceID.find_last_of("/") + 1, resourceID.find("]") - resourceID.find_last_of("/") - 1);
        }
    }

    name = name.substr(0, name.find_last_of("."));

    return name;
}
