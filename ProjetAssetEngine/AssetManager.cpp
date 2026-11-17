
#include "AssetManager.h"

#include "Checksum.h"
#include "Log.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "BinaryArchive.h"

#include <filesystem>
#include <memory>
#include <algorithm>

AssetManager::AssetManager() = default;

AssetManager::~AssetManager()
{
    // Attendre tous les chargements avant de detruire les assets.
    m_assetJobSystem.WaitIdle();

    ClearRegistry();
}

void AssetManager::ClearRegistry()
{
    for (auto& [id, asset] : m_assetRegistry)
    {
        if (asset == nullptr)
        {
            continue;
        }

        // Destruction de l'objet charge.
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
    {
        return;
    }

    // Un seul chargement a la fois pour cet asset.
    std::unique_lock<std::mutex> assetLock(asset->mutex);

    if (asset->objet != nullptr)
    {
        return;
    }

    // Verification de l'extension.
    bool isJson = asset->path.ends_with(".json");
    bool isBinary = asset->path.ends_with(".bin");

    if (!isJson && !isBinary)
    {
        LOG_ERROR("Format d'asset non supporte.");

        assetLock.unlock();

        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);

        return;
    }

    // Verification du checksum.
    uint32_t currentChecksum =
        Checksum::ComputeFromFile(asset->path);

    if (currentChecksum == 0 ||
        (asset->checksum != 0 &&
            asset->checksum != currentChecksum))
    {
        LOG_ERROR("Checksum invalide ou fichier inaccessible.");

        assetLock.unlock();

        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);

        return;
    }

    LOG_INFO("Checksum valide.");

    // Creation temporaire de l'objet.
    std::unique_ptr<RTTI> loadedObject(
        Factory::Create(asset->type)
    );

    if (loadedObject == nullptr)
    {
        LOG_ERROR("Impossible de creer l'objet via la Factory.");

        assetLock.unlock();

        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingTasks.erase(asset->uiid);

        return;
    }

    // Deserialisation selon le format.
    try
    {
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

        // Publication uniquement apres la deserialisation.
        asset->objet = loadedObject.release();

        LOG_INFO(
            ("Asset charge : " + asset->path).c_str()
        );
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(
            ("Erreur de chargement : " +
                std::string(e.what())).c_str()
        );
    }
    catch (...)
    {
        LOG_ERROR("Erreur inconnue durant le chargement.");
    }

    assetLock.unlock();

    // Fin de la tache.
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

        // Eviter plusieurs chargements du meme asset.
        if (!m_loadingTasks.emplace(asset->uiid).second)
        {
            return;
        }
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

void AssetManager::ScanDirectory(
    const std::string& directoryPath)
{
    // Les anciens assets ne doivent pas etre detruits
    // pendant qu'un worker les utilise.
    m_assetJobSystem.WaitIdle();

    ClearRegistry();

    if (!std::filesystem::exists(directoryPath))
    {
        LOG_ERROR(
            ("Dossier introuvable : " + directoryPath).c_str()
        );

        return;
    }

    try
    {
        for (const auto& entry :
            std::filesystem::recursive_directory_iterator(directoryPath))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            std::string extension =
                entry.path().extension().string();

            if (extension != ".json" && extension != ".bin")
            {
                continue;
            }

            Asset* newAsset = new Asset();

            newAsset->uiid = Guid::Generate();

            newAsset->path = entry.path().string();

            newAsset->type = "Pawn";

            newAsset->objet = nullptr;

            newAsset->checksum =
                Checksum::ComputeFromFile(newAsset->path);

            m_assetRegistry.emplace(
                newAsset->uiid,
                newAsset
            );

            LOG_INFO(
                ("Asset indexe : " + newAsset->path).c_str()
            );
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LOG_ERROR(
            ("Erreur de scan : " +
                std::string(e.what())).c_str()
        );
    }
}

JobSystem& AssetManager::GetJobSystem()
{
    return m_assetJobSystem;
}
