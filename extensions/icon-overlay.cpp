#include "ext-common.h"
#include "ext-utils.h"
#include "shell-ext.h"
#include "log.h"
#include "commands.h"

#include <string>

namespace utils = seafile::utils;

// "The Shell calls IShellIconOverlayIdentifier::GetOverlayInfo to request the
//  location of the handler's icon overlay. The icon overlay handler returns
//  the name of the file containing the overlay image, and its index within
//  that file. The Shell then adds the icon overlay to the system image list."

STDMETHODIMP ShellExt::GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags)
{
    // seaf_ext_log ("GetOverlayInfo called for icon type %d!", (int)status_);

    std::string dll = utils::getThisDllPath();

    std::unique_ptr<wchar_t> ico(utils::stdStringtoWString(dll));
    int wlen = wcslen(ico.get());
    if (wlen + 1 > cchMax)
        return S_FALSE;

    wmemcpy(pwszIconFile, ico.get(), wlen + 1);

    *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

    *pIndex = (int)status_ - 1;

    return S_OK;
}

STDMETHODIMP ShellExt::GetPriority(int *priority)
{
    /* The priority value can be 0 ~ 100, with 0 be the highest */
    *priority = seafile::RepoInfo::N_Status - status_;
    return S_OK;
}


// "Before painting an object's icon, the Shell passes the object's name to
//  each icon overlay handler's IShellIconOverlayIdentifier::IsMemberOf
//  method. If a handler wants to have its icon overlay displayed,
//  it returns S_OK. The Shell then calls the handler's
//  IShellIconOverlayIdentifier::GetOverlayInfo method to determine which icon
//  to display."

STDMETHODIMP ShellExt::IsMemberOf(LPCWSTR path_w, DWORD attr)
{
    if (!seafile::utils::isShellExtEnabled()) {
        return S_FALSE;
    }

    std::string path = utils::wStringToStdString(path_w);

    if (!path.size()) {
        seaf_ext_log ("convert to char failed");
        return S_FALSE;
    }

    // seaf_ext_log ("IsMemberOf called for %s!", path.c_str());

    /* If length of path is shorter than 3, it should be a drive path,
     * such as C:\ , which should not be a repo folder ; And the
     * current folder being displayed must be "My Computer". If we
     * don't return quickly, it will lag the display.
     */
    if (path.size() <= 3) {
        return S_FALSE;
    }

    if (access(path.c_str(), F_OK) < 0 ||
        !(GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY)) {
        return S_FALSE;
    }

    seafile::RepoInfo info = getRepoInfoByPath(utils::localeToUtf8(path));
    if (info.isValid() && info.status == status_) {
        // seaf_ext_log ("[ICON] %d Set for %s", (int)status_, path.c_str());
        return S_OK;
    }
    return S_FALSE;
}
