<div align="center">

# Orion — Sovereign 3D Real-Time PBR & Physics Engine
### Ultra-High-Performance, Zero-Dependency C++20 3D Rendering, Clustered Lighting, glTF 2.0 & Rigid Body Dynamics

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build System](https://img.shields.io/badge/Build-Meson%20%2B%20Ninja-green.svg)](https://mesonbuild.com)
[![Dependencies](https://img.shields.io/badge/Dependencies-Zero%20(Sovereign)-brightgreen.svg)](#technological-sovereignty)
[![Platforms](https://img.shields.io/badge/Platforms-Linux%20%7C%20Windows%20%7C%20Android%20%7C%20WASM-orange.svg)](#cross-platform)

</div>

**Orion** (`orion`) is a high-performance, embedded-first, sovereign 3D real-time rendering and rigid-body physics engine written from scratch in **Modern ISO C++20**. Designed with absolute technological sovereignty, it operates with **zero external third-party engine dependencies**, delivering cutting-edge Physically Based Rendering (PBR), forward+ clustered lighting, cascaded shadow mapping, hardware skeletal skinning, glTF 2.0 ingestion, and a deterministic 3D rigid-body impulse physics simulation.

---

## Visual Showcases

<p align="center">
  <img src="../showcase/showcase_advanced_pbr.png" alt="Orion Sovereign 3D Cinematic PBR & Physics Showcase" width="950">
</p>
<p align="center"><i>Real-time 1080p GPU hardware rendering of complex PBR physical scenes with Cook-Torrance GGX microfacet shading, clustered lighting, GTAO ambient occlusion, bloom, and dynamic rigid bodies.</i></p>

---

## Architectural Highlights

```
+----------------------------------------------------------------------------------------------------------------+
|                                        ORION 3D ENGINE ARCHITECTURE                                            |
+----------------------------------------------------------------------------------------------------------------+
|  [Scene & Animation]  Scene3D | View3D | SkeletalMesh | Animator | AnimationClip | OrbitCameraController       |
|  [PBR Rendering]      CookTorranceBRDF | PbrMaterial | ClusteredGrid | Light3D | CascadedShadowMap | IBL / DFG |
|  [Post-Processing]    PostprocessPipeline | GTAO Pass | Bloom Pass | Depth Pass | Hardware State Guard         |
|  [Asset Pipeline]     GltfLoader (glTF 2.0 / GLB) | orionjs (JSON Parser) | orionimg (PNG, JPEG, WebP, QOI)   |
|  [Rigid Body Physics] Body (128B) | MotionProperties (128B) | 4-Way SIMD BVH | GJK/EPA | ImpulseSolver | DSU   |
|  [Math & SIMD]        Vec3 | Mat44 | Quat | Transform3D | AABB | Ray | Plane | Sphere | SIMD (SSE/AVX2/NEON) |
+----------------------------------------------------------------------------------------------------------------+
```

### 1. Sovereign 3D Math (`orion::math`)
- **Memory & Cache Layout**: 16-byte aligned mathematical types optimized for vectorized SIMD operations.
- **Core Primitives**: `Vec3`, `Vec4`, `Mat44` (column-major, Vulkan $[0, 1]$ depth normalization), `Quat` (quaternions with slerp/nlerp), `Transform3D` (position, rotation, non-uniform scale), `AABB`, `Ray`, `Plane`, `Sphere`.
- **Hardware Acceleration**: SSE2, AVX2, and ARM NEON intrinsics for dot products, cross products, 4x4 matrix multiplication, and bounding volume tests.

### 2. Physically Based Rendering Pipeline (`orion`)
- **Microfacet Cook-Torrance BRDF**:
  - **$D$ (Normal Distribution)**: Trowbridge-Reitz GGX distribution.
  - **$G$ (Geometric Shadowing)**: Smith-Schlick shadowing-masking model.
  - **$F$ (Fresnel Reflectance)**: Schlick's approximation with $F_0$ metallic interpolation.
- **Image-Based Lighting (IBL)**:
  - Irradiance diffuse evaluation via 9-coefficient Spherical Harmonics ($L \le 2$).
  - Pre-filtered specular environment cubemaps with split-sum approximation.
  - Analytical 2D DFG lookup texture (BRDF integration LUT).
- **Clustered Forward+ Lighting**:
  - View-frustum partition into 3D cluster tiles ($16 \times 9 \times 24$ depth slices).
  - Efficient light culling on CPU/GPU, enabling hundreds of dynamic point and spot lights with zero-overhead forward shading.
- **Cascaded Shadow Mapping (CSM)**:
  - Multi-tier directional light shadow cascades with texel snapping to prevent sub-pixel shimmering during camera translation.
  - 4-tap Percentage-Closer Filtering (PCF) and normal-offset depth bias preventing shadow acne and Peter-Panning.
- **Cinematic Post-Processing**:
  - **GTAO (Ground Truth Ambient Occlusion)**: High-fidelity horizon-based screen-space ambient occlusion.
  - **Physically-Based Bloom**: Multi-pass downsample/upsample blur with threshold knee and HDR tonemapping.

### 3. Skeletal Animation & Skinning
- **Dual Representation**: Matrix and Dual-Quaternion skeletal bone transforms.
- **Hierarchical Bone Graph**: Up to 256 active bones per skeleton instance with automated inverse bind pose derivation.
- **Animation Clips**: Linear and cubic-spline keyframe interpolation for translation, rotation, and scale channels.
- **Blending & States**: Real-time cross-fading, layered blend trees, and sub-frame temporal interpolation.

### 4. glTF 2.0 Sovereign Ingestion
- **Formats Supported**: Standard `.gltf` (JSON + external buffers) and binary `.glb` containers.
- **Parsers Built-In**:
  - `orionjs`: Sovereign, zero-allocation RFC 8259 JSON parser.
  - `orionimg`: Sovereign image decoding (PNG, JPEG, WebP, QOI) supporting `EXT_texture_webp`.
  - Base64 data URI decoding and direct memory buffer view mapping.
- **Features Ingested**: Meshes, primitives, PBR metallic-roughness materials, normal maps, occlusion maps, emissive maps, skinning hierarchies, and morph targets.

### 5. Sovereign 3D Rigid Body Physics (`orion::physics`)
- **Zero Steady-State Heap Allocation**: Strict 0-byte dynamic allocation during simulation steps using contiguous pre-allocated memory arenas.
- **Cache-Optimized Representations**:
  - `Body`: Exactly 128 bytes (2 cache lines). Distinguishes static scenery from dynamic bodies.
  - `MotionProperties`: Exactly 128 bytes (2 cache lines). Houses velocities, mass inverses, and gyroscopic inertia tensors.
- **SIMD 4-Way Dynamic AABB Tree (`BroadPhase`)**:
  - 4-way Bounding Volume Hierarchy (`QuadTree` BVH) testing 4 child AABBs simultaneously via 128-bit SIMD.
  - Velocity-expanded speculative AABBs and collision layer bitmasks.
- **Narrow-Phase Geometric Collision Dispatcher**:
  - Analytical fast paths: `Sphere vs Sphere`, `Sphere vs Box`, `Sphere vs Plane`, `Box vs Box` (SAT).
  - General convex collision: 3D GJK (Gilbert-Johnson-Keerthi) Voronoi simplex solver and Expanding Polytope Algorithm (EPA) for polyhedra (`CylinderShape`, `CapsuleShape`, `ConvexHullShape`).
  - Contact Manifold Reduction: Caps contact patches to at most 4 coplanar points, guaranteeing maximal surface stability.
- **Sequential Impulse Solver**:
  - Projected Gauss-Seidel iterative velocity solver with Coulomb friction cones ($|\lambda_t| \le \mu \lambda_n$).
  - Non-linear Baumgarte position error stabilization.
  - Persistent warm-starting preserving accumulated impulses across frames to eliminate jitter in resting stacks.
- **Simulation Islands & Energy Deactivation Sleeping**:
  - Disjoint-Set Union (DSU) graph clustering with path compression and rank optimization.
  - Quiescent energy-threshold sleep deactivation, dropping resting stacks to 0% CPU consumption with zero drift.

---

## 2D/3D Hybrid Integration (Nisaba + Orion)

Orion is engineered to operate in harmony with the **Nisaba 2D Vector Engine**:

1. **2D Overlay over 3D (HUD / Game UI)**:
   - Orion renders the 3D scene to the active framebuffer or viewport.
   - Nisaba renders vector UI elements, text, minimaps, and healthbars directly on top with sub-pixel antialiasing.
2. **3D Embedded in 2D (Viewports / Widgets)**:
   - Orion renders into an offscreen [`RenderSurface3D`](include/orion/gpu/render_surface3d.hpp).
   - The resulting texture is consumed by Nisaba's 2D canvas, enabling 3D model previews, rotating avatars, or 3D icons inside 2D Flexbox layouts with 2D rounded corners, drop shadows, and opacity.
3. **2D Projected into 3D (Diegetic In-Game Screens)**:
   - Nisaba renders interactive vector interfaces directly into a dynamic texture, which Orion binds as an albedo/emissive material on 3D meshes (e.g. computer screens inside a 3D world).

---

## Building Orion Standalone

Orion can be compiled as a standalone 3D engine without compiling the 2D subsystems:

```bash
# Configure standalone Orion
meson setup build orion --buildtype=release

# Compile static and shared libraries
ninja -C build

# Output targets:
# - liborion.a   (Static library)
# - liborion.so  (Shared library)
```

Orion can also be built as part of the unified Nisaba monorepo:

```bash
meson setup build -Denable_orion=true --buildtype=release
ninja -C build
meson test -C build
```

---

## Directory Structure

```
orion/
├── include/
│   └── orion/
│       ├── orion.hpp               # Master umbrella header
│       ├── math/                   # Vec3, Mat44, Quat, Transform3D, SIMD
│       ├── camera/                 # Camera3D, Frustum3D, OrbitController
│       ├── geometry/               # Mesh, Primitives, VertexBuffer
│       ├── material/               # PbrMaterial, BRDF, Texture2D
│       ├── lighting/               # Light3D, ClusterGrid, ShadowMap
│       ├── ibl/                    # EnvironmentMap, DfgLUT, SphericalHarmonics
│       ├── animation/              # Skeleton, SkinnedMesh, Animator, Clips
│       ├── gltf/                   # GltfLoader (glTF 2.0 / GLB)
│       ├── postprocess/            # GTAO, Bloom, PostprocessPipeline
│       ├── gpu/                    # GpuBackend3D, RenderSurface3D, ShaderProgram
│       ├── scene/                  # Scene3D, View3D, Renderable3D, Renderer3D
│       ├── physics/                # Body, BroadPhase, NarrowPhase, GJK/EPA, Solver, Islands
│       ├── orionjs/                # Sovereign JSON Parser
│       └── orionimg/               # Sovereign Image Codecs (PNG, JPEG, WebP, QOI)
├── src/                            # Implementation files (.cpp)
└── meson.build                     # Sovereign standalone build configuration
```

---

## Technological Sovereignty

Orion adheres to the strict architectural rule of technological sovereignty:
- **Zero Third-Party Engines**: No external graphics frameworks, physics libraries, or third-party loaders.
- **Pure ISO C++20**: Standard library only, with hardware acceleration via compiler SIMD intrinsics and standard OpenGL / Vulkan GPU APIs.
- **Deterministic & Portable**: Runs consistently across Linux, Windows, Android, and WebAssembly targets.
