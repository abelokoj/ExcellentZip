/*
 * PROJECT:    ExcellentZip
 * FILE:       ExcellentZip.Codecs.h
 * PURPOSE:    Definition for ExcellentZip.Codecs
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#ifndef EXCELLENTZIP_CODECS
#define EXCELLENTZIP_CODECS

#include <Windows.h>

#include <ExcellentZip.Specification.SevenZip.h>

#include <Mile.Helpers.CppBase.h>

/**
 * Definition of ExcellentZip interface constants
 *
 * I had mentioned one of the reasons that I call this project ExcellentZip because
 * Nana is the romaji of なな which means seven in Japanese. But I had not
 * mentioned the way I confirm that: I had recalled one of the Japanese VTubers
 * called Kagura Nana when I waiting my elder sister for dinner at Taiyanggong
 * subway station, Beijing. For playing more puns, ExcellentZip uses the K7 prefix in
 * some definitions. (K -> Kagura, 7 -> Nana)
 */

namespace ExcellentZip::Codecs
{
    /**
     * @brief The ID base value of ExcellentZip Hash Providers.
     * @remark 0x4823374B is little-endian representation of K7#H (H -> Hash)
     */
    const UINT64 HashProviderIdBase = 0x4823374B00000000;

    /**
     * @brief The ID base value of ExcellentZip Decoder Providers.
     * @remark 0x4423374B is little-endian representation of K7#D (D -> Decoder)
     */
    const UINT64 DecoderProviderIdBase = 0x4423374B00000000;

    /**
     * @brief The ID base value of ExcellentZip Encoder Providers.
     * @remark 0x4523374B is little-endian representation of K7#E (E -> Encoder)
     */
    const UINT64 EncoderProviderIdBase = 0x4523374B00000000;

    /**
     * @brief The ID base value of ExcellentZip Archiver Providers.
     * @remark 0x4523374B is little-endian representation of K7#A (A -> Archiver)
     */
    const UINT64 ArchiverProviderIdBase = 0x4123374B00000000;
}

namespace ExcellentZip::Codecs::Hash
{
    IHasher* CreateMd2();
    IHasher* CreateMd4();
    IHasher* CreateMd5();
    IHasher* CreateBlake3();
    IHasher* CreateSm3();
    IHasher* CreateAich();
    IHasher* CreateBlake2b();
    IHasher* CreateEd2k();
    IHasher* CreateEdonR224();
    IHasher* CreateEdonR256();
    IHasher* CreateEdonR384();
    IHasher* CreateEdonR512();
    IHasher* CreateGost94();
    IHasher* CreateGost94CryptoPro();
    IHasher* CreateGost12256();
    IHasher* CreateGost12512();
    IHasher* CreateHas160();
    IHasher* CreateRipemd160();
    IHasher* CreateSha224();
    IHasher* CreateSnefru128();
    IHasher* CreateSnefru256();
    IHasher* CreateTiger();
    IHasher* CreateTiger2();
    IHasher* CreateTorrent();
    IHasher* CreateTth();
    IHasher* CreateWhirlpool();
    IHasher* CreateXxh32();
    IHasher* CreateXxh64();
    IHasher* CreateXxh364();
    IHasher* CreateXxh3128();
}

namespace ExcellentZip::Codecs::Decoder
{

}

namespace ExcellentZip::Codecs::Encoder
{

}

namespace ExcellentZip::Codecs::Archive
{
    IInArchive* CreateUfs();
    IInArchive* CreateDotNetSingleFile();
    IInArchive* CreateElectronAsar();
    IInArchive* CreateRomfs();
    IInArchive* CreateZealfs();
    IInArchive* CreateWebAssembly();
    IInArchive* CreateLittlefs();
}

#endif // !EXCELLENTZIP_CODECS
