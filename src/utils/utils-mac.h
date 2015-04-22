#ifndef SEAFILE_CLIENT_UTILS_MAC_H_
#define SEAFILE_CLIENT_UTILS_MAC_H_
#include <QtGlobal>
#ifdef Q_OS_MAC
#include <QString>
#if defined(__OBJC__)
#import <Foundation/Foundation.h>
#else  // __OBJC__
#include <CoreFoundation/CoreFoundation.h>
#endif // __OBJC__

typedef void DarkModeChangedCallback(bool value);

template<typename T>
T CFCast(const CFTypeRef& cf_val);

namespace utils {
namespace mac {

template<typename T>
T getValueFromDictionary(CFDictionaryRef dict, CFStringRef key) {
  CFTypeRef value = CFDictionaryGetValue(dict, key);
  T value_specific = CFCast<T>(value);

  return value_specific;
}

QString getValueFromCFString(CFStringRef cfstring);

void setDockIconStyle(bool hidden);
void orderFrontRegardless(unsigned long long win_id, bool force = false);
bool get_auto_start();
void set_auto_start(bool enabled);
void copyTextToPasteboard(const QString &text);

bool is_darkmode();
void set_darkmode_watcher(DarkModeChangedCallback *cb);
void get_current_osx_version(unsigned *major, unsigned *minor, unsigned *patch);

QString fix_file_id_url(const QString &path);

QString mainBundlePath();

} // namespace mac
} // namespace utils

#endif /* Q_OS_MAC */
#endif /* SEAFILE_CLIENT_UTILS_MAC_H_ */
