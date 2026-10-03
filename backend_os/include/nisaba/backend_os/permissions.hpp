#pragma once
/// @file permissions.hpp
/// @brief Cross-platform runtime permissions management for nisaba.
/// First-class support for Android runtime permissions with modern API 33+
/// granular media and notification permission resolution, plus seamless
/// desktop (Windows / Linux) stubs.

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>

namespace nisaba::backend_os {

/// System permission types recognized by nisaba.
enum class Permission {
    /// Normal network internet access (android.permission.INTERNET).
    Internet,

    /// Access network state and Wi-Fi connectivity (android.permission.ACCESS_NETWORK_STATE).
    NetworkState,

    /// Smart storage access:
    /// - API 33+ (Android 13+): Maps to READ_MEDIA_IMAGES, READ_MEDIA_VIDEO, READ_MEDIA_AUDIO.
    /// - API < 33: Maps to READ_EXTERNAL_STORAGE, WRITE_EXTERNAL_STORAGE.
    Storage,

    /// Photos / Images media permission (READ_MEDIA_IMAGES on API 33+, READ_EXTERNAL_STORAGE on < 33).
    StorageImages,

    /// Video media permission (READ_MEDIA_VIDEO on API 33+, READ_EXTERNAL_STORAGE on < 33).
    StorageVideo,

    /// Audio media permission (READ_MEDIA_AUDIO on API 33+, READ_EXTERNAL_STORAGE on < 33).
    StorageAudio,

    /// All files access for file managers (MANAGE_EXTERNAL_STORAGE on API 30+).
    ManageExternalStorage,

    /// Geographic location (both Fine and Coarse location).
    Location,

    /// Precise GPS location (android.permission.ACCESS_FINE_LOCATION).
    LocationFine,

    /// Approximate network location (android.permission.ACCESS_COARSE_LOCATION).
    LocationCoarse,

    /// Background location access (android.permission.ACCESS_BACKGROUND_LOCATION on API 29+).
    LocationBackground,

    /// Post notifications (android.permission.POST_NOTIFICATIONS on API 33+; auto-granted on < 33).
    Notifications,

    /// Camera capture (android.permission.CAMERA).
    Camera,

    /// Microphone and audio recording (android.permission.RECORD_AUDIO).
    Microphone,

    /// Bluetooth connection and discovery (BLUETOOTH_SCAN, BLUETOOTH_CONNECT on API 31+).
    Bluetooth,

    /// Read and write contacts (android.permission.READ_CONTACTS, WRITE_CONTACTS).
    Contacts,

    /// Read and write calendar (android.permission.READ_CALENDAR, WRITE_CALENDAR).
    Calendar,

    /// Body sensors and heart rate (android.permission.BODY_SENSORS).
    BodySensors,

    /// Phone state and calls (android.permission.READ_PHONE_STATE, CALL_PHONE).
    Phone,
};

/// The resolution status of a permission.
enum class PermissionStatus {
    /// The permission is currently granted.
    Granted,

    /// The permission is denied, but can be requested again.
    Denied,

    /// The permission was permanently denied by the user ("Don't ask again" selected,
    /// or denied twice on Android 11+). The app should prompt the user to open settings.
    PermanentlyDenied,

    /// Restricted by device management policies or parental controls.
    Restricted,

    /// Partially or selectively granted (e.g. limited photo picker access).
    Limited,

    /// The status could not be determined.
    Unknown,
};

/// Callback for a single permission request result.
using PermissionCallback = std::function<void(PermissionStatus)>;

/// Callback for multiple permission request results.
using MultiPermissionCallback = std::function<void(const std::unordered_map<Permission, PermissionStatus>&)>;

/// Callback for raw Android permission strings request results.
using StringPermissionCallback = std::function<void(const std::unordered_map<std::string, PermissionStatus>&)>;

/// Runtime permissions manager.
class Permissions {
public:
    /// Check the status of a permission synchronously.
    [[nodiscard]] static PermissionStatus check(Permission permission);

    /// Check the status of a raw Android permission string synchronously.
    [[nodiscard]] static PermissionStatus check(std::string_view android_permission_string);

    /// Convenience check: returns true if the permission is Granted.
    [[nodiscard]] static bool isGranted(Permission permission);

    /// Convenience check: returns true if the raw Android permission is Granted.
    [[nodiscard]] static bool isGranted(std::string_view android_permission_string);

    /// Request a single permission asynchronously.
    /// The callback is invoked when the user responds or immediately if already resolved.
    static void request(Permission permission, PermissionCallback callback);

    /// Request multiple permissions asynchronously.
    static void request(const std::vector<Permission>& permissions, MultiPermissionCallback callback);

    /// Request arbitrary Android permission strings asynchronously.
    static void request(const std::vector<std::string>& android_permissions, StringPermissionCallback callback);

    /// Check if rationale should be shown before requesting the permission
    /// (returns true if the user previously denied the request without selecting "Don't ask again").
    [[nodiscard]] static bool shouldShowRationale(Permission permission);

    /// Check if rationale should be shown for a raw Android permission string.
    [[nodiscard]] static bool shouldShowRationale(std::string_view android_permission_string);

    /// Open the system application settings page so the user can manually grant
    /// permissions that were permanently denied.
    /// @return true if the settings activity was successfully launched.
    static bool openAppSettings();

    /// Convert a Permission enum to the corresponding Android permission strings
    /// based on the target/running Android SDK level.
    [[nodiscard]] static std::vector<std::string> toAndroidStrings(Permission permission);

    /// Convert an Android permission string to the closest Permission enum.
    [[nodiscard]] static Permission fromAndroidString(std::string_view name);

    /// Convert Permission enum to a human-readable string.
    [[nodiscard]] static const char* toString(Permission permission);

    /// Convert PermissionStatus enum to a human-readable string.
    [[nodiscard]] static const char* toString(PermissionStatus status);
};

} // namespace nisaba::backend_os
