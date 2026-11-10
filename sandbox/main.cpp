#include <AssetEngine/Asset/Asset.h>
#include <AssetEngine/Asset/AssetManager.h>
#include <AssetEngine/Asset/JobSystem.h>
#include <AssetEngine/Core/Factory.h>
#include <AssetEngine/Core/Guid.h>
#include <AssetEngine/Core/IO/IInputStream.h>
#include <AssetEngine/Core/IO/IOutputStream.h>
#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Core/IO/FileInputStream.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>
#include <AssetEngine/Core/Log.h>
#include <AssetEngine/Core/RTTI.h>
#include <AssetEngine/Security/Checksum.h>
#include <AssetEngine/Serialization/Serialization.h>
#include <AssetEngine/Serialization/Binary/BinaryWriter.h>
#include <AssetEngine/Serialization/Binary/BinaryReader.h>
#include <AssetEngine/Serialization/JsonArchive.h>

#include <iostream>

int main()
{
	std::cout << "Hello world !" << std::endl;
}