// Shell icon handler for ExcellentZip's own MSIX file associations. MIT License.
#pragma once
#include "ExcellentZip.Icons.h"
#include <ShlObj.h>
#include <Shlwapi.h>
#include <winrt/base.h>

namespace ExcellentZipIcons
{
    struct __declspec(uuid("6AB7C30F-177D-4CB9-9306-F9CE183E58A9")) IconHandler :
        winrt::implements<IconHandler, IPersistFile, IExtractIconW>
    {
        std::wstring Extension;

        HRESULT STDMETHODCALLTYPE GetClassID(CLSID* ClassId) noexcept override
        {
            if (!ClassId) return E_POINTER;
            *ClassId = __uuidof(IconHandler);
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE IsDirty() noexcept override { return S_FALSE; }
        HRESULT STDMETHODCALLTYPE Load(LPCOLESTR File, DWORD) noexcept override
        {
            if (!File) return E_POINTER;
            try
            {
                const wchar_t* Suffix = ::PathFindExtensionW(File);
                Extension = *Suffix == L'.' ? Suffix + 1 : L"";
                return S_OK;
            }
            catch (...) { return winrt::to_hresult(); }
        }
        HRESULT STDMETHODCALLTYPE Save(LPCOLESTR, BOOL) noexcept override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE SaveCompleted(LPCOLESTR) noexcept override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE GetCurFile(LPOLESTR* File) noexcept override
        {
            if (!File) return E_POINTER;
            *File = nullptr;
            return E_NOTIMPL;
        }
        HRESULT STDMETHODCALLTYPE GetIconLocation(UINT, LPWSTR File, UINT Capacity,
            int* Index, UINT* Flags) noexcept override
        {
            if (!File || !Index || !Flags) return E_POINTER;
            try
            {
                auto Path = IconPath(SelectedPack(), Extension.c_str());
                if (::GetFileAttributesW(Path.c_str()) == INVALID_FILE_ATTRIBUTES)
                    Path = IconPath(4, Extension.c_str());
                if (Path.empty() || Path.size() >= Capacity)
                    return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                ::wcscpy_s(File, Capacity, Path.c_str());
                *Index = 0;
                *Flags = GIL_PERINSTANCE | GIL_DONTCACHE;
                return S_OK;
            }
            catch (...) { return winrt::to_hresult(); }
        }
        HRESULT STDMETHODCALLTYPE Extract(LPCWSTR, UINT, HICON*, HICON*, UINT) noexcept override
        {
            return S_FALSE; // Let the Shell load the multi-resolution ICO.
        }
    };

    struct IconFactory : winrt::implements<IconFactory, IClassFactory>
    {
        HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* Outer, REFIID Interface,
            void** Object) noexcept override
        {
            if (!Object) return E_POINTER;
            *Object = nullptr;
            if (Outer) return CLASS_E_NOAGGREGATION;
            try
            {
                return winrt::make<IconHandler>()->QueryInterface(Interface, Object);
            }
            catch (...) { return winrt::to_hresult(); }
        }
        HRESULT STDMETHODCALLTYPE LockServer(BOOL Lock) noexcept override
        {
            if (Lock) ++winrt::get_module_lock();
            else --winrt::get_module_lock();
            return S_OK;
        }
    };
}
