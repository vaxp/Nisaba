/// @file permissions.cpp
/// @brief Implementation of runtime permissions subsystem.
/// Bridges to AndroidPlatformBackend via JNI on Android, or provides clean
/// Granted stubs on desktop platforms (Windows / Linux).

#include "nisaba/backend_os/permissions.hpp"
#include "nisaba/backend_os/platform.hpp"

#if defined(__ANDROID__)
#include "nisaba/backend_os/android/android_platform.hpp"
#endif

#include <algorithm>

namespace nisaba::backend_os {

namespace {

#if defined(__ANDROID__)
android::AndroidPlatformBackend* getBackend() {
    auto* platform = Platform::instance();
    if (!platform) return nullptr;
    return static_cast<android::AndroidPlatformBackend*>(platform->getAndroidBackend());
}

int getDeviceSdk() {
    auto* backend = getBackend();
    return backend ? backend->getAndroidSdkVersion() : 33;
}
#else
int getDeviceSdk() {
    return 33;
}
#endif

} // namespace

PermissionStatus Permissions::check(Permission permission) {
#if defined(__ANDROID__)
    auto* backend = getBackend();
    if (!backend) return PermissionStatus::Denied;

    // Notifications prior to Android 13 (API 33) were granted automatically
    if (permission == Permission::Notifications && getDeviceSdk() < 33) {
        return PermissionStatus::Granted;
    }

    std::vector<std::string> perms = toAndroidStrings(permission);
    if (perms.empty()) {
        return PermissionStatus::Granted;
    }

    bool all_granted = true;
    for (const auto& p : perms) {
        PermissionStatus s = backend->checkPermission(p);
        if (s != PermissionStatus::Granted) {
            all_granted = false;
            break;
        }
    }

    if (all_granted) {
        return PermissionStatus::Granted;
    }
    return PermissionStatus::Denied;
#else
    (void)permission;
    return PermissionStatus::Granted;
#endif
}

PermissionStatus Permissions::check(std::string_view android_permission_string) {
#if defined(__ANDROID__)
    auto* backend = getBackend();
    if (!backend) return PermissionStatus::Denied;
    return backend->checkPermission(android_permission_string);
#else
    (void)android_permission_string;
    return PermissionStatus::Granted;
#endif
}

bool Permissions::isGranted(Permission permission) {
    return check(permission) == PermissionStatus::Granted;
}

bool Permissions::isGranted(std::string_view android_permission_string) {
    return check(android_permission_string) == PermissionStatus::Granted;
}

bool Permissions::shouldShowRationale(Permission permission) {
#if defined(__ANDROID__)
    auto* backend = getBackend();
    if (!backend) return false;

    std::vector<std::string> perms = toAndroidStrings(permission);
    for (const auto& p : perms) {
        if (backend->shouldShowRationale(p)) {
            return true;
        }
    }
    return false;
#else
    (void)permission;
    return false;
#endif
}

bool Permissions::shouldShowRationale(std::string_view android_permission_string) {
#if defined(__ANDROID__)
    auto* backend = getBackend();
    return backend ? backend->shouldShowRationale(android_permission_string) : false;
#else
    (void)android_permission_string;
    return false;
#endif
}

bool Permissions::openAppSettings() {
#if defined(__ANDROID__)
    auto* backend = getBackend();
    return backend ? backend->openAppSettings() : false;
#else
    return false;
#endif
}

void Permissions::request(Permission permission, PermissionCallback callback) {
    request(std::vector<Permission>{ permission }, [permission, cb = std::move(callback)](
        const std::unordered_map<Permission, PermissionStatus>& results)
    {
        if (!cb) return;
        auto it = results.find(permission);
        if (it != results.end()) {
            cb(it->second);
        } else {
            cb(PermissionStatus::Denied);
        }
    });
}

void Permissions::request(const std::vector<Permission>& permissions, MultiPermissionCallback callback) {
    if (permissions.empty()) {
        if (callback) callback({});
        return;
    }

    std::vector<std::string> all_strings;
    std::unordered_map<Permission, std::vector<std::string>> perm_map;

    for (Permission p : permissions) {
        std::vector<std::string> strs = toAndroidStrings(p);
        for (const auto& s : strs) {
            if (std::find(all_strings.begin(), all_strings.end(), s) == all_strings.end()) {
                all_strings.push_back(s);
            }
        }
        perm_map[p] = std::move(strs);
    }

    request(all_strings, [callback = std::move(callback), permissions, perm_map = std::move(perm_map)](
        const std::unordered_map<std::string, PermissionStatus>& str_results)
    {
        if (!callback) return;

        std::unordered_map<Permission, PermissionStatus> final_results;
        for (Permission p : permissions) {
            auto it = perm_map.find(p);
            if (it == perm_map.end() || it->second.empty()) {
                final_results[p] = PermissionStatus::Granted;
                continue;
            }

            bool all_granted = true;
            bool any_perm_denied = false;

            for (const auto& s : it->second) {
                auto res_it = str_results.find(s);
                PermissionStatus st = (res_it != str_results.end()) ? res_it->second : PermissionStatus::Denied;
                if (st == PermissionStatus::PermanentlyDenied) {
                    any_perm_denied = true;
                    all_granted = false;
                } else if (st != PermissionStatus::Granted) {
                    all_granted = false;
                }
            }

            if (all_granted) {
                final_results[p] = PermissionStatus::Granted;
            } else if (any_perm_denied) {
                final_results[p] = PermissionStatus::PermanentlyDenied;
            } else {
                final_results[p] = PermissionStatus::Denied;
            }
        }

        callback(final_results);
    });
}

void Permissions::request(const std::vector<std::string>& android_permissions, StringPermissionCallback callback) {
#if defined(__ANDROID__)
    auto* backend = getBackend();
    if (backend) {
        backend->requestPermissions(android_permissions, std::move(callback));
        return;
    }

    std::unordered_map<std::string, PermissionStatus> fallback;
    for (const auto& s : android_permissions) {
        fallback[s] = PermissionStatus::Denied;
    }
    if (callback) callback(fallback);
#else
    std::unordered_map<std::string, PermissionStatus> fallback;
    for (const auto& s : android_permissions) {
        fallback[s] = PermissionStatus::Granted;
    }
    if (callback) callback(fallback);
#endif
}

std::vector<std::string> Permissions::toAndroidStrings(Permission permission) {
    int sdk = getDeviceSdk();

    switch (permission) {
        case Permission::Internet:
            return { "android.permission.INTERNET" };

        case Permission::NetworkState:
            return { "android.permission.ACCESS_NETWORK_STATE" };

        case Permission::Storage:
            if (sdk >= 33) {
                return {
                    "android.permission.READ_MEDIA_IMAGES",
                    "android.permission.READ_MEDIA_VIDEO",
                    "android.permission.READ_MEDIA_AUDIO"
                };
            } else {
                return {
                    "android.permission.READ_EXTERNAL_STORAGE",
                    "android.permission.WRITE_EXTERNAL_STORAGE"
                };
            }

        case Permission::StorageImages:
            if (sdk >= 33) {
                return { "android.permission.READ_MEDIA_IMAGES" };
            } else {
                return { "android.permission.READ_EXTERNAL_STORAGE" };
            }

        case Permission::StorageVideo:
            if (sdk >= 33) {
                return { "android.permission.READ_MEDIA_VIDEO" };
            } else {
                return { "android.permission.READ_EXTERNAL_STORAGE" };
            }

        case Permission::StorageAudio:
            if (sdk >= 33) {
                return { "android.permission.READ_MEDIA_AUDIO" };
            } else {
                return { "android.permission.READ_EXTERNAL_STORAGE" };
            }

        case Permission::ManageExternalStorage:
            return { "android.permission.MANAGE_EXTERNAL_STORAGE" };

        case Permission::Location:
            return {
                "android.permission.ACCESS_FINE_LOCATION",
                "android.permission.ACCESS_COARSE_LOCATION"
            };

        case Permission::LocationFine:
            return { "android.permission.ACCESS_FINE_LOCATION" };

        case Permission::LocationCoarse:
            return { "android.permission.ACCESS_COARSE_LOCATION" };

        case Permission::LocationBackground:
            return { "android.permission.ACCESS_BACKGROUND_LOCATION" };

        case Permission::Notifications:
            if (sdk >= 33) {
                return { "android.permission.POST_NOTIFICATIONS" };
            } else {
                return {};
            }

        case Permission::Camera:
            return { "android.permission.CAMERA" };

        case Permission::Microphone:
            return { "android.permission.RECORD_AUDIO" };

        case Permission::Bluetooth:
            if (sdk >= 31) {
                return {
                    "android.permission.BLUETOOTH_SCAN",
                    "android.permission.BLUETOOTH_CONNECT"
                };
            } else {
                return {
                    "android.permission.BLUETOOTH",
                    "android.permission.BLUETOOTH_ADMIN"
                };
            }

        case Permission::Contacts:
            return {
                "android.permission.READ_CONTACTS",
                "android.permission.WRITE_CONTACTS"
            };

        case Permission::Calendar:
            return {
                "android.permission.READ_CALENDAR",
                "android.permission.WRITE_CALENDAR"
            };

        case Permission::BodySensors:
            return { "android.permission.BODY_SENSORS" };

        case Permission::Phone:
            return {
                "android.permission.READ_PHONE_STATE",
                "android.permission.CALL_PHONE"
            };
    }

    return {};
}

Permission Permissions::fromAndroidString(std::string_view name) {
    if (name == "android.permission.INTERNET")                 return Permission::Internet;
    if (name == "android.permission.ACCESS_NETWORK_STATE")      return Permission::NetworkState;
    if (name == "android.permission.READ_MEDIA_IMAGES")         return Permission::StorageImages;
    if (name == "android.permission.READ_MEDIA_VIDEO")          return Permission::StorageVideo;
    if (name == "android.permission.READ_MEDIA_AUDIO")          return Permission::StorageAudio;
    if (name == "android.permission.READ_EXTERNAL_STORAGE" ||
        name == "android.permission.WRITE_EXTERNAL_STORAGE")    return Permission::Storage;
    if (name == "android.permission.MANAGE_EXTERNAL_STORAGE")   return Permission::ManageExternalStorage;
    if (name == "android.permission.ACCESS_FINE_LOCATION")      return Permission::LocationFine;
    if (name == "android.permission.ACCESS_COARSE_LOCATION")    return Permission::LocationCoarse;
    if (name == "android.permission.ACCESS_BACKGROUND_LOCATION")return Permission::LocationBackground;
    if (name == "android.permission.POST_NOTIFICATIONS")        return Permission::Notifications;
    if (name == "android.permission.CAMERA")                    return Permission::Camera;
    if (name == "android.permission.RECORD_AUDIO")              return Permission::Microphone;
    if (name.rfind("android.permission.BLUETOOTH", 0) == 0)     return Permission::Bluetooth;
    if (name.rfind("android.permission.READ_CONTACTS", 0) == 0 ||
        name.rfind("android.permission.WRITE_CONTACTS", 0) == 0)return Permission::Contacts;
    if (name.rfind("android.permission.READ_CALENDAR", 0) == 0 ||
        name.rfind("android.permission.WRITE_CALENDAR", 0) == 0)return Permission::Calendar;
    if (name == "android.permission.BODY_SENSORS")              return Permission::BodySensors;
    if (name == "android.permission.READ_PHONE_STATE" ||
        name == "android.permission.CALL_PHONE")                return Permission::Phone;

    return Permission::Internet;
}

const char* Permissions::toString(Permission permission) {
    switch (permission) {
        case Permission::Internet:              return "Internet";
        case Permission::NetworkState:          return "NetworkState";
        case Permission::Storage:               return "Storage";
        case Permission::StorageImages:         return "StorageImages";
        case Permission::StorageVideo:          return "StorageVideo";
        case Permission::StorageAudio:          return "StorageAudio";
        case Permission::ManageExternalStorage: return "ManageExternalStorage";
        case Permission::Location:              return "Location";
        case Permission::LocationFine:          return "LocationFine";
        case Permission::LocationCoarse:        return "LocationCoarse";
        case Permission::LocationBackground:    return "LocationBackground";
        case Permission::Notifications:         return "Notifications";
        case Permission::Camera:                return "Camera";
        case Permission::Microphone:            return "Microphone";
        case Permission::Bluetooth:             return "Bluetooth";
        case Permission::Contacts:              return "Contacts";
        case Permission::Calendar:              return "Calendar";
        case Permission::BodySensors:           return "BodySensors";
        case Permission::Phone:                 return "Phone";
    }
    return "Unknown";
}

const char* Permissions::toString(PermissionStatus status) {
    switch (status) {
        case PermissionStatus::Granted:           return "Granted";
        case PermissionStatus::Denied:            return "Denied";
        case PermissionStatus::PermanentlyDenied: return "PermanentlyDenied";
        case PermissionStatus::Restricted:        return "Restricted";
        case PermissionStatus::Limited:           return "Limited";
        case PermissionStatus::Unknown:           return "Unknown";
    }
    return "Unknown";
}

} // namespace nisaba::backend_os
