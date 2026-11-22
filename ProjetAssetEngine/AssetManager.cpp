
#include "AssetManager.h"

#include "Checksum.h"
#include "Log.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "BinaryArchive.h"

#include <filesystem>
#include <memory>
#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <cstdint>

AssetManager::AssetManager() = default;

AssetManager::~AssetManager()
{
    m_assetJobSystem.WaitIdle();

    ClearRegistry();
}

void AssetManager::ClearRegistry()
{
    for (auto& [id, asset] : m_assetRegistry)
    {
        if (asset == nullptr)
            continue;

        {
            std::lock_guard<std::mutex> lock(asset->mutex);

            delete asset->objet;
            asset->objet = nullptr;
        }

        delete asset;
    }

    m_assetRegistry.clear();

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.clear();
    }
}


void AssetManager::LoadSync(Asset* asset)
{
    if (asset == nullptr)
        return;

    bool alreadyLoaded = false;

    {
        std::lock_guard<std::mutex> lock(asset->mutex);
        alreadyLoaded = asset->objet != nullptr;
    }

    if (alreadyLoaded)
    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);
        return;
    }

    try
    {
        bool isJson = asset->path.ends_with(".json");
        bool isBinary = asset->path.ends_with(".bin");

        if (!isJson && !isBinary)
        {
            LOG_ERROR("Format d'asset non supporte.");
            throw std::runtime_error("Format non supporte.");
        }

        uint32_t currentChecksum = Checksum::ComputeFromFile(asset->path);

        if (currentChecksum == 0 ||
            (asset->checksum != 0 && asset->checksum != currentChecksum))
        {
            LOG_ERROR("Checksum invalide ou fichier inaccessible.");
            throw std::runtime_error("Checksum invalide.");
        }

        LOG_INFO("Checksum valide.");

        std::unique_ptr<RTTI> loadedObject(Factory::Create(asset->type));

        if (!loadedObject)
        {
            LOG_ERROR("Impossible de creer l'objet via la Factory.");
            throw std::runtime_error("Factory::Create a echoue.");
        }

        if (isJson)
        {
            JsonArchive archive(asset->path, ArchiveDirection::Read);

            loadedObject->Process(archive);
        }
        else
        {
            BinaryArchive archive(
                asset->path,
                ArchiveDirection::Read
            );

            if (!archive.IsValid())
                throw std::runtime_error("Impossible d'ouvrir le fichier binaire.");

            loadedObject->Process(archive);
        }

        {
            std::lock_guard<std::mutex> lock(asset->mutex);

            if (asset->objet == nullptr)
                asset->objet = loadedObject.release();
        }

        LOG_INFO(("Asset charge : " + asset->path).c_str());
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(("Erreur de chargement : " + std::string(e.what())).c_str());
    }
    catch (...)
    {
        LOG_ERROR("Erreur inconnue durant le chargement.");
    }

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);
    }
}


void AssetManager::LoadAsync(Asset* asset)
{
    if (asset == nullptr || asset->IsLoaded())
        return;

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);

        if (!m_loadingTasks.emplace(asset->uiid).second)
            return;
    }

    try
    {
        m_assetJobSystem.Submit(
            [this, asset]()
            {
                LoadSync(asset);
            }
        );
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);

        throw;
    }
}


void AssetManager::ScanDirectory(const std::string& directoryPath, std::vector<Pawn*>& worldPawns)
{
    if (!std::filesystem::exists(directoryPath))
    {
        LOG_ERROR("Le dossier d'assets n'existe pas : " + directoryPath);
        return;
    }

    m_assetJobSystem.WaitIdle();

    std::vector<std::string> foundFiles;


    for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath))
    {
        if (!entry.is_regular_file())
            continue;

        std::filesystem::path filePath = entry.path();
        std::string ext = filePath.extension().string();

        if (ext != ".json" && ext != ".bin")
            continue;

        filePath = filePath.lexically_normal();

        std::string currentPath = filePath.string();

        std::filesystem::path assetDirectory = filePath.parent_path();
        std::string assetStem = filePath.stem().string();

        auto duplicateIt = std::find_if(foundFiles.begin(), foundFiles.end(), [&](const std::string& existingPath)
            {
                std::filesystem::path existing(existingPath);

                return existing.parent_path() == assetDirectory &&
                    existing.stem().string() == assetStem;
            }
        );

        if (duplicateIt != foundFiles.end())
        {
            std::filesystem::path existingPath(*duplicateIt);

            if (ext == ".json" && existingPath.extension() == ".bin")
                *duplicateIt = currentPath;            
            else
                continue;
            
        }
        else
            foundFiles.push_back(currentPath);
        

        uint32_t diskChecksum = Checksum::ComputeFromFile(currentPath);

        if (diskChecksum == 0)
        {
            LOG_ERROR(("Impossible de calculer le checksum : " + currentPath).c_str());
            continue;
        }


        Asset* existingAsset = nullptr;

        for (auto& [id, assetPtr] : m_assetRegistry)
        {
            if (assetPtr != nullptr && assetPtr->path == currentPath)
            {
                existingAsset = assetPtr;
                break;
            }
        }

        if (existingAsset != nullptr)
        {
            if (existingAsset->checksum != diskChecksum)
            {
                LOG_WARN(("ALERTE : L'asset a ete modifie sur le disque ! -> " + currentPath).c_str());

                {
                    std::lock_guard<std::mutex> lock(existingAsset->mutex);

                    if (existingAsset->objet != nullptr)
                    {
                        Pawn* loadedPawn = SAFE_CAST(Pawn, existingAsset->objet);

                        if (loadedPawn != nullptr)
                        {
                            worldPawns.erase(
                                std::remove(
                                    worldPawns.begin(),
                                    worldPawns.end(),
                                    loadedPawn
                                ),
                                worldPawns.end()
                            );
                        }

                        delete existingAsset->objet;
                        existingAsset->objet = nullptr;
                    }

                    existingAsset->checksum = diskChecksum;
                }

                LOG_INFO("Cache de l'asset invalide : rechargement necessaire.");
            }
        }
        else
        {
            Asset* newAsset = new Asset();

            newAsset->uiid = Guid::Generate();
            newAsset->path = currentPath;
            newAsset->type = "Pawn";
            newAsset->objet = nullptr;
            newAsset->checksum = diskChecksum;

            m_assetRegistry[newAsset->uiid] = newAsset;

            LOG_INFO("Nouvel asset indexe dans le Registry : " + currentPath);
        }
    }


    for (auto it = m_assetRegistry.begin(); it != m_assetRegistry.end();)
    {
        Asset* asset = it->second;

        if (asset == nullptr)
        {
            it = m_assetRegistry.erase(it);
            continue;
        }

        auto foundIt = std::find(foundFiles.begin(), foundFiles.end(), asset->path);

        if (foundIt == foundFiles.end())
        {
            LOG_INFO(("Suppression du registre (fichier absent du disque) : " + asset->path).c_str());

            {
                std::lock_guard<std::mutex> lock(asset->mutex);

                if (asset->objet != nullptr)
                {
                    Pawn* deadPawn = SAFE_CAST(Pawn, asset->objet);

                    if (deadPawn != nullptr)
                    {
                        worldPawns.erase(
                            std::remove(
                                worldPawns.begin(),
                                worldPawns.end(),
                                deadPawn
                            ),
                            worldPawns.end()
                        );
                    }

                    delete asset->objet;
                    asset->objet = nullptr;
                }
            }

            delete asset;

            it = m_assetRegistry.erase(it);
            continue;
        }

        ++it;
    }
}


JobSystem& AssetManager::GetJobSystem() noexcept
{
    return m_assetJobSystem;
}
