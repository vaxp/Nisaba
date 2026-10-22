#include "nisaba/render3d/gltf/gltf_loader.hpp"
#include "nisaba/json/json.hpp"
#include "nisaba/image/image_io.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <cctype>
#include <algorithm>

namespace nisaba::render3d {

using Quat = nisaba::physics::Quat;

namespace {

// --- Sovereign Base64 Decoder ---
std::vector<uint8_t> decode_base64(std::string_view input) {
    std::vector<uint8_t> out;
    out.reserve((input.size() * 3) / 4);

    auto decode_char = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+' || c == '-') return 62;
        if (c == '/' || c == '_') return 63;
        return -1;
    };

    uint32_t val = 0;
    int valb = -8;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) continue;
        if (c == '=') break; // padding
        int d = decode_char(c);
        if (d < 0) continue;
        val = (val << 6) | static_cast<uint32_t>(d);
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// --- Internal glTF parsing structures ---
struct BufferViewInfo {
    size_t buffer_idx{0};
    size_t byte_offset{0};
    size_t byte_length{0};
    size_t byte_stride{0};
};

struct AccessorInfo {
    size_t buffer_view_idx{0};
    size_t byte_offset{0};
    uint32_t component_type{5126}; // 5126 = FLOAT
    size_t count{0};
    std::string type{"SCALAR"};
    bool normalized{false};
};

size_t get_component_count(std::string_view type) noexcept {
    if (type == "SCALAR") return 1;
    if (type == "VEC2") return 2;
    if (type == "VEC3") return 3;
    if (type == "VEC4") return 4;
    if (type == "MAT4") return 16;
    return 1;
}

size_t get_component_size(uint32_t comp_type) noexcept {
    switch (comp_type) {
        case 5120: // BYTE
        case 5121: // UNSIGNED_BYTE
            return 1;
        case 5122: // SHORT
        case 5123: // UNSIGNED_SHORT
            return 2;
        case 5125: // UNSIGNED_INT
        case 5126: // FLOAT
            return 4;
        default:
            return 4;
    }
}

std::vector<float> read_accessor_floats(
    const AccessorInfo& acc,
    const std::vector<BufferViewInfo>& buffer_views,
    const std::vector<std::vector<uint8_t>>& buffers
) {
    std::vector<float> result;
    if (acc.buffer_view_idx >= buffer_views.size()) return result;

    const auto& bv = buffer_views[acc.buffer_view_idx];
    if (bv.buffer_idx >= buffers.size()) return result;

    const auto& buf = buffers[bv.buffer_idx];
    const size_t num_components = get_component_count(acc.type);
    const size_t comp_size = get_component_size(acc.component_type);
    const size_t element_size = num_components * comp_size;
    const size_t stride = (bv.byte_stride > 0) ? bv.byte_stride : element_size;

    result.resize(acc.count * num_components);

    for (size_t i = 0; i < acc.count; ++i) {
        const size_t offset = bv.byte_offset + acc.byte_offset + i * stride;
        if (offset + element_size > buf.size()) break;

        const uint8_t* ptr = buf.data() + offset;

        for (size_t c = 0; c < num_components; ++c) {
            const size_t out_idx = i * num_components + c;
            switch (acc.component_type) {
                case 5126: { // FLOAT
                    float val;
                    std::memcpy(&val, ptr + c * 4, 4);
                    result[out_idx] = val;
                    break;
                }
                case 5120: { // BYTE
                    int8_t val = static_cast<int8_t>(ptr[c]);
                    result[out_idx] = acc.normalized ? std::max(val / 127.0f, -1.0f) : static_cast<float>(val);
                    break;
                }
                case 5121: { // UNSIGNED_BYTE
                    uint8_t val = ptr[c];
                    result[out_idx] = acc.normalized ? (val / 255.0f) : static_cast<float>(val);
                    break;
                }
                case 5122: { // SHORT
                    int16_t val;
                    std::memcpy(&val, ptr + c * 2, 2);
                    result[out_idx] = acc.normalized ? std::max(val / 32767.0f, -1.0f) : static_cast<float>(val);
                    break;
                }
                case 5123: { // UNSIGNED_SHORT
                    uint16_t val;
                    std::memcpy(&val, ptr + c * 2, 2);
                    result[out_idx] = acc.normalized ? (val / 65535.0f) : static_cast<float>(val);
                    break;
                }
                case 5125: { // UNSIGNED_INT
                    uint32_t val;
                    std::memcpy(&val, ptr + c * 4, 4);
                    result[out_idx] = static_cast<float>(val);
                    break;
                }
                default:
                    result[out_idx] = 0.0f;
                    break;
            }
        }
    }

    return result;
}

std::vector<uint32_t> read_accessor_indices(
    const AccessorInfo& acc,
    const std::vector<BufferViewInfo>& buffer_views,
    const std::vector<std::vector<uint8_t>>& buffers
) {
    std::vector<uint32_t> result;
    if (acc.buffer_view_idx >= buffer_views.size()) return result;

    const auto& bv = buffer_views[acc.buffer_view_idx];
    if (bv.buffer_idx >= buffers.size()) return result;

    const auto& buf = buffers[bv.buffer_idx];
    const size_t comp_size = get_component_size(acc.component_type);
    const size_t stride = (bv.byte_stride > 0) ? bv.byte_stride : comp_size;

    result.resize(acc.count);

    for (size_t i = 0; i < acc.count; ++i) {
        const size_t offset = bv.byte_offset + acc.byte_offset + i * stride;
        if (offset + comp_size > buf.size()) break;

        const uint8_t* ptr = buf.data() + offset;

        switch (acc.component_type) {
            case 5121: { // UNSIGNED_BYTE
                result[i] = static_cast<uint32_t>(ptr[0]);
                break;
            }
            case 5123: { // UNSIGNED_SHORT
                uint16_t val;
                std::memcpy(&val, ptr, 2);
                result[i] = static_cast<uint32_t>(val);
                break;
            }
            case 5125: { // UNSIGNED_INT
                uint32_t val;
                std::memcpy(&val, ptr, 4);
                result[i] = val;
                break;
            }
            default:
                result[i] = 0;
                break;
        }
    }

    return result;
}

std::vector<uint16_t> read_accessor_joints(
    const AccessorInfo& acc,
    const std::vector<BufferViewInfo>& buffer_views,
    const std::vector<std::vector<uint8_t>>& buffers
) {
    std::vector<uint16_t> result;
    if (acc.buffer_view_idx >= buffer_views.size()) return result;

    const auto& bv = buffer_views[acc.buffer_view_idx];
    if (bv.buffer_idx >= buffers.size()) return result;

    const auto& buf = buffers[bv.buffer_idx];
    const size_t num_components = get_component_count(acc.type);
    const size_t comp_size = get_component_size(acc.component_type);
    const size_t element_size = num_components * comp_size;
    const size_t stride = (bv.byte_stride > 0) ? bv.byte_stride : element_size;

    result.resize(acc.count * num_components);

    for (size_t i = 0; i < acc.count; ++i) {
        const size_t offset = bv.byte_offset + acc.byte_offset + i * stride;
        if (offset + element_size > buf.size()) break;

        const uint8_t* ptr = buf.data() + offset;

        for (size_t c = 0; c < num_components; ++c) {
            const size_t out_idx = i * num_components + c;
            switch (acc.component_type) {
                case 5121: // UNSIGNED_BYTE
                    result[out_idx] = static_cast<uint16_t>(ptr[c]);
                    break;
                case 5123: { // UNSIGNED_SHORT
                    uint16_t val = 0;
                    std::memcpy(&val, ptr + c * 2, 2);
                    result[out_idx] = val;
                    break;
                }
                case 5125: { // UNSIGNED_INT
                    uint32_t val = 0;
                    std::memcpy(&val, ptr + c * 4, 4);
                    result[out_idx] = static_cast<uint16_t>(val);
                    break;
                }
                default:
                    result[out_idx] = 0;
                    break;
            }
        }
    }

    return result;
}

} // anonymous namespace

GltfLoadResult GltfLoader::load_from_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        GltfLoadResult res;
        res.success = false;
        res.error_message = "Failed to open glTF file: " + filepath;
        return res;
    }

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        GltfLoadResult res;
        res.success = false;
        res.error_message = "Failed to read glTF file bytes: " + filepath;
        return res;
    }

    std::string base_dir;
    const auto slash_pos = filepath.find_last_of("/\\");
    if (slash_pos != std::string::npos) {
        base_dir = filepath.substr(0, slash_pos);
    }

    return load_from_memory(buffer, base_dir);
}

GltfLoadResult GltfLoader::load_from_memory(std::span<const uint8_t> data, const std::string& base_dir) {
    if (data.size() < 4) {
        GltfLoadResult res;
        res.success = false;
        res.error_message = "Empty or invalid glTF data stream";
        return res;
    }

    // Check for Binary glTF (GLB) header magic 0x46546C67 ("glTF")
    uint32_t magic = 0;
    std::memcpy(&magic, data.data(), 4);

    if (magic == 0x46546C67) {
        if (data.size() < 12) {
            GltfLoadResult res;
            res.success = false;
            res.error_message = "Truncated GLB container header";
            return res;
        }

        uint32_t version = 0;
        uint32_t length = 0;
        std::memcpy(&version, data.data() + 4, 4);
        std::memcpy(&length, data.data() + 8, 4);

        if (version != 2) {
            GltfLoadResult res;
            res.success = false;
            res.error_message = "Unsupported GLB version: " + std::to_string(version);
            return res;
        }

        size_t offset = 12;
        std::string_view json_view;
        std::vector<std::vector<uint8_t>> glb_buffers;

        while (offset + 8 <= data.size()) {
            uint32_t chunk_length = 0;
            uint32_t chunk_type = 0;
            std::memcpy(&chunk_length, data.data() + offset, 4);
            std::memcpy(&chunk_type, data.data() + offset + 4, 4);
            offset += 8;

            if (offset + chunk_length > data.size()) break;

            if (chunk_type == 0x4E4F534A) { // "JSON"
                json_view = std::string_view(
                    reinterpret_cast<const char*>(data.data() + offset),
                    chunk_length
                );
            } else if (chunk_type == 0x004E4942) { // "BIN\0"
                std::vector<uint8_t> bin_buf(
                    data.data() + offset,
                    data.data() + offset + chunk_length
                );
                glb_buffers.push_back(std::move(bin_buf));
            }

            offset += chunk_length;
        }

        if (json_view.empty()) {
            GltfLoadResult res;
            res.success = false;
            res.error_message = "Missing JSON chunk in GLB asset";
            return res;
        }

        return load_from_json(json_view, base_dir, glb_buffers);
    }

    // Standard ASCII JSON glTF
    std::string_view json_str(reinterpret_cast<const char*>(data.data()), data.size());
    return load_from_json(json_str, base_dir, {});
}

GltfLoadResult GltfLoader::load_from_json(
    std::string_view json_str,
    const std::string& base_dir,
    const std::vector<std::vector<uint8_t>>& external_buffers
) {
    GltfLoadResult result;
    result.scene = std::make_shared<Scene3D>();

    auto root_opt = nisaba::json::JsonValue::parse(json_str);
    if (!root_opt) {
        result.success = false;
        result.error_message = "Failed to parse glTF JSON syntax";
        return result;
    }

    const auto& root = *root_opt;

    // 1. Ingest Buffers
    std::vector<std::vector<uint8_t>> buffers;
    const auto& json_buffers = root.get_array("buffers");
    buffers.reserve(json_buffers.size());

    for (size_t i = 0; i < json_buffers.size(); ++i) {
        if (i < external_buffers.size() && !external_buffers[i].empty()) {
            buffers.push_back(external_buffers[i]);
            continue;
        }

        const auto& buf_obj = json_buffers[i];
        std::string_view uri = buf_obj.get_string("uri");

        if (uri.starts_with("data:")) {
            const auto b64_pos = uri.find(";base64,");
            if (b64_pos != std::string_view::npos) {
                std::string_view b64_data = uri.substr(b64_pos + 8);
                buffers.push_back(decode_base64(b64_data));
            } else {
                buffers.emplace_back();
            }
        } else if (!uri.empty() && !base_dir.empty()) {
            std::string path = base_dir + "/" + std::string(uri);
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                const auto sz = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<uint8_t> fbuf(static_cast<size_t>(sz));
                file.read(reinterpret_cast<char*>(fbuf.data()), sz);
                buffers.push_back(std::move(fbuf));
            } else {
                buffers.emplace_back();
            }
        } else {
            buffers.emplace_back();
        }
    }

    // 2. Ingest BufferViews
    std::vector<BufferViewInfo> buffer_views;
    const auto& json_bvs = root.get_array("bufferViews");
    buffer_views.reserve(json_bvs.size());

    for (const auto& bv_val : json_bvs) {
        BufferViewInfo bv;
        bv.buffer_idx = static_cast<size_t>(bv_val.get_int("buffer", 0));
        bv.byte_offset = static_cast<size_t>(bv_val.get_int("byteOffset", 0));
        bv.byte_length = static_cast<size_t>(bv_val.get_int("byteLength", 0));
        bv.byte_stride = static_cast<size_t>(bv_val.get_int("byteStride", 0));
        buffer_views.push_back(bv);
    }

    // 3. Ingest Accessors
    std::vector<AccessorInfo> accessors;
    const auto& json_accs = root.get_array("accessors");
    accessors.reserve(json_accs.size());

    for (const auto& acc_val : json_accs) {
        AccessorInfo acc;
        acc.buffer_view_idx = static_cast<size_t>(acc_val.get_int("bufferView", 0));
        acc.byte_offset = static_cast<size_t>(acc_val.get_int("byteOffset", 0));
        acc.component_type = static_cast<uint32_t>(acc_val.get_int("componentType", 5126));
        acc.count = static_cast<size_t>(acc_val.get_int("count", 0));
        acc.type = std::string(acc_val.get_string("type", "SCALAR"));
        acc.normalized = acc_val.get_bool("normalized", false);
        accessors.push_back(acc);
    }

    // 4. Ingest Images & Textures
    const auto& json_images = root.get_array("images");
    std::vector<std::shared_ptr<Texture2D>> loaded_images;
    loaded_images.reserve(json_images.size());

    for (const auto& img_val : json_images) {
        std::shared_ptr<Texture2D> tex;
        if (img_val.has("bufferView")) {
            const size_t bv_idx = static_cast<size_t>(img_val.get_int("bufferView", 0));
            if (bv_idx < buffer_views.size()) {
                const auto& bv = buffer_views[bv_idx];
                if (bv.buffer_idx < buffers.size()) {
                    const auto& b = buffers[bv.buffer_idx];
                    if (bv.byte_offset + bv.byte_length <= b.size()) {
                        std::span<const uint8_t> span(b.data() + bv.byte_offset, bv.byte_length);
                        tex = Texture2D::load_from_memory(span);
                        if (!tex) {
                            std::cerr << "[GltfLoader] Failed to decode image bufferView " << bv_idx << " (" << span.size() << " bytes)" << std::endl;
                        }
                    }
                }
            }
        } else if (img_val.has("uri")) {
            std::string_view uri = img_val.get_string("uri");
            if (uri.starts_with("data:")) {
                const auto b64_pos = uri.find(";base64,");
                if (b64_pos != std::string_view::npos) {
                    auto b64_bytes = decode_base64(uri.substr(b64_pos + 8));
                    tex = Texture2D::load_from_memory(b64_bytes);
                }
            } else if (!base_dir.empty()) {
                std::string path = base_dir + "/" + std::string(uri);
                tex = Texture2D::load_from_file(path);
            }
        }

        if (!tex) {
            tex = Texture2D::create_solid(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        loaded_images.push_back(tex);
    }

    const auto& json_textures = root.get_array("textures");
    result.textures.reserve(json_textures.size());
    for (const auto& tex_val : json_textures) {
        const int source_idx = tex_val.get_int("source", -1);
        if (source_idx >= 0 && static_cast<size_t>(source_idx) < loaded_images.size()) {
            result.textures.push_back(loaded_images[source_idx]);
        } else {
            result.textures.push_back(Texture2D::create_solid(Vec4(1.0f, 1.0f, 1.0f, 1.0f)));
        }
    }

    // 5. Ingest Materials
    const auto& json_materials = root.get_array("materials");
    result.materials.reserve(json_materials.size());

    for (const auto& mat_val : json_materials) {
        auto mat = std::make_shared<PbrMaterial>();

        if (mat_val.has("pbrMetallicRoughness")) {
            const auto pbr_val = mat_val.get("pbrMetallicRoughness");

            if (pbr_val && pbr_val->has("baseColorFactor")) {
                const auto& col_arr = pbr_val->get_array("baseColorFactor");
                if (col_arr.size() >= 4) {
                    mat->base_color_factor = Vec4(
                        col_arr[0].to_float(1.0f),
                        col_arr[1].to_float(1.0f),
                        col_arr[2].to_float(1.0f),
                        col_arr[3].to_float(1.0f)
                    );
                }
            }

            mat->metallic_factor = pbr_val ? pbr_val->get_float("metallicFactor", 1.0f) : 1.0f;
            mat->roughness_factor = pbr_val ? pbr_val->get_float("roughnessFactor", 1.0f) : 1.0f;

            if (pbr_val && pbr_val->has("baseColorTexture")) {
                const auto& bct = pbr_val->get_object("baseColorTexture");
                const auto it = bct.find("index");
                if (it != bct.end()) {
                    const int tex_idx = it->second.to_int(-1);
                    if (tex_idx >= 0 && static_cast<size_t>(tex_idx) < result.textures.size()) {
                        mat->base_color_texture = result.textures[tex_idx];
                    }
                }
            }

            if (pbr_val && pbr_val->has("metallicRoughnessTexture")) {
                const auto& mrt = pbr_val->get_object("metallicRoughnessTexture");
                const auto it = mrt.find("index");
                if (it != mrt.end()) {
                    const int tex_idx = it->second.to_int(-1);
                    if (tex_idx >= 0 && static_cast<size_t>(tex_idx) < result.textures.size()) {
                        auto t = result.textures[tex_idx];
                        if (t && (t->width > 1 || t->height > 1)) {
                            mat->metallic_roughness_texture = t;
                        } else {
                            // If texture decoding failed, use realistic fallback
                            mat->metallic_factor = 0.35f;
                            mat->roughness_factor = 0.45f;
                        }
                    }
                }
            }
        }

        if (mat_val.has("emissiveFactor")) {
            const auto& em_arr = mat_val.get_array("emissiveFactor");
            if (em_arr.size() >= 3) {
                mat->emissive_factor = Vec3(
                    em_arr[0].to_float(0.0f),
                    em_arr[1].to_float(0.0f),
                    em_arr[2].to_float(0.0f)
                );
            }
        }
        if (mat_val.has("emissiveTexture")) {
            const auto& et = mat_val.get_object("emissiveTexture");
            const auto it = et.find("index");
            if (it != et.end()) {
                const int tex_idx = it->second.to_int(-1);
                if (tex_idx >= 0 && static_cast<size_t>(tex_idx) < result.textures.size()) {
                    mat->emissive_texture = result.textures[tex_idx];
                }
            }
        }

        if (mat_val.has("normalTexture")) {
            const auto& nt = mat_val.get_object("normalTexture");
            const auto it = nt.find("index");
            if (it != nt.end()) {
                const int tex_idx = it->second.to_int(-1);
                if (tex_idx >= 0 && static_cast<size_t>(tex_idx) < result.textures.size()) {
                    mat->normal_texture = result.textures[tex_idx];
                }
            }
            const auto scale_it = nt.find("scale");
            if (scale_it != nt.end()) {
                mat->normal_scale = scale_it->second.to_float(1.0f);
            }
        }

        if (mat_val.has("occlusionTexture")) {
            const auto& ot = mat_val.get_object("occlusionTexture");
            const auto it = ot.find("index");
            if (it != ot.end()) {
                const int tex_idx = it->second.to_int(-1);
                if (tex_idx >= 0 && static_cast<size_t>(tex_idx) < result.textures.size()) {
                    mat->occlusion_texture = result.textures[tex_idx];
                }
            }
            const auto str_it = ot.find("strength");
            if (str_it != ot.end()) {
                mat->occlusion_strength = str_it->second.to_float(1.0f);
            }
        }

        std::string_view alpha_mode = mat_val.get_string("alphaMode", "OPAQUE");
        if (alpha_mode == "MASK") {
            mat->alpha_mode = AlphaMode::Mask;
            mat->alpha_cutoff = mat_val.get_float("alphaCutoff", 0.5f);
        } else if (alpha_mode == "BLEND") {
            mat->alpha_mode = AlphaMode::Blend;
        } else {
            mat->alpha_mode = AlphaMode::Opaque;
        }

        mat->double_sided = mat_val.get_bool("doubleSided", false);

        result.materials.push_back(std::move(mat));
    }

    if (result.materials.empty()) {
        result.materials.push_back(PbrMaterial::create_default_dielectric());
    }

    // 6. Ingest Meshes
    const auto& json_meshes = root.get_array("meshes");
    result.meshes.reserve(json_meshes.size());

    for (const auto& mesh_val : json_meshes) {
        auto mesh = std::make_shared<Mesh3D>();
        const auto& prims = mesh_val.get_array("primitives");

        bool has_any_normals = false;
        bool has_any_tangents = false;

        for (const auto& prim_val : prims) {
            const auto& attrs = prim_val.get_object("attributes");
            const auto pos_it = attrs.find("POSITION");
            if (pos_it == attrs.end()) continue;

            const size_t pos_acc_idx = static_cast<size_t>(pos_it->second.to_int(-1));
            if (pos_acc_idx >= accessors.size()) continue;

            const auto& pos_acc = accessors[pos_acc_idx];
            const auto pos_data = read_accessor_floats(pos_acc, buffer_views, buffers);
            const size_t vert_count = pos_acc.count;
            if (pos_data.size() < vert_count * 3) {
                continue; // Skip primitives with unresolvable or compressed position data
            }

            std::vector<float> norm_data;
            const auto norm_it = attrs.find("NORMAL");
            if (norm_it != attrs.end()) {
                const size_t norm_acc_idx = static_cast<size_t>(norm_it->second.to_int(-1));
                if (norm_acc_idx < accessors.size()) {
                    norm_data = read_accessor_floats(accessors[norm_acc_idx], buffer_views, buffers);
                    has_any_normals = true;
                }
            }

            std::vector<float> tang_data;
            const auto tang_it = attrs.find("TANGENT");
            if (tang_it != attrs.end()) {
                const size_t tang_acc_idx = static_cast<size_t>(tang_it->second.to_int(-1));
                if (tang_acc_idx < accessors.size()) {
                    tang_data = read_accessor_floats(accessors[tang_acc_idx], buffer_views, buffers);
                    has_any_tangents = true;
                }
            }

            std::vector<float> uv0_data;
            const auto uv0_it = attrs.find("TEXCOORD_0");
            if (uv0_it != attrs.end()) {
                const size_t uv0_acc_idx = static_cast<size_t>(uv0_it->second.to_int(-1));
                if (uv0_acc_idx < accessors.size()) {
                    uv0_data = read_accessor_floats(accessors[uv0_acc_idx], buffer_views, buffers);
                }
            }

            std::vector<float> uv1_data;
            const auto uv1_it = attrs.find("TEXCOORD_1");
            if (uv1_it != attrs.end()) {
                const size_t uv1_acc_idx = static_cast<size_t>(uv1_it->second.to_int(-1));
                if (uv1_acc_idx < accessors.size()) {
                    uv1_data = read_accessor_floats(accessors[uv1_acc_idx], buffer_views, buffers);
                }
            }

            std::vector<uint16_t> joints_data;
            const auto joints_it = attrs.find("JOINTS_0");
            if (joints_it != attrs.end()) {
                const size_t j_acc_idx = static_cast<size_t>(joints_it->second.to_int(-1));
                if (j_acc_idx < accessors.size()) {
                    joints_data = read_accessor_joints(accessors[j_acc_idx], buffer_views, buffers);
                }
            }

            std::vector<float> weights_data;
            const auto weights_it = attrs.find("WEIGHTS_0");
            if (weights_it != attrs.end()) {
                const size_t w_acc_idx = static_cast<size_t>(weights_it->second.to_int(-1));
                if (w_acc_idx < accessors.size()) {
                    weights_data = read_accessor_floats(accessors[w_acc_idx], buffer_views, buffers);
                }
            }

            const bool has_skin_attr = !joints_data.empty() && !weights_data.empty();
            if (has_skin_attr) {
                mesh->skin_data.reserve(mesh->skin_data.size() + vert_count);
            }

            const uint32_t vertex_offset = static_cast<uint32_t>(mesh->vertices.size());
            mesh->vertices.reserve(mesh->vertices.size() + vert_count);

            for (size_t i = 0; i < vert_count; ++i) {
                Vertex3D v{};
                v.position = Vec3(pos_data[i * 3], pos_data[i * 3 + 1], pos_data[i * 3 + 2]);

                if (!norm_data.empty() && (i * 3 + 2) < norm_data.size()) {
                    v.normal = Vec3(norm_data[i * 3], norm_data[i * 3 + 1], norm_data[i * 3 + 2]);
                } else {
                    v.normal = Vec3::up();
                }

                if (!tang_data.empty() && (i * 4 + 3) < tang_data.size()) {
                    v.tangent[0] = tang_data[i * 4];
                    v.tangent[1] = tang_data[i * 4 + 1];
                    v.tangent[2] = tang_data[i * 4 + 2];
                    v.tangent[3] = tang_data[i * 4 + 3];
                } else {
                    v.tangent[0] = 1.0f;
                    v.tangent[1] = 0.0f;
                    v.tangent[2] = 0.0f;
                    v.tangent[3] = 1.0f;
                }

                if (!uv0_data.empty() && (i * 2 + 1) < uv0_data.size()) {
                    v.uv[0] = uv0_data[i * 2];
                    v.uv[1] = uv0_data[i * 2 + 1];
                } else {
                    v.uv[0] = 0.0f;
                    v.uv[1] = 0.0f;
                }

                if (!uv1_data.empty() && (i * 2 + 1) < uv1_data.size()) {
                    v.uv1[0] = uv1_data[i * 2];
                    v.uv1[1] = uv1_data[i * 2 + 1];
                } else {
                    v.uv1[0] = 0.0f;
                    v.uv1[1] = 0.0f;
                }

                mesh->vertices.push_back(v);

                if (has_skin_attr) {
                    VertexSkinData sd{};
                    if ((i * 4 + 3) < joints_data.size() && (i * 4 + 3) < weights_data.size()) {
                        sd.joints[0] = joints_data[i * 4 + 0];
                        sd.joints[1] = joints_data[i * 4 + 1];
                        sd.joints[2] = joints_data[i * 4 + 2];
                        sd.joints[3] = joints_data[i * 4 + 3];

                        sd.weights[0] = weights_data[i * 4 + 0];
                        sd.weights[1] = weights_data[i * 4 + 1];
                        sd.weights[2] = weights_data[i * 4 + 2];
                        sd.weights[3] = weights_data[i * 4 + 3];

                        float sum_w = sd.weights[0] + sd.weights[1] + sd.weights[2] + sd.weights[3];
                        if (sum_w > 1e-4f) {
                            float inv_sum = 1.0f / sum_w;
                            sd.weights[0] *= inv_sum;
                            sd.weights[1] *= inv_sum;
                            sd.weights[2] *= inv_sum;
                            sd.weights[3] *= inv_sum;
                        }
                    }
                    mesh->skin_data.push_back(sd);
                }
            }

            // Primitive Indices
            const uint32_t index_offset = static_cast<uint32_t>(mesh->indices.size());
            uint32_t index_count = 0;

            if (prim_val.has("indices")) {
                const size_t ind_acc_idx = static_cast<size_t>(prim_val.get_int("indices", -1));
                if (ind_acc_idx < accessors.size()) {
                    auto indices = read_accessor_indices(accessors[ind_acc_idx], buffer_views, buffers);
                    index_count = static_cast<uint32_t>(indices.size());
                    mesh->indices.reserve(mesh->indices.size() + indices.size());
                    for (uint32_t idx : indices) {
                        mesh->indices.push_back(idx + vertex_offset);
                    }
                }
            } else {
                // Non-indexed triangle list
                index_count = static_cast<uint32_t>(vert_count);
                mesh->indices.reserve(mesh->indices.size() + vert_count);
                for (uint32_t i = 0; i < index_count; ++i) {
                    mesh->indices.push_back(vertex_offset + i);
                }
            }

            SubMesh sm;
            sm.index_offset = index_offset;
            sm.index_count = index_count;
            sm.vertex_offset = vertex_offset;
            sm.material_index = static_cast<uint32_t>(std::max(0, prim_val.get_int("material", 0)));
            mesh->add_submesh(sm);
        }

        if (!has_any_normals) {
            mesh->compute_normals();
        }
        if (!has_any_tangents) {
            mesh->compute_tangents();
        }
        mesh->compute_bounds();
        mesh->upload_to_buffers();

        result.meshes.push_back(std::move(mesh));
    }

    // 7. Ingest Nodes & Hierarchy
    const auto& json_nodes = root.get_array("nodes");

    // 7. Parse Skins (Rigged Skeletons)
    const auto& json_skins = root.get_array("skins");
    for (size_t s_i = 0; s_i < json_skins.size(); ++s_i) {
        const auto& skin_val = json_skins[s_i];
        auto skeleton = std::make_shared<Skeleton>(std::string(skin_val.get_string("name", "Skeleton")));

        std::vector<Mat44> inv_bind_matrices;
        if (skin_val.has("inverseBindMatrices")) {
            const size_t inv_acc_idx = static_cast<size_t>(skin_val.get_int("inverseBindMatrices", -1));
            if (inv_acc_idx < accessors.size()) {
                auto floats = read_accessor_floats(accessors[inv_acc_idx], buffer_views, buffers);
                const size_t mat_count = floats.size() / 16;
                inv_bind_matrices.resize(mat_count);
                for (size_t m_i = 0; m_i < mat_count; ++m_i) {
                    Mat44 mat;
                    for (int r = 0; r < 4; ++r) {
                        for (int c = 0; c < 4; ++c) {
                            mat.m[r * 4 + c] = floats[m_i * 16 + c * 4 + r];
                        }
                    }
                    inv_bind_matrices[m_i] = mat;
                }
            }
        }

        const auto& joints_arr = skin_val.get_array("joints");
        skeleton->joints.resize(joints_arr.size());

        for (size_t j_i = 0; j_i < joints_arr.size(); ++j_i) {
            const int node_idx = joints_arr[j_i].to_int(-1);
            auto& joint = skeleton->joints[j_i];
            joint.node_index = node_idx;

            if (node_idx >= 0 && static_cast<size_t>(node_idx) < json_nodes.size()) {
                const auto& n_val = json_nodes[static_cast<size_t>(node_idx)];
                joint.name = std::string(n_val.get_string("name", ""));

                if (n_val.has("translation")) {
                    const auto& t_arr = n_val.get_array("translation");
                    if (t_arr.size() >= 3) {
                        joint.bind_translation = Vec3(t_arr[0].to_float(), t_arr[1].to_float(), t_arr[2].to_float());
                    }
                }
                if (n_val.has("rotation")) {
                    const auto& r_arr = n_val.get_array("rotation");
                    if (r_arr.size() >= 4) {
                        joint.bind_rotation = Quat(r_arr[0].to_float(), r_arr[1].to_float(), r_arr[2].to_float(), r_arr[3].to_float());
                    }
                }
                if (n_val.has("scale")) {
                    const auto& sc_arr = n_val.get_array("scale");
                    if (sc_arr.size() >= 3) {
                        joint.bind_scale = Vec3(sc_arr[0].to_float(1.0f), sc_arr[1].to_float(1.0f), sc_arr[2].to_float(1.0f));
                    }
                }
            }

            joint.local_translation = joint.bind_translation;
            joint.local_rotation = joint.bind_rotation;
            joint.local_scale = joint.bind_scale;

            if (j_i < inv_bind_matrices.size()) {
                joint.inverse_bind_matrix = inv_bind_matrices[j_i];
            }
        }

        // Resolve parent joint indices
        for (size_t j_i = 0; j_i < skeleton->joints.size(); ++j_i) {
            const int node_idx = skeleton->joints[j_i].node_index;
            if (node_idx < 0) continue;

            for (size_t parent_j = 0; parent_j < skeleton->joints.size(); ++parent_j) {
                if (parent_j == j_i) continue;
                const int p_node = skeleton->joints[parent_j].node_index;
                if (p_node >= 0 && static_cast<size_t>(p_node) < json_nodes.size()) {
                    for (const auto& c_val : json_nodes[static_cast<size_t>(p_node)].get_array("children")) {
                        if (c_val.to_int(-1) == node_idx) {
                            skeleton->joints[j_i].parent_index = static_cast<int32_t>(parent_j);
                            break;
                        }
                    }
                }
                if (skeleton->joints[j_i].parent_index >= 0) break;
            }
        }

        skeleton->update_matrices();
        result.skeletons.push_back(std::move(skeleton));
    }

    // 8. Parse Animations
    const auto& json_anims = root.get_array("animations");
    for (size_t a_i = 0; a_i < json_anims.size(); ++a_i) {
        const auto& anim_val = json_anims[a_i];
        auto clip = std::make_shared<AnimationClip>(std::string(anim_val.get_string("name", "Animation_" + std::to_string(a_i))));

        struct SamplerData {
            InterpolationMode interpolation{InterpolationMode::Linear};
            std::vector<float> timestamps;
            std::vector<float> output_floats;
        };

        std::vector<SamplerData> samplers;
        const auto& samplers_arr = anim_val.get_array("samplers");
        for (const auto& s_val : samplers_arr) {
            SamplerData sd;
            std::string interp_str = std::string(s_val.get_string("interpolation", "LINEAR"));
            if (interp_str == "STEP") sd.interpolation = InterpolationMode::Step;
            else if (interp_str == "CUBICSPLINE") sd.interpolation = InterpolationMode::CubicSpline;
            else sd.interpolation = InterpolationMode::Linear;

            const size_t in_acc = static_cast<size_t>(s_val.get_int("input", -1));
            const size_t out_acc = static_cast<size_t>(s_val.get_int("output", -1));

            if (in_acc < accessors.size()) {
                sd.timestamps = read_accessor_floats(accessors[in_acc], buffer_views, buffers);
                if (!sd.timestamps.empty()) {
                    clip->duration = std::max(clip->duration, sd.timestamps.back());
                }
            }
            if (out_acc < accessors.size()) {
                sd.output_floats = read_accessor_floats(accessors[out_acc], buffer_views, buffers);
            }
            samplers.push_back(std::move(sd));
        }

        const auto& channels_arr = anim_val.get_array("channels");
        for (const auto& ch_val : channels_arr) {
            const int s_idx = ch_val.get_int("sampler", -1);
            if (s_idx < 0 || static_cast<size_t>(s_idx) >= samplers.size()) continue;

            const auto& s_data = samplers[static_cast<size_t>(s_idx)];
            const auto* target_val = ch_val.get("target");
            const int target_node = target_val ? target_val->get_int("node", -1) : -1;
            std::string path_str = target_val ? std::string(target_val->get_string("path", "")) : "";

            AnimationChannel ch;
            ch.target_node = target_node;
            ch.interpolation = s_data.interpolation;
            ch.timestamps = s_data.timestamps;

            if (path_str == "translation") {
                ch.target_path = TargetPath::Translation;
                const size_t count = s_data.output_floats.size() / 3;
                ch.vec3_values.reserve(count);
                for (size_t k = 0; k < count; ++k) {
                    ch.vec3_values.emplace_back(
                        s_data.output_floats[k * 3 + 0],
                        s_data.output_floats[k * 3 + 1],
                        s_data.output_floats[k * 3 + 2]
                    );
                }
            } else if (path_str == "rotation") {
                ch.target_path = TargetPath::Rotation;
                const size_t count = s_data.output_floats.size() / 4;
                ch.quat_values.reserve(count);
                for (size_t k = 0; k < count; ++k) {
                    ch.quat_values.emplace_back(
                        s_data.output_floats[k * 4 + 0],
                        s_data.output_floats[k * 4 + 1],
                        s_data.output_floats[k * 4 + 2],
                        s_data.output_floats[k * 4 + 3]
                    );
                }
            } else if (path_str == "scale") {
                ch.target_path = TargetPath::Scale;
                const size_t count = s_data.output_floats.size() / 3;
                ch.vec3_values.reserve(count);
                for (size_t k = 0; k < count; ++k) {
                    ch.vec3_values.emplace_back(
                        s_data.output_floats[k * 3 + 0],
                        s_data.output_floats[k * 3 + 1],
                        s_data.output_floats[k * 3 + 2]
                    );
                }
            }

            clip->channels.push_back(std::move(ch));
        }

        result.animations.push_back(std::move(clip));
    }

    // 9. Ingest Nodes & Hierarchy
    // Node hierarchy traversal lambda
    auto process_node = [&](auto& self, int node_idx, const Mat44& parent_transform) -> void {
        if (node_idx < 0 || static_cast<size_t>(node_idx) >= json_nodes.size()) return;
        const auto& node_val = json_nodes[node_idx];

        Mat44 local_transform = Mat44::identity();

        if (node_val.has("matrix")) {
            const auto& m_arr = node_val.get_array("matrix");
            if (m_arr.size() >= 16) {
                // Transpose glTF column-major matrix to Nisaba row-major Mat44
                for (int row = 0; row < 4; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        local_transform.m[row * 4 + col] = m_arr[col * 4 + row].to_float(0.0f);
                    }
                }
            }
        } else {
            Vec3 translation = Vec3::zero();
            Quat rotation = Quat::identity();
            Vec3 scale = Vec3::one();

            if (node_val.has("translation")) {
                const auto& t_arr = node_val.get_array("translation");
                if (t_arr.size() >= 3) {
                    translation = Vec3(
                        t_arr[0].to_float(0.0f),
                        t_arr[1].to_float(0.0f),
                        t_arr[2].to_float(0.0f)
                    );
                }
            }

            if (node_val.has("rotation")) {
                const auto& r_arr = node_val.get_array("rotation");
                if (r_arr.size() >= 4) {
                    rotation = Quat(
                        r_arr[0].to_float(0.0f),
                        r_arr[1].to_float(0.0f),
                        r_arr[2].to_float(0.0f),
                        r_arr[3].to_float(1.0f)
                    );
                }
            }

            if (node_val.has("scale")) {
                const auto& s_arr = node_val.get_array("scale");
                if (s_arr.size() >= 3) {
                    scale = Vec3(
                        s_arr[0].to_float(1.0f),
                        s_arr[1].to_float(1.0f),
                        s_arr[2].to_float(1.0f)
                    );
                }
            }

            local_transform = Mat44::transform(translation, rotation, scale);
        }

        const Mat44 world_transform = parent_transform * local_transform;

        if (node_val.has("mesh")) {
            const int mesh_idx = node_val.get_int("mesh", -1);
            if (mesh_idx >= 0 && static_cast<size_t>(mesh_idx) < result.meshes.size()) {
                auto mesh_ptr = result.meshes[mesh_idx];
                const int skin_idx = node_val.get_int("skin", -1);

                std::shared_ptr<Mesh3D> render_mesh = mesh_ptr;

                if (skin_idx >= 0 && static_cast<size_t>(skin_idx) < result.skeletons.size() && mesh_ptr->has_skinning()) {
                    auto dyn_mesh = std::make_shared<Mesh3D>(*mesh_ptr);
                    auto skinned_inst = std::make_shared<SkinnedMeshInstance>(
                        mesh_ptr, dyn_mesh, result.skeletons[static_cast<size_t>(skin_idx)]
                    );
                    result.skinned_meshes.push_back(skinned_inst);
                    render_mesh = dyn_mesh;
                }

                auto renderable = std::make_shared<Renderable3D>(render_mesh);

                for (size_t sm_i = 0; sm_i < mesh_ptr->submesh_count(); ++sm_i) {
                    const uint32_t mat_idx = mesh_ptr->get_submesh(sm_i).material_index;
                    if (mat_idx < result.materials.size()) {
                        renderable->set_material(sm_i, result.materials[mat_idx]);
                    }
                }

                renderable->set_transform(world_transform);
                result.scene->add_renderable(renderable);
                result.renderables.push_back(std::move(renderable));
            }
        }

        if (node_val.has("children")) {
            for (const auto& child_val : node_val.get_array("children")) {
                self(self, child_val.to_int(-1), world_transform);
            }
        }
    };

    // Determine root nodes
    std::vector<int> root_node_indices;
    const auto& json_scenes = root.get_array("scenes");
    int default_scene_idx = root.get_int("scene", 0);

    if (!json_scenes.empty() && default_scene_idx >= 0 && static_cast<size_t>(default_scene_idx) < json_scenes.size()) {
        const auto& scn = json_scenes[default_scene_idx];
        for (const auto& nid : scn.get_array("nodes")) {
            root_node_indices.push_back(nid.to_int(-1));
        }
    } else {
        // Fallback: collect all nodes not marked as child of any node
        std::vector<bool> is_child(json_nodes.size(), false);
        for (const auto& n : json_nodes) {
            for (const auto& c : n.get_array("children")) {
                int cid = c.to_int(-1);
                if (cid >= 0 && static_cast<size_t>(cid) < is_child.size()) {
                    is_child[cid] = true;
                }
            }
        }
        for (size_t i = 0; i < is_child.size(); ++i) {
            if (!is_child[i]) {
                root_node_indices.push_back(static_cast<int>(i));
            }
        }
    }

    for (int root_idx : root_node_indices) {
        process_node(process_node, root_idx, Mat44::identity());
    }

    // 10. Instantiate Animator if animations or skeletons exist
    if (!result.skeletons.empty()) {
        result.animator = std::make_shared<Animator>(result.skeletons[0]);
        result.animator->clips = result.animations;
        result.animator->skinned_meshes = result.skinned_meshes;
        if (!result.animations.empty()) {
            result.animator->play(0, true, 1.0f);
        }
    }

    result.success = true;
    return result;
}

} // namespace nisaba::render3d
