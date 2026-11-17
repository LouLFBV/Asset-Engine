#include "Checksum.h"

#include <limits>

uint32_t Checksum::CalculateCRC32(const unsigned char* data, size_t length) // Sort une empreinte unique
{
    uint32_t crc = std::numeric_limits<uint32_t>::max(); // = 0xFFFFFFFF
    for (size_t i = 0; i < length; ++i) { // On parcours le buffer d'entrée octet par octet
        crc = crc ^ data[i]; //À chaque tour, on combine l'octet courant (data[i]) avec notre registre crc en utilisant un XOR (^). Ça permet de mélanger les données du fichier dans l'empreinte au fur et à mesure.
        for (int j = 0; j < 8; ++j) { // La boucle interne sur les 8 bits
            if (crc & 1) //On regarde si le bit tout à fait à droite (le bit le plus bas) du registre vaut 1.
                losses_or_bits(crc); // On appelle ta fonction losses_or_bits(crc) qui décale le tout et applique le fameux polynôme magique (0xEDB88320) pour corriger et disperser les bits.
            else
                crc >>= 1; // On fait simplement un décalage simple vers la droite (crc >>= 1) sans appliquer le polynôme.
        }
    }
    return ~crc; // une fois que tout le buffer a été avalé, on applique un NOT binaire (~ inverse tous les bits : les 0 deviennent des 1 et inversement). C'est la dernière étape indispensable de la norme CRC32 pour obtenir la valeur finale définitive.
}

uint32_t Checksum::ComputeFromString(const std::string& content)
{
    return CalculateCRC32(reinterpret_cast<const unsigned char*>(content.c_str()), content.size());
}

uint32_t Checksum::ComputeFromFile(const std::string& filepath) // charger un fichier entier en mémoire et calculer son empreinte d'un seul coup
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate); // std::ios::binary : On ouvre le fichier en mode binaire pour éviter que le système d'exploitation ne s'amuse à modifier des caractères (comme les retours à la ligne \r\n sur Windows), ce qui fausserait complètement le calcul du CRC32.
    //std::ios::ate(pour At The End) : C'est une astuce de pro. Au lieu d'ouvrir le fichier au début, ce flag positionne directement le curseur de lecture à la fin du fichier dès son ouverture.

    if (!file.is_open()) {
        return 0; // Erreur d'ouverture
    }

    std::streamsize size = file.tellg(); // file.tellg() : Comme le curseur est positionné à la fin grâce à ate, demander sa position (tellg) nous donne instantanément la taille exacte du fichier en octets
    file.seekg(0, std::ios::beg); // file.seekg(0, std::ios::beg) : Maintenant qu'on connaît la taille, on ramène le curseur tout au début du fichier (std::ios::beg) pour pouvoir commencer la lecture.

    std::vector<unsigned char> buffer(static_cast<size_t>(size)); // file.seekg(0, std::ios::beg) : Maintenant qu'on connaît la taille, on ramène le curseur tout au début du fichier (std::ios::beg) pour pouvoir commencer la lecture.
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) { // file.read(...) : On lit tout le contenu du fichier d'un seul bloc et on le stocke dans notre buffer. 
                                                                   //Le reinterpret_cast<char*> est juste là pour satisfaire le type attendu par la fonction de lecture de la bibliothèque standard (std::ifstream).
        return CalculateCRC32(buffer.data(), buffer.size());
    }

    return 0;
}