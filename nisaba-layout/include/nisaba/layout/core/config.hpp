#pragma once

#include <cstdint>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/types.hpp>

namespace nisaba::layout {

class Config;
class Node;

[[nodiscard]] bool configUpdateInvalidatesLayout(
    const Config& oldConfig,
    const Config& newConfig) noexcept;

/**
 * Sovereign configuration profile for layout calculation and scaling.
 */
class LAYOUT_EXPORT Config {
 public:
  explicit Config(LoggerCallback logger = nullptr) noexcept;

  void setUseWebDefaults(bool webDefaults) noexcept { useWebDefaults_ = webDefaults; }
  [[nodiscard]] bool useWebDefaults() const noexcept { return useWebDefaults_; }

  void setPointScaleFactor(float factor) noexcept;
  [[nodiscard]] float getPointScaleFactor() const noexcept { return pointScaleFactor_; }

  void setContext(void* ctx) noexcept { context_ = ctx; }
  [[nodiscard]] void* getContext() const noexcept { return context_; }

  [[nodiscard]] uint32_t getVersion() const noexcept { return version_; }

  void setLogger(LoggerCallback logger) noexcept { logger_ = logger; }
  void log(const Node* node, LogLevel level, const char* format, ...) const;

  void setCloneNodeCallback(CloneNodeCallback cloneNode) noexcept { cloneNode_ = cloneNode; }
  [[nodiscard]] Node* cloneNode(const Node* node, const Node* owner, size_t childIndex) const;

  // Compatibility stubs for errata / experiments
  void setErrata(Errata) noexcept {}
  [[nodiscard]] bool hasErrata(Errata) const noexcept { return false; }
  void setExperimentalFeatureEnabled(ExperimentalFeature, bool) noexcept {}
  [[nodiscard]] bool isExperimentalFeatureEnabled(ExperimentalFeature) const noexcept { return false; }

  [[nodiscard]] static const Config& getDefault() noexcept;

 private:
  float pointScaleFactor_{1.0f};
  bool useWebDefaults_{false};
  uint32_t version_{0};
  void* context_{nullptr};
  LoggerCallback logger_{nullptr};
  CloneNodeCallback cloneNode_{nullptr};
};

} // namespace nisaba::layout
