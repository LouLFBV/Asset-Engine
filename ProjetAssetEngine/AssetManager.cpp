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

    {
        std::lock_guard<std::mutex> lock(asset->mutex);

        if (asset->objet != nullptr)
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

        if (currentChecksum == 0 || (asset->checksum != 0 && asset->checksum != currentChecksum))
        {
            LOG_ERROR("Checksum invalide ou fichier inaccessible.");
            throw std::runtime_error("Checksum invalide.");
        }

        LOG_INFO("Checksum valide.");

        std::unique_ptr<RTTI> loadedObject(Factory::Create(asset->type)
        );

        if (!loadedObject)
        {
            LOG_ERROR("Impossible de creer l'objet via la Factory.");
            throw std::runtime_error("Factory::Create a echoue.");
        }

        if (isJson)
        {
            JsonArchive archive(
                asset->path,
                ArchiveDirection::Read
            );

            loadedObject->Process(archive);
        }
        else
        {
            BinaryArchive archive(
                asset->path,
                ArchiveDirection::Read
            );

            loadedObject->Process(archive);
        }

        {
            std::lock_guard<std::mutex> lock(asset->mutex);

            if (asset->objet == nullptr)
            {
                asset->objet = loadedObject.release();
            }
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
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        if (!m_loadingTasks.emplace(asset->uiid).second)
            return;
    }

    try
    {
        m_assetJobSystem.Submit([this, asset]()
            {
                LoadSync(asset);
            });
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
        if (entry.is_regular_file()) 
        {
            std::string ext = entry.path().extension().string();
            if (ext == ".json" || ext == ".bin") 
            {
                std::string filePath = entry.path().string();
                std::filesystem::path p(filePath);
                std::string stem = p.stem().string(); 

                bool skip = false;
                for (const auto& existingPath : foundFiles) {
                    std::filesystem::path existingP(existingPath);
                    if (existingP.stem().string() == stem) 
                    {
                        if (ext == ".bin" && existingP.extension().string() == ".json")
                        {
                            skip = true;
                            break;
                        }

                        if (ext == ".json" && existingP.extension().string() == ".bin")
                        {
                            skip = true;
                            break;
                        }
                    }
                }

                if (skip)
                    continue;

                foundFiles.push_back(filePath);
                uint32_t diskChecksum = Checksum::ComputeFromFile(filePath);

                Asset* existingAsset = nullptr;
                for (auto& [id, assetPtr] : m_assetRegistry)
                {
                    if (assetPtr != nullptr && assetPtr->path == filePath)
                    {
                        existingAsset = assetPtr;
                        break;
                    }
                }

                if (existingAsset != nullptr) 
                {
                    if (existingAsset->checksum != diskChecksum)
                    {
                        LOG_WARN(("ALERTE : L'asset a ete modifie sur le disque ! -> " + filePath).c_str());
                        existingAsset->checksum = diskChecksum;
                    }
                }
                else 
                {
                    Asset* newAsset = new Asset();
                    newAsset->uiid = Guid::Generate();
                    newAsset->path = filePath;
                    newAsset->type = "Pawn";
                    newAsset->objet = nullptr;
                    newAsset->checksum = diskChecksum;

                    m_assetRegistry[newAsset->uiid] = newAsset;
                    LOG_INFO("Nouvel asset indexe dans le Registry : " + filePath);
                }
            }
        }
    }

    for (auto it = m_assetRegistry.begin(); it != m_assetRegistry.end(); ) 
    {
        Asset* asset = it->second;
        if (asset != nullptr) 
        {
            auto foundIt = std::find(foundFiles.begin(), foundFiles.end(), asset->path);
            if (foundIt == foundFiles.end()) 
            {
                LOG_INFO(("Suppression du registre (fichier absent du disque) : " + asset->path).c_str());

                if (asset->objet != nullptr) 
                {
                    Pawn* deadPawn = SAFE_CAST(Pawn, asset->objet);
                    if (deadPawn) 
                    {
                        worldPawns.erase(
                            std::remove(worldPawns.begin(), worldPawns.end(), deadPawn),
                            worldPawns.end()
                        );
                    }

                    delete asset->objet;
                    asset->objet = nullptr;
                }

                delete asset;
                it = m_assetRegistry.erase(it);
                continue;
            }
        }
        ++it;
    }
}

JobSystem& AssetManager::GetJobSystem() noexcept
{
    return m_assetJobSystem;
}