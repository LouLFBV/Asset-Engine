#include "BinaryArchive.h"

BinaryArchive::BinaryArchive(const std::string& filepath, ArchiveDirection dir)
    : Archive(dir)
{
    // Définition des modes d'ouverture de fichier
    std::ios_base::openmode mode = std::ios::binary; //std::ios_base::openmode : C'est le type de variable standard en C++ qui sert à stocker des drapeaux (flags) de configuration pour les fichiers.
                                                     //std::ios::binary : On commence par dire qu'on veut travailler en mode binaire.

    if (IsReading()) {
        mode |= std::ios::in; // L'opérateur |= (le ou binaire cumulé) sert à ajouter un drapeau sans effacer les précédents. Ici, on ajoute std::ios::in (pour Input), qui donne l'autorisation de lire dans le fichier.
    }
    else {
        mode |= std::ios::out | std::ios::trunc; // trunc efface le fichier existant
        //On ajoute deux drapeaux d'un coup grâce aux | :
        //    std::ios::out(Output) : Pour donner l'autorisation d'écrire dans le fichier.
        //    std::ios::trunc(Truncate / Tronquer) : C'est une sécurité indispensable. Si le fichier existe déjà sur le disque, ce flag efface tout son contenu à l'ouverture pour repartir sur une feuille blanche.
        //    Ça évite que de vieilles données qui traînent ne se mélangent avec ta nouvelle sauvegarde.
    }

    m_stream.open(filepath, mode);

    if (!m_stream.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filepath << std::endl;
    }
}

BinaryArchive::~BinaryArchive() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
}


void BinaryArchive::Process(const char* Key, int& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, float& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, double& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, bool& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, char& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

//void BinaryArchive::Process(const char* Key, uint16_t& Data) {
//    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
//    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
//}
//
//void BinaryArchive::Process(const char* Key, uint32_t& Data) {
//    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
//    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
//}
//
//void BinaryArchive::Process(const char* Key, int16_t& Data) {
//    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
//    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
//}

void BinaryArchive::Process(const char* Key, std::string& Data) {
    if (IsReading()) {
        // 1. Lire la taille de la chaine
        uint32_t length = 0;
        m_stream.read(reinterpret_cast<char*>(&length), sizeof(length));

        /*Le piège du binaire : Une std::string en C++ a une taille variable. Quand le fichier a été écrit, on ne sait pas à l'avance si le texte fait 5 caractères ou 500.
        La solution : La première chose qu'on lit dans le fichier, c'est un entier sur 4 octets (uint32_t) qui représente la taille exacte de la chaîne de caractères.*/

        // 2. Préparer la chaine et lire les caractères
        Data.resize(length);
        if (length > 0) {
            m_stream.read(&Data[0], length); // En C++11 et +, c'est parfaitement valide
        }
        /*Data.resize(length); : On redimensionne notre variable Data pour lui réserver la place exacte en mémoire (par exemple, si la taille lue est 12, on réserve 12 octets).
        m_stream.read(&Data[0], length); : On lit directement les length octets suivants dans le fichier et on les injecte dans le buffer interne de la chaîne.
        (Depuis C++11, la mémoire d'un std::string est garantie d'être contiguë, donc &Data[0] pointe directement sur le tableau de caractères).*/
    }
    else {
        // 1. Écrire la taille de la chaine
        uint32_t length = static_cast<uint32_t>(Data.size());
        m_stream.write(reinterpret_cast<const char*>(&length), sizeof(length));

        // 2. Écrire les caractères
        if (length > 0) {
            m_stream.write(Data.c_str(), length);
        }
    }
}