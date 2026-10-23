#include "orion/orion.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <string_view>

using namespace orion;
using namespace orion::physics;

static int g_tests_passed = 0;
static int g_tests_total = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        g_tests_total++; \
        if (!(cond)) { \
            std::cerr << "[-] FAILED: " << msg << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            assert(false && msg); \
        } else { \
            g_tests_passed++; \
        } \
    } while(0)

static constexpr float TEST_EPS = 1e-4f;

void test_gltf_pbr_extensions() {
    std::cout << "[*] Testing glTF 2.0 Modern PBR Material Extensions Ingestion..." << std::endl;

    constexpr std::string_view gltf_json = R"({
  "asset": { "version": "2.0" },
  "materials": [
    {
      "name": "ClearCoatCarPaint",
      "pbrMetallicRoughness": {
        "baseColorFactor": [0.8, 0.05, 0.05, 1.0],
        "metallicFactor": 0.9,
        "roughnessFactor": 0.2
      },
      "extensions": {
        "KHR_materials_clearcoat": {
          "clearcoatFactor": 0.85,
          "clearcoatRoughnessFactor": 0.03
        }
      }
    },
    {
      "name": "BrushedAnisotropicTitanium",
      "pbrMetallicRoughness": {
        "baseColorFactor": [0.75, 0.75, 0.78, 1.0],
        "metallicFactor": 1.0,
        "roughnessFactor": 0.35
      },
      "extensions": {
        "KHR_materials_anisotropy": {
          "anisotropyStrength": 0.72
        }
      }
    },
    {
      "name": "VelvetCloth",
      "pbrMetallicRoughness": {
        "baseColorFactor": [0.1, 0.02, 0.15, 1.0],
        "roughnessFactor": 0.8
      },
      "extensions": {
        "KHR_materials_sheen": {
          "sheenColorFactor": [0.9, 0.4, 0.5],
          "sheenRoughnessFactor": 0.15
        }
      }
    },
    {
      "name": "OpticalGlass",
      "pbrMetallicRoughness": {
        "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
        "roughnessFactor": 0.05
      },
      "extensions": {
        "KHR_materials_transmission": {
          "transmissionFactor": 0.96
        },
        "KHR_materials_ior": {
          "ior": 1.52
        },
        "KHR_materials_volume": {
          "thicknessFactor": 0.65,
          "attenuationColor": [0.85, 0.95, 1.0]
        }
      }
    },
    {
      "name": "HumanSkinSubsurface",
      "pbrMetallicRoughness": {
        "baseColorFactor": [0.9, 0.7, 0.6, 1.0],
        "roughnessFactor": 0.4
      },
      "extensions": {
        "KHR_materials_subsurface": {
          "subsurfaceColor": [0.95, 0.3, 0.2],
          "subsurfacePower": 14.5
        }
      }
    },
    {
      "name": "UnlitUIElement",
      "pbrMetallicRoughness": {
        "baseColorFactor": [0.0, 1.0, 0.5, 1.0]
      },
      "extensions": {
        "KHR_materials_unlit": {}
      }
    }
  ]
})";

    GltfLoadResult res = GltfLoader::load_from_json(gltf_json);
    TEST_ASSERT(res.success, "glTF JSON parsed successfully");
    TEST_ASSERT(res.materials.size() == 6, "All 6 extended materials ingested");

    // 1. Clear Coat Verification
    {
        const auto& mat = res.materials[0];
        TEST_ASSERT(mat->shading_model == ShadingModel::ClearCoat, "Material 0 identified as ClearCoat");
        TEST_ASSERT(std::abs(mat->clear_coat - 0.85f) < TEST_EPS, "Clear coat factor is 0.85");
        TEST_ASSERT(std::abs(mat->clear_coat_roughness - 0.03f) < TEST_EPS, "Clear coat roughness is 0.03");
        TEST_ASSERT(std::abs(mat->metallic_factor - 0.9f) < TEST_EPS, "Base metallic factor preserved");
    }

    // 2. Anisotropy Verification
    {
        const auto& mat = res.materials[1];
        TEST_ASSERT(mat->shading_model == ShadingModel::Anisotropic, "Material 1 identified as Anisotropic");
        TEST_ASSERT(std::abs(mat->anisotropy - 0.72f) < TEST_EPS, "Anisotropy strength is 0.72");
    }

    // 3. Sheen / Cloth Verification
    {
        const auto& mat = res.materials[2];
        TEST_ASSERT(mat->shading_model == ShadingModel::Cloth, "Material 2 identified as Cloth");
        TEST_ASSERT(std::abs(mat->sheen_color.x - 0.9f) < TEST_EPS, "Sheen Red is 0.9");
        TEST_ASSERT(std::abs(mat->sheen_color.y - 0.4f) < TEST_EPS, "Sheen Green is 0.4");
        TEST_ASSERT(std::abs(mat->sheen_color.z - 0.5f) < TEST_EPS, "Sheen Blue is 0.5");
        TEST_ASSERT(std::abs(mat->sheen_roughness - 0.15f) < TEST_EPS, "Sheen roughness is 0.15");
    }

    // 4. Transmission, IOR & Volume Verification
    {
        const auto& mat = res.materials[3];
        TEST_ASSERT(mat->shading_model == ShadingModel::Transmission, "Material 3 identified as Transmission");
        TEST_ASSERT(std::abs(mat->transmission - 0.96f) < TEST_EPS, "Transmission factor is 0.96");
        TEST_ASSERT(std::abs(mat->ior - 1.52f) < TEST_EPS, "Index of Refraction is 1.52");
        TEST_ASSERT(std::abs(mat->thickness - 0.65f) < TEST_EPS, "Thickness factor is 0.65");
        TEST_ASSERT(std::abs(mat->absorption.x - 0.85f) < TEST_EPS, "Attenuation Red is 0.85");
    }

    // 5. Subsurface Scattering Verification
    {
        const auto& mat = res.materials[4];
        TEST_ASSERT(mat->shading_model == ShadingModel::Subsurface, "Material 4 identified as Subsurface");
        TEST_ASSERT(std::abs(mat->subsurface_color.x - 0.95f) < TEST_EPS, "Subsurface tint Red is 0.95");
        TEST_ASSERT(std::abs(mat->subsurface_power - 14.5f) < TEST_EPS, "Subsurface power is 14.5");
    }

    // 6. Unlit Verification
    {
        const auto& mat = res.materials[5];
        TEST_ASSERT(mat->shading_model == ShadingModel::Unlit, "Material 5 identified as Unlit");
    }

    std::cout << "  -> All 6 glTF 2.0 Modern PBR material extensions validated!" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign glTF 2.0 PBR Extensions Unit Tests" << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_gltf_pbr_extensions();

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Results: " << g_tests_passed << " / " << g_tests_total << " assertions passed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (g_tests_passed == g_tests_total) ? 0 : 1;
}
