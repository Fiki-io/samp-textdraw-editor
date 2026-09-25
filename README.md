# SA-MP TextDraw Editor (Native C++ Android)

A native Android TextDraw Editor for GTA SA-MP (San Andreas Multiplayer), built with **C++ (NDK)**, **OpenGL ES 3.0**, and **Dear ImGui**.

Engineered for 100% authentic in-game rendering (zero dummy boxes), touch precision, and protection against reverse engineering.

---

## 🌟 Key Features

1. **Virtual 640x480 Viewport Engine**:
   - Matches SA-MP's fixed coordinate system ($X \in [0.0, 640.0], Y \in [0.0, 480.0]$).
   - Supports **4:3 Classic SA-MP** Letterbox mode and **16:9 Widescreen** mode.
   - Smooth pinch-to-zoom ($0.25\times$ to $10.0\times$) and pan navigation.
   - Configurable grid snapping ($0.1, 0.5, 1.0, 5.0, 10.0$ units).

2. **Authentic GTA SA Fonts (Font 0 – 3)**:
   - Uses real GTA SA font texture atlases (`font1`, `font2`) and proportional spacing metrics from `fonts.dat`.
   - Font 0: Diploma / Gothic
   - Font 1: Standard (Chalet London 1960)
   - Font 2: Sub-header / Futura Condensed
   - Font 3: Pricedown (GTA Title font)

3. **591 Authentic GTA SA Sprites (Font 4: `TXD:TEXTURE`)**:
   - All classic SA-MP sprite libraries bundled directly in the APK:
     - `hud:` (69 sprites including `radar_light`, `fist`, `arrow`, blips, crosshairs)
     - `ld_beat:` (`chit`, `circle`, `cring`, `cross`, etc.)
     - `ld_card:`, `ld_chat:`, `ld_drv:`, `ld_dual:`, `ld_grav:`, `ld_none:`, `ld_otb:`, `ld_poke:`, `ld_pool:`, `ld_race:`, `ld_roul:`, `ld_shtr:`, `ld_slot:`, `ld_spac:`, `ld_tatt:`
     - `loadscs:`, `loadsc0` – `loadsc13:`
   - Integrated **Sprite Picker** with instant live search filter.

4. **Real 3D Model Preview (Font 5: `TextDrawSetPreviewModel`) — Anti-Dummy Box**:
   - Built-in RenderWare `.dff` 3D mesh parser & renderer.
   - Bundles 143 iconic vehicles (Infernus, Sultan, NRG-500, etc.), skins (CJ, Smoke, Ryder, Grove, Ballas, Cops), and weapons.
   - Full support for:
     - `TextDrawSetPreviewRot(rx, ry, rz, zoom)`
     - `TextDrawSetPreviewVehCol(color1, color2)` mapped to official 256 SA-MP vehicle colors from `carcols.json`.

5. **Anti-Reverse Engineering (Hardened Native C++)**:
   - Core logic, coordinate calculations, and rendering are compiled to machine code in `libtextdraw_editor.so` (`arm64-v8a` and `armeabi-v7a`).
   - Compiler hardening: `-O3`, `-fvisibility=hidden`, `-fvisibility-inlines-hidden`, `-Wl,--gc-sections`, and debug symbol stripping (`-s`).
   - Cannot be decompiled by Java/Kotlin decompilers (JADX, APKTool).

6. **Micro D-Pad for Mobile Touch Precision**:
   - 4-way direction buttons with step selector ($0.1, 0.5, 1.0, 5.0$) to nudge elements by exact pixels without finger slipping.

7. **Production Pawn Exporter & Importer**:
   - Generates clean, ready-to-use Pawn code for both Global (`TextDrawCreate`) and Player TextDraws (`CreatePlayerTextDraw`).
   - One-tap "Copy to Clipboard".
   - Pawn code importer to load and edit existing scripts.

---

## 🚀 Cloud Build via GitHub Actions (Zero Laptop Load)

You do **not** need to install Android Studio or the NDK on your laptop. Build the APK directly in the cloud:

### How to Build:
1. Create a new repository on GitHub (e.g. `samp-textdraw-editor`).
2. Push the contents of the `android/` directory to your repository:
   ```bash
   cd android
   git init
   git add .
   git commit -m "Initial commit: Native C++ SA-MP TextDraw Editor"
   git branch -M main
   git remote add origin https://github.com/<your-username>/<your-repo-name>.git
   git push -u origin main
   ```
3. Open your repository on GitHub in your browser.
4. Click on the **Actions** tab.
5. The workflow **"Build Android APK (C++ Native)"** will automatically start building.
   *(You can also trigger it manually by clicking "Run workflow")*.
6. Once the build finishes (~2-3 minutes), click on the completed run.
7. Under **Artifacts**, download **`SAMP-TextDraw-Editor-Release-APK`**.
8. Unzip and install the `.apk` on your Android phone!
