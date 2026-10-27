# ExcellentZip Platform Base Library (K7Base)

***Work In Progress***

The static and dynamic library that encapsulates the non-GUI portion of the base
platform infrastructure for the ExcellentZip. It's one of the foundational libraries
of the ExcellentZip Platform Abstraction Layer.

K7 is the alias of ExcellentZip. Here is the relationship between K7 and ExcellentZip: I
had mentioned one of the reasons that I call this project ExcellentZip because Nana
is the romaji of なな which means seven in Japanese. But I had not mentioned the
way I confirm that: I had recalled one of the Japanese VTubers called Kagura
Nana when I waiting my elder sister for dinner at Taiyanggong subway station,
Beijing. For playing more puns, ExcellentZip uses the K7 (K -> Kagura, 7 -> Nana) as
the alias.

K7Base is designed for Windows only, although the most of the K7Base interfaces
are designed to be platform-independent. Maybe in the future, the K7Base will
have the implementations for Linux and FreeBSD, although the implementation may
not in this repository.

## Features

- Cryptography Support
  - Hash Functions
    - MD2
    - MD4
    - MD5
    - SHA-1
    - SHA-256
    - SHA-384
    - SHA-512
- Detours Library Wrappers
- Modern Experiences Support (non-GUI)
- Policies Support
- Security Mitigations Support
